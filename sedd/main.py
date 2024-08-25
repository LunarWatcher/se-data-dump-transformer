from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.webdriver import WebDriver
from selenium.webdriver.firefox.options import Options
from selenium.common.exceptions import NoSuchElementException
from typing import Dict


from time import sleep
import urllib.request

import re
import os
import sys
from traceback import print_exception

import argparse


from .config import load_sedd_config
from .data import sites
from .meta import notifications
from .watcher.observer import register_pending_downloads_observer
from . import utils

parser = argparse.ArgumentParser(
    prog="sedd",
    description="Automatic (unofficial) SE data dump downloader for the anti-community data dump format",
)
parser.add_argument(
    "-s", "--skip-loaded",
    required=False,
    default=False,
    action="store_true",
    dest="skip_loaded"
)
parser.add_argument(
    "-o", "--outputDir",
    required=False,
    dest="output_dir",
    default=os.path.join(os.getcwd(), "downloads")
)
parser.add_argument(
    "--dry-run",
    required=False,
    default=False,
    action="store_true",
    dest="dry_run"
)

args = parser.parse_args()


def get_download_dir():
    download_dir = args.output_dir

    if not os.path.exists(download_dir):
        os.makedirs(download_dir)

    print(download_dir)

    return download_dir


options = Options()
options.enable_downloads = True
options.set_preference("browser.download.folderList", 2)
options.set_preference("browser.download.manager.showWhenStarting", False)
options.set_preference("browser.download.dir", get_download_dir())
options.set_preference(
    "browser.helperApps.neverAsk.saveToDisk", "application/x-gzip")

browser = webdriver.Firefox(
    options=options
)

sedd_config = load_sedd_config()

ubo_download_url = sedd_config.get_ubo_download_url()

if not os.path.exists("ubo.xpi"):
    print(f"Downloading uBO from: {ubo_download_url}")
    urllib.request.urlretrieve(ubo_download_url, "ubo.xpi")


ubo_id = browser.install_addon("ubo.xpi", temporary=True)


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
        print(f"Extracting from {site}...")

        if site not in ["https://meta.stackexchange.com", "https://stackapps.com"]:
            # https://regex101.com/r/kG6nTN/1
            meta_url = re.sub(
                r"(https://(?:[^.]+\.(?=stackexchange))?)", r"\1meta.", site)

        main_loaded = utils.is_file_downloaded(args.output_dir, site)
        meta_loaded = utils.is_file_downloaded(args.output_dir, meta_url)

        if args.skip_loaded and main_loaded and meta_loaded:
            pass
        else:
            login_or_create(browser, site)
            download_data_dump(
                browser,
                site,
                meta_url,
                etags
            )

    while True is True:
        sleep(1)

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
