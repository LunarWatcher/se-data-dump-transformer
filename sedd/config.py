from typing import TypedDict, Literal

from json import load
from os import path, getcwd


class SEDDNotificationsConfig(TypedDict):
    provider: Literal['native'] | None


class SEDDUboConfig(TypedDict):
    download_url: str


default_ubo_url = "https://github.com/gorhill/uBlock/releases/download/1.59.0/uBlock0_1.59.0.firefox.signed.xpi"

default_notifications_config: SEDDNotificationsConfig = {
    'provider': None
}

default_ubo_config: SEDDUboConfig = {
    "download_url": default_ubo_url
}


class SEDDConfig:
    email: str
    password: str
    notifications: SEDDNotificationsConfig
    ubo: SEDDUboConfig

    def __init__(self, email: str, pwd: str, notifications: SEDDNotificationsConfig, ubo: SEDDUboConfig):
        self.email = email
        self.password = pwd
        self.notifications = notifications
        self.ubo = ubo

    def get_notifications_provider(self) -> Literal['native'] | None:
        notifications_config = self.notifications
        return notifications_config['provider'] if hasattr(notifications_config, 'provider') else None

    def get_ubo_download_url(self) -> str:
        ubo_config = self.ubo
        return ubo_config["download_url"] if hasattr(ubo_config, 'download_url') else default_ubo_url


def load_sedd_config() -> SEDDConfig:
    config_path = path.join(getcwd(), 'config.json')

    config: SEDDConfig = None

    with open(config_path, "r") as f:
        config = load(f)

        email = config["email"]
        password = config["password"]

        notifications = config['notifications'] if hasattr(
            config, 'notifications') else default_notifications_config

        ubo = config['ubo'] if hasattr(config, 'ubo') else default_ubo_config

        config = SEDDConfig(email, password, notifications, ubo)

    return config
