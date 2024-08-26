from selenium.webdriver import Firefox
from ..config.typings import SEDDUboSettings


def ubo_set_user_settings(browser: Firefox, settings: SEDDUboSettings) -> None:
    # using vAPI 'userSettings' message to set uBO's user settings
    # see: https://github.com/gorhill/uBlock/blob/master/src/js/settings.js#L215
    for key, val in settings['userSettings'].items():
        browser.execute_script("""
            const name = arguments[0];
            const value = arguments[1];

            globalThis.vAPI.messaging.send('dashboard', {
                what: 'userSettings', name, value,
            });
        """, key, val)


def ubo_set_advanced_settings(browser: Firefox, settings: SEDDUboSettings) -> None:
    # using vAPI 'writeHiddenSettings' message to set uBO's advanced settings
    # see: https://github.com/gorhill/uBlock/blob/master/src/js/advanced-settings.js#L177
    browser.execute_script("""
        const settings = arguments[0];

        const content = Object.entries(settings)
                           .map(([k,v]) => `${k} ${v}`)
                           .join('\\n');

        globalThis.vAPI.messaging.send('dashboard', {
            what: 'writeHiddenSettings', content,
        });
    """, settings['hiddenSettings'])


def ubo_set_selected_filters(browser: Firefox, settings: SEDDUboSettings) -> None:
    # using vAPI 'applyFilterListSelection' message to set uBO's selected filter lists
    # see: https://github.com/gorhill/uBlock/blob/master/src/js/storage.js#L486
    browser.execute_script("""
        const toSelect = arguments[0];

        globalThis.vAPI.messaging.send('dashboard', {
            what: 'applyFilterListSelection', toSelect,
        })
    """, settings['selectedFilterLists'])


def ubo_set_whitelist(browser: Firefox, settings: SEDDUboSettings) -> None:
    # using vAPI 'setWhitelist' message to set uBO's trusted sites list
    # see: https://github.com/gorhill/uBlock/blob/master/src/js/messaging.js#L225
    browser.execute_script("""
        const list = arguments[0];

        globalThis.vAPI.messaging.send('dashboard', {
            what: 'setWhitelist', whitelist: list.join('\\n')
        })
    """, settings["whitelist"])


def ubo_set_dynamic_rules(browser: Firefox, settings: SEDDUboSettings) -> None:
    # using vAPI 'modifyRuleset' message to set uBO's dynamic rules
    # see: https://github.com/gorhill/uBlock/blob/master/src/js/dyna-rules.js#L279
    browser.execute_script("""
        const { toAdd = [], toRemove = [] } = arguments[0]

        globalThis.vAPI.messaging.send('dashboard', {
            what: 'modifyRuleset', permanent: true,
            toAdd: toAdd.join('\\n'),
            toRemove: toRemove.join('\\n'),
        })

    """, settings["dynamicFilters"])


def ubo_set_user_filters(browser: Firefox, settings: SEDDUboSettings) -> None:
    # using vAPI 'writeUserFilters' message to set uBO's user filters
    # see: https://github.com/gorhill/uBlock/blob/master/src/js/storage.js#L582
    browser.execute_script("""
        const { trusted, enabled, toOverwrite = [] } = arguments[0]

        globalThis.vAPI.messaging.send('dashboard', {
            what: 'writeUserFilters', trusted, enabled,
            content: toOverwrite.join('\\n')
        })
    """, settings['userFilters'])
