from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.firefox.webdriver import WebDriver
from selenium.webdriver.firefox.options import Options

from sedd.data import sites
from time import sleep
import json

from .meta import notifications

options = Options()
options.set_preference("browser.download.folderList", 2)
options.set_preference("browser.download.manager.showWhenStarting", False)
options.set_preference("browser.download.dir", "./downloads")
options.set_preference("browser.helperApps.neverAsk.saveToDisk", "application/x-gzip")

browser = webdriver.Firefox(
    options = options
)


with open("config.json", "r") as f:
    config = json.load(f)

email = config["email"]
password = config["password"]

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

            browser.find_element(By.ID, "submit-button").click()

            captchaWalled = False
            while "/nocaptcha" in browser.current_url:
                if not captchaWalled:
                    captchaWalled = True

                notifications.notify("Captcha wall hit during login", config)
                sleep(10)

            if captchaWalled:
                continue

            if not is_logged_in(browser, site):
                raise RuntimeError("Login failed")

            break


def download_data_dump(browser: WebDriver, site: str):
    print("Downloading is not yet implemented")

for site in sites.sites:
    print(f"Extracting from {site}...")

    login_or_create(browser, site)
    download_data_dump(
        browser,
        site
    )
