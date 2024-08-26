from os import path, makedirs
from urllib import request

from selenium import webdriver
from selenium.webdriver.firefox.options import Options

from .config import SEDDConfig


def init_output_dir(output_dir: str):
    if not path.exists(output_dir):
        makedirs(output_dir)

    print(output_dir)

    return output_dir


def init_firefox_driver(config: SEDDConfig, output_dir: str):
    options = Options()
    options.enable_downloads = True
    options.set_preference("browser.download.folderList", 2)
    options.set_preference("browser.download.manager.showWhenStarting", False)
    options.set_preference("browser.download.dir", output_dir)
    options.set_preference(
        "browser.helperApps.neverAsk.saveToDisk", "application/x-gzip")

    browser = webdriver.Firefox(
        options=options
    )

    ubo_download_url = config.get_ubo_download_url()

    if not path.exists("ubo.xpi"):
        print(f"Downloading uBO from: {ubo_download_url}")
        request.urlretrieve(ubo_download_url, "ubo.xpi")

    ubo_id = browser.install_addon("ubo.xpi", temporary=True)

    return browser, ubo_id
