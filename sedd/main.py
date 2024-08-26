from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.webdriver import WebDriver
from selenium.common.exceptions import NoSuchElementException
from typing import Dict


from time import sleep

import re
import sys
from traceback import print_exception


from .cli import parse_cli_args
from .config import load_sedd_config
from .data import sites
from .meta import notifications
from .watcher.observer import register_pending_downloads_observer
from . import utils

from .driver import init_output_dir, init_firefox_driver

args = parse_cli_args()

sedd_config = load_sedd_config()

output_dir = init_output_dir(args.output_dir)

browser, ubo_id = init_firefox_driver(sedd_config, output_dir)


def kill_cookie_shit(browser: WebDriver):
    sleep(3)
    browser.execute_script(
        """let elem = document.getElementById("onetrust-banner-sdk"); if (elem) { elem.parentNode.removeChild(elem); }""")
    sleep(1)


def is_logged_in(browser: WebDriver, site: str):
    url = f"{site}/users/current"
    browser.get(url)
    sleep(1)

    return "/users/" in browser.current_url


def login_or_create(browser: WebDriver, site: str):
    if is_logged_in(browser, site):
        print("Already logged in")
    else:
        print("Not logged in and/or not registered. Logging in now")
        while True:
            browser.get(f"{site}/users/login")

            if "?newreg" in browser.current_url:
                print(f"Auto-created {site} without login needed")
                break

            email_elem = browser.find_element(By.ID, "email")
            password_elem = browser.find_element(By.ID, "password")
            email_elem.send_keys(sedd_config.email)
            password_elem.send_keys(sedd_config.password)

            curr_url = browser.current_url
            browser.find_element(By.ID, "submit-button").click()
            while browser.current_url == curr_url:
                sleep(3)

            captcha_walled = False
            while "/nocaptcha" in browser.current_url:
                if not captcha_walled:
                    captcha_walled = True

                notifications.notify(
                    "Captcha wall hit during login", sedd_config
                )

                sleep(10)

            if captcha_walled:
                continue

            if not is_logged_in(browser, site):
                raise RuntimeError("Login failed")

            break


def download_data_dump(browser: WebDriver, site: str, meta_url: str, etags: Dict[str, str]):
    print(f"Downloading data dump from {site}")

    def _exec_download(browser: WebDriver):
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

            if not args.dry_run:
                utils.archive_file(args.output_dir, site)

            _exec_download(browser)

        if args.skip_loaded and meta_loaded:
            pass
        else:
            browser.get(f"{meta_url}/users/data-dump-access/current")

            if not args.dry_run:
                utils.archive_file(args.output_dir, meta_url)

            _exec_download(browser)


etags: Dict[str, str] = {}

try:
    state, observer = register_pending_downloads_observer(args.output_dir)

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
            print(f"Extracting from {site}...")

            login_or_create(browser, site)
            download_data_dump(
                browser,
                site,
                meta_url,
                etags
            )

    if observer:
        pending = state.size()

        print(f"Waiting for {pending} download{'s'[:pending^1]} to complete")

        while True:
            if state.empty():
                observer.stop()
                browser.quit()

                utils.cleanup_archive(args.output_dir)
                break
            else:
                sleep(1)

except KeyboardInterrupt:
    pass

except:
    exception = sys.exc_info()

    try:
        print_exception(exception)
    except:
        print(exception)

    browser.quit()
finally:
    # TODO: replace with validation once downloading is verified done
    # (or export for separate, later verification)
    # Though keeping it here, removing files and re-running downloads feels like a better idea
    print(etags)
