from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.webdriver import WebDriver
from selenium.common.exceptions import NoSuchElementException
from typing import Dict
import glob

from time import sleep, monotonic

import re
import os
import sys
import random
from traceback import print_exception


from .cli import parse_cli_args, SEDDCLIArgs
from .config import load_sedd_config
from .data import sites
from .meta import notifications
from .watcher.observer import register_pending_downloads_observer
from .watcher.state import DownloadState
from .watcher.recovery import LastState
from . import utils

from .driver import init_output_dir, init_firefox_driver
import logging
from loguru import logger

args = parse_cli_args()


if args.verbose:
    logging.basicConfig(level=logging.DEBUG)

sedd_config = load_sedd_config()

output_dir = init_output_dir(args.output_dir)

browser, ubo_id = init_firefox_driver(
    sedd_config,
    args.disable_undetected,
    output_dir
)


def kill_cookie_shit(browser: WebDriver):
    sleep(3)
    browser.execute_script(
        """let elem = document.getElementById("onetrust-banner-sdk"); if (elem) { elem.parentNode.removeChild(elem); }""")
    sleep(1)

def solve_recaptcha(browser: WebDriver):
    try:
        logger.info("Trying to bypass captcha")
        # Wait for the captcha to load into a checkbox
        sleep(5)
        recaptcha_iframe = browser.find_element(
            By.CSS_SELECTOR,
            "iframe[title='reCAPTCHA']"
        )
        browser.switch_to.frame(
            recaptcha_iframe
        )
        # TODO: will this ever work?
        sleep(1)
        browser.find_element(
            By.ID,
            "recaptcha-anchor"
        ).click()

        try:
            browser.find_element(By.ID, "rc-imageselect")
        except:
            logger.info("Successfully bypassed captcha")
            return

        raise RuntimeError("reCAPTCHA threw in an image selector. Can't solve")
    except Exception as e:
        print(e)
        logger.error(
            "Looks like the captcha at {} isn't recaptcha. Can't solve",
            browser.current_url
        )
        return
    finally:
        browser.switch_to.default_content()


def check_cloudflare_intercept(browser: WebDriver):
    if browser.title == "Just a moment...":
        logger.info("CF verification hit. Trying soft workaround")
        sleep(15)

        if (browser.title == "Just a moment..."):
            logger.warning("Waiting did not work. Trying to solve captcha")
            solve_recaptcha(browser)
            sleep(4)
            if browser.title != "Just a moment...":
                logger.info("Managed to bypass.")
                return
            logger.error(
                "Irrecoverable state suspected; captcha solving likely required"
            )
            if sedd_config.unsupervised:
                raise RuntimeError(
                    "Can't get past full-screen cloudflare block. Manual intervention needed"
                )

            notifications.notify(
                "CloudFlare verification hit; auto-verification failed. "
                    "Please complete the captcha",
                sedd_config
            )
        else:
            logger.info("Auto-recovered from CF wall")
            return

        while browser.title == "Just a moment...":
            logger.info("Still stuck on CF verification. Waiting for 10 seconds")
            sleep(10)

def is_logged_in(browser: WebDriver, site: str):
    url = f"{site}/users/current"
    browser.get(url)
    sleep(1)
    check_cloudflare_intercept(browser)

    return "/users/" in browser.current_url


def login_or_create(browser: WebDriver, site: str):
    if is_logged_in(browser, site):
        logger.debug("Already logged in")
    else:
        logger.info("Not logged in and/or not registered. Logging in now")
        while True:
            browser.get(f"{site}/users/login")
            check_cloudflare_intercept(browser)

            if "?newreg" in browser.current_url:
                logger.info(f"Auto-created {site} without login needed")
                break

            email_elem = browser.find_element(By.ID, "email")
            password_elem = browser.find_element(By.ID, "password")
            email_elem.send_keys(sedd_config.email)
            password_elem.send_keys(sedd_config.password)
            retryLogin = False

            curr_url = browser.current_url
            browser.find_element(By.ID, "submit-button").click()

            try:
                elem = browser.find_element(By.CSS_SELECTOR, "#login-form > .js-error-message")
                if elem is not None:
                    logger.info("Login failed quietly. Retrying")
                    continue
            except:
                # No error element
                pass

            check_cloudflare_intercept(browser)

            while browser.current_url == curr_url:
                sleep(3)


            captcha_walled = False
            while "/nocaptcha" in browser.current_url:
                if not captcha_walled:
                    logger.error("Captcha wall hit during login")
                    solve_recaptcha(browser)
                    sleep(4)
                    if "/nocaptcha" not in browser.current_url:
                        logger.info("Auto-solving worked")
                        break
                    captcha_walled = True
                    if args.unsupervised:
                        raise RuntimeError(
                            "Unsolvable captcha wall hit during login"
                        )
                    notifications.notify(
                        "Captcha wall hit during login", sedd_config
                    )
                sleep(10)

            if captcha_walled or retryLogin:
                continue

            if not is_logged_in(browser, site):
                raise RuntimeError("Login failed")

            break


def download_data_dump(browser: WebDriver, site: str, meta_url: str | None, etags: Dict[str, str]):
    logger.info(f"Downloading data dump from {site}")

    def _exec_download(browser: WebDriver):
        if args.keep_consent:
            logger.debug('Consent dialog will not be auto-removed')
        else:
            kill_cookie_shit(browser)

        try:
            checkbox = browser.find_element(By.ID, "datadump-agree-checkbox")
            btn = browser.find_element(By.ID, "datadump-download-button")
        except NoSuchElementException:
            raise RuntimeError(f"Bad site: {site}")

        if args.dry_run:
            return

        browser.execute_script("""
        (function() {
            let oldFetch = window.fetch;
            window.fetch = (url, opts) => {
                let promise = oldFetch(url, opts);

                if (url.includes("/link")) {
                    promise.then(res => {
                        res.clone().json().then(json => {
                            window.extractedUrl = json["url"];
                            console.log(extractedUrl);
                        });
                        return res;
                    });
                    return new Promise(resolve => setTimeout(resolve, 4000))
                        .then(_ => promise);
                }
                return promise;
            };
        })();
        """)

        checkbox.click()
        sleep(1)
        btn.click()
        check_cloudflare_intercept(browser)
        sleep(2)
        url = browser.execute_script("return window.extractedUrl;")
        utils.extract_etag(url, etags)

        sleep(5)

    main_loaded = utils.is_file_downloaded(args.output_dir, site)
    meta_loaded = utils.is_file_downloaded(args.output_dir, meta_url)

    if not args.skip_loaded or not main_loaded or not meta_loaded:
        if args.skip_loaded and main_loaded:
            pass
        else:
            browser.get(f"{site}/users/data-dump-access/current")
            check_cloudflare_intercept(browser)

            if not args.dry_run:
                utils.archive_file(args.output_dir, site)

            _exec_download(browser)

        if args.skip_loaded and meta_loaded:
            pass
        elif meta_url is not None:
            browser.get(f"{meta_url}/users/data-dump-access/current")
            check_cloudflare_intercept(browser)

            if not args.dry_run:
                utils.archive_file(args.output_dir, meta_url)

            _exec_download(browser)

def await_date_change(args: SEDDCLIArgs, driver):
    site = "https://stackoverflow.com"

    failed: bool = False
    seconds_in_hour = 60 * 60
    while True:
        browser.get(f"{site}/users/data-dump-access/current")
        if "page not found" in browser.title.lower():
            if failed:
                raise RuntimeError("Looks like login failed")
            login_or_create(driver, site)
            check_cloudflare_intercept(driver)
            failed = True
            continue

        if browser.current_url.endswith("/error"):
            logger.error(
                "Redirected to /error; SE fucked something up. Update failed, "
                "trying again in 3 hours"
            )
            sleep(
                # 3 hours
                seconds_in_hour * 3
                # + some jitter
                + random.randint(
                    -seconds_in_hour,
                    seconds_in_hour
                )
            )
            continue

        failed = False

        try:
            info_elem = driver.find_elements(
                By.XPATH,
                "//*[contains(text(), 'Last uploaded: ')]"
            )
            if len(info_elem) != 1:
                raise RuntimeError("XPath returned with elements, but not just one")

            elem_content = info_elem[0].text.replace("Last uploaded: ", "")
            logger.debug("Found date {}", elem_content)
            if len(elem_content) == 0 or "<" in elem_content:
                raise RuntimeError("The date is no longer in plain text")
            elif not re.match(r'^[a-zA-Z]+ \d{1,2}, \d{1,4}$', elem_content):
                raise RuntimeError("The date format has changed")

            
            if args.detect in elem_content:
                logger.debug("Still not ready. Sleeping for approximately 6 hours")

                seconds_in_hour = 60 * 60
                sleep(
                    # 6 hours
                    seconds_in_hour * 6
                    # + some jitter
                    + random.randint(
                        -seconds_in_hour // 2,
                        seconds_in_hour // 2
                    )
                )
            else:
                logger.info("Data dump update found!")
                notifications.notify(
                    "The data dump has been updated to {} and will be downloaded".format(
                        elem_content
                    ),
                    sedd_config
                )
                break

        except:
            logger.error(
                "Looks like the data dump page markup has changed. This will "
                "likely require changes to sedd to fix. "
                "Please open an issue: https://github.com/LunarWatcher/se-data-dump-transformer/"
            )
            raise

def do_download(site: str):
    if site not in ["https://meta.stackexchange.com", "https://stackapps.com"]:
        # https://regex101.com/r/kG6nTN/1
        meta_url = re.sub(
            r"(https://(?:[^.]+\.(?=stackexchange))?)", r"\1meta.", site)
    else:
        meta_url = None

    main_loaded = utils.is_file_downloaded(args.output_dir, site)
    meta_loaded = utils.is_file_downloaded(args.output_dir, meta_url) or \
        meta_url is None

    if args.skip_loaded and main_loaded and meta_loaded:
        pass
    else:
        logger.info(f"Extracting from {site}...")

        login_or_create(browser, site)
        download_data_dump(
            browser,
            site,
            meta_url,
            etags
        )

def clear_part_files():
    files = glob.glob(
        os.path.join(
            args.output_dir,
            "*.part"
        )
    )

    for file in files:
        logger.debug(
            "Found existing .part file ({}). Removing...",
            file
        )
        os.remove(file)

def normalize_meta(url: str):
    if (url.startswith("meta.") and url != "meta.stackexchange.com"):
        return url.replace("meta.", "")
    return url

def try_recover_fucked_download(
        last_sizes: dict[str, LastState],
        state: DownloadState
):
    # TODO: This is a very naive approach. Can we get the information directly
    # form the webdriver?
    for path in state.pending:
        # The path is in the form of the full site URL with the .zip
        # The .part file is in the form `[first path component].[garbage].[rest]`
        # meta.stackexchange.com.7z would be 
        # meta.[garbage.]stackexchange.7z.part
        spl = path.split(".", 1)
        matches = glob.glob(
            os.path.join(
                args.output_dir,
                '.*.'.join(spl) + ".part"
            )
        )

        if len(matches) == 0:
            logger.warning("{} has 0 part files. Race condition?", path)
        elif len(matches) > 1:
            logger.error(
                "{} has more than 1 part file! Error recovery impossible",
                path
            )
        else:
            part_path = matches[0]
            size = os.path.getsize(part_path)

            last_size = last_sizes.get(
                path,
                LastState(monotonic(), 0)
            )

            # We only commit the size if it's changing
            if (last_size.last_observed_size < size):
                last_size.last_observed_size = size
                last_size.last_observed_change = monotonic()

                last_sizes[path] = last_size
            else:
                # If it isn't changing, we allow for a 60 second window 
                # After 30 seconds, a warning is issued. This is likely useless
                # in practice, because a failure this long almost certainly
                # means the download has died
                # After 60 seconds, we treat the download as failed and move on
                # with our lives. Nuke the part file, redo the download
                delta = monotonic() - last_size.last_observed_change

                if (delta > 60):
                    logger.error(
                        "{} has failed. {} has been pinned for 60 seconds. "
                        "Retrying",
                        path,
                        part_path
                    )
                    del last_sizes[path]

                    os.remove(part_path)
                    # TODO: optimally, we'd just call a single function that
                    # directly downloads the specific site. Unfortunately, the
                    # system wasn't set up to deal with this, and I don't feel
                    # like rewriting it when all I want is to download the god
                    # damn file
                    # The download system should be split up to allow for this,
                    # but it'll be a bigger refactor to do it. The current URL
                    # system is fairly fragile, really
                    do_download(
                        "https://" + normalize_meta(
                            path.replace(".7z", "")
                        )
                    )
                elif (
                    # the "and" is to avoid excessive spam. This allows a 2
                    # second window
                    delta > 30 and delta < 32
                ):
                    logger.warning(
                        "No observed change in {} for 30 seconds",
                        path
                    )

etags: Dict[str, str] = {}

try:
    state, observer = register_pending_downloads_observer(args.output_dir)
    if args.detect is not None:
        await_date_change(args, browser)
    if args.wipe_part_files:
        clear_part_files()
    for site in sites.sites:
        do_download(site)

    if observer:
        pending = state.size()

        logger.info(f"Waiting for {pending} download{'s'[:pending^1]} to complete")
        if args.wipe_part_files:
            logger.info("Download error recovery is enabled.")

        last_sizes = {}
        while True:
            if state.empty():
                observer.stop()
                browser.quit()

                utils.cleanup_archive(args.output_dir)
                break
            else:
                if args.wipe_part_files:
                    try_recover_fucked_download(
                        last_sizes,
                        state
                    )
                sleep(1)

    notifications.notify(
        "Data dump download done!",
        sedd_config
    )
except KeyboardInterrupt:
    pass

except:
    print_exception(sys.exception())
    browser.quit()
    exit(-1)
finally:
    # TODO: replace with validation once downloading is verified done
    # (or export for separate, later verification)
    # Though keeping it here, removing files and re-running downloads feels like a better idea
    print(etags)
