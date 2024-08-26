from selenium.webdriver import Firefox
from sys import exc_info
from time import sleep

from ..config import SEDDConfig
from .utils import ubo_set_user_settings, \
    ubo_set_advanced_settings, ubo_set_selected_filters, \
    ubo_set_whitelist, ubo_set_dynamic_rules, ubo_set_user_filters, \
    ubo_reload_all_filters


def init_ubo_settings(browser: Firefox, config: SEDDConfig, ubo_id: str) -> bool:
    try:
        browser.get(
            f'moz-extension://{ubo_id}/dashboard.html#settings.html'
        )

        settings = config.get_ubo_settings()

        ubo_set_user_settings(browser, settings)
        ubo_set_advanced_settings(browser, settings)

        # idk why, but applyFilterListSelection only works after a delay
        sleep(1)

        ubo_set_selected_filters(browser, settings)
        ubo_set_whitelist(browser, settings)
        ubo_set_dynamic_rules(browser, settings)
        ubo_set_user_filters(browser, settings)

        ubo_reload_all_filters(browser)
    except:
        print('Failed to set uBLock config, using defaults')
        print(exc_info())
