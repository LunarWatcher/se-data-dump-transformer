from typing import Literal

from json import load
from os import path, getcwd

from .defaults import default_ubo_url, default_ubo_settings, default_notifications_config, default_ubo_config
from .typings import SEDDNotificationsConfig, SEDDUboConfig, SEDDUboSettings


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
        return notifications_config['provider'] if 'provider' in notifications_config else None

    def get_ubo_download_url(self) -> str:
        ubo_config = self.ubo
        return ubo_config['download_url'] if 'download_url' in ubo_config else default_ubo_url

    def get_ubo_settings(self) -> SEDDUboSettings:
        ubo_config = self.ubo
        return ubo_config['settings'] if 'settings' in ubo_config else default_ubo_settings


def load_sedd_config() -> SEDDConfig:
    config_path = path.join(getcwd(), 'config.json')

    config: SEDDConfig = None

    with open(config_path, "r") as f:
        config = load(f)

        email = config["email"]
        password = config["password"]

        notifications = config['notifications'] if 'notifications' in config else default_notifications_config

        ubo = config['ubo'] if 'ubo' in config else default_ubo_config

        config = SEDDConfig(email, password, notifications, ubo)

    return config
