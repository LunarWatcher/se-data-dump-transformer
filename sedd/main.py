import selenium
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.webdriver import WebDriver
from selenium.webdriver.firefox.options import Options

from sedd.data import sites
from time import sleep
import json
import urllib.request

from .meta import notifications
import re
import os

import argparse

parser = argparse.ArgumentParser(
    prog="sedd",
    description="Automatic (unofficial) SE data dump downloader for the anti-community data dump format",
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
options.set_preference("browser.download.folderList", 2)
options.set_preference("browser.download.manager.showWhenStarting", False)
options.set_preference("browser.download.dir", get_download_dir())
options.set_preference("browser.helperApps.neverAsk.saveToDisk", "application/x-gzip")

browser = webdriver.Firefox(
    options = options
)
if not os.path.exists("ubo.xpi"):
    print("Downloading uBO")
    urllib.request.urlretrieve(
        "https://github.com/gorhill/uBlock/releases/download/1.59.0/uBlock0_1.59.0.firefox.signed.xpi",
        "ubo.xpi"
    )


ubo_id = browser.install_addon("ubo.xpi", temporary=True)

with open("config.json", "r") as f:
    config = json.load(f)

email = config["email"]
password = config["password"]

def kill_cookie_shit(browser: WebDriver):
    sleep(3)
    browser.execute_script("""let elem = document.getElementById("onetrust-banner-sdk"); if (elem) { elem.parentNode.removeChild(elem); }""")
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
            email_elem.send_keys(email)
            password_elem.send_keys(password)

            curr_url = browser.current_url
            browser.find_element(By.ID, "submit-button").click()
            while browser.current_url == curr_url:
                sleep(3)

            captcha_walled = False
            while "/nocaptcha" in browser.current_url:
                if not captcha_walled:
                    captcha_walled = True

                notifications.notify("Captcha wall hit during login", config)
                sleep(10)

            if captcha_walled:
                continue

            if not is_logged_in(browser, site):
                raise RuntimeError("Login failed")

            break


def download_data_dump(browser: WebDriver, site: str):
    print(f"Downloading data dump from {site}")

    def _exec_download(browser: WebDriver):
        kill_cookie_shit(browser)
        try:
            checkbox = browser.find_element(By.ID, "datadump-agree-checkbox")
            btn = browser.find_element(By.ID, "datadump-download-button")
        except selenium.common.exceptions.NoSuchElementException:
            raise RuntimeError(f"Bad site: {site}")

        if args.dry_run:
            return True

        checkbox.click()
        sleep(1)
        btn.click()
        sleep(4)

        return True

    browser.get(f"{site}/users/data-dump-access/current")
    # Assumes access has not been granted yet
    if not _exec_download(browser):
        return

    if site not in ["https://meta.stackexchange.com", "https://stackapps.com"]:
        # https://regex101.com/r/kG6nTN/1
        meta_url = re.sub(r"(https://(?:[^.]+\.(?=stackexchange))?)", r"\1meta.", site)
        print(meta_url)
        browser.get(f"{meta_url}/users/data-dump-access/current")
        _exec_download(browser)

for site in sites.sites:
    print(f"Extracting from {site}...")

    login_or_create(browser, site)
    download_data_dump(
        browser,
        site
    )
