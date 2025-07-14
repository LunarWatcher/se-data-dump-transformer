from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.webdriver import WebDriver
from selenium.common.exceptions import NoSuchElementException
from typing import Dict


from time import sleep

import re
import sys
import random
from traceback import print_exception


from .cli import parse_cli_args, SEDDCLIArgs
from .config import load_sedd_config
from .data import sites
from .meta import notifications
from .watcher.observer import register_pending_downloads_observer
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


def download_data_dump(browser: WebDriver, site: str, meta_url: str, etags: Dict[str, str]):
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
        else:
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
        

etags: Dict[str, str] = {}

try:
    state, observer = register_pending_downloads_observer(args.output_dir)
    if args.detect is not None:
        await_date_change(args, browser)

    for site in sites.sites:
        if site not in ["https://meta.stackexchange.com", "https://stackapps.com"]:
            # https://regex101.com/r/kG6nTN/1
            meta_url = re.sub(
                r"(https://(?:[^.]+\.(?=stackexchange))?)", r"\1meta.", site)

        main_loaded = utils.is_file_downloaded(args.output_dir, site)
        meta_loaded = utils.is_file_downloaded(args.output_dir, meta_url)

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

    if observer:
        pending = state.size()

        logger.info(f"Waiting for {pending} download{'s'[:pending^1]} to complete")

        while True:
            if state.empty():
                observer.stop()
                browser.quit()

                utils.cleanup_archive(args.output_dir)
                break
            else:
                sleep(1)

    notifications.notify(
        "Data dump download done!",
        sedd_config
    )
except KeyboardInterrupt:
    pass

except:
    exception = sys.exc_info()

    try:
        print_exception(exception)
    except:
        logger.error(exception)

    browser.quit()
    exit(-1)
finally:
    # TODO: replace with validation once downloading is verified done
    # (or export for separate, later verification)
    # Though keeping it here, removing files and re-running downloads feels like a better idea
    print(etags)
