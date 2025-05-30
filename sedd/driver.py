from os import path, makedirs
from urllib import request
from json import dumps
from uuid import uuid4
import platform

from selenium import webdriver
from selenium.webdriver.firefox.options import Options
from undetected_geckodriver import Firefox as UFirefox

from .config import SEDDConfig
from .ubo import init_ubo_settings


def init_output_dir(output_dir: str):
    if not path.exists(output_dir):
        makedirs(output_dir)

    print(output_dir)

    return output_dir


def init_firefox_driver(config: SEDDConfig, disable_undetected: bool, output_dir: str):
    options = Options()
    options.enable_downloads = True
    options.set_preference("browser.download.folderList", 2)
    options.set_preference("browser.download.manager.showWhenStarting", False)
    options.set_preference("browser.download.dir", output_dir)
    options.set_preference(
        "browser.helperApps.neverAsk.saveToDisk", "application/x-gzip"
    )

    # our own uuid for uBO so as we don't need to do the dance of inspecing internals
    ubo_internal_uuid = f"{uuid4()}"

    options.set_preference("extensions.webextensions.uuids", dumps(
        {"uBlock0@raymondhill.net": ubo_internal_uuid}))

    is_apple = platform.system() == "Darwin"
    use_undetected = not disable_undetected and not is_apple
    if use_undetected:
        print("Using undetected-geckodriver")
        browser = UFirefox(options = options)
    else:
        print("Warning: using standard geckodriver. Cloudflare may perpetually block you")
        if is_apple:
            print("This option is forced on macOS. For undetected_geckodriver, "
                  "run the downloader in a Linux or Windows environment.")
        browser = webdriver.Firefox(options=options)

    ubo_download_url = config.get_ubo_download_url()

    if not path.exists("ubo.xpi"):
        print(f"Downloading uBO from: {ubo_download_url}")
        request.urlretrieve(ubo_download_url, "ubo.xpi")

    ubo_id = browser.install_addon("ubo.xpi", temporary=True)

    init_ubo_settings(browser, config, ubo_internal_uuid)

    return browser, ubo_id
