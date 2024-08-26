from .typings import SEDDNotificationsConfig, SEDDUboSettings, SEDDUboConfig

default_ubo_url = "https://github.com/gorhill/uBlock/releases/download/1.59.0/uBlock0_1.59.0.firefox.signed.xpi"

default_notifications_config: SEDDNotificationsConfig = {
    'provider': None
}

default_ubo_settings: SEDDUboSettings = {
    'userSettings': {
        'advancedUserEnabled': True,
    },
    "hiddenSettings": {},
    "selectedFilterLists": [
        "user-filters",
        "ublock-filters",
        "ublock-badware",
        "ublock-privacy",
        "ublock-unbreak",
        "ublock-quick-fixes",
        "easylist",
        "easyprivacy",
        "urlhaus-1",
        "plowe-0"
    ],
    "whitelist": [
        "chrome-extension-scheme",
        "moz-extension-scheme"
    ],
    "dynamicFilters": {
        "toAdd": [],
        "toRemove": []
    },
    "userFilters": {
        "enabled": False,
        "trusted": False,
        "toOverwrite": []
    }
}

default_ubo_config: SEDDUboConfig = {
    'download_url': default_ubo_url,
    'settings': default_ubo_settings
}
