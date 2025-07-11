from desktop_notifier import DesktopNotifier
import asyncio

from ..config import SEDDConfig
import requests
from loguru import logger


def native(message: str, _):
    asyncio.run(
        DesktopNotifier().send(
            title="The data dump downloader needs attention",
            message=f"{message}"
        )
    )

def ntfy(message: str, config):
    requests.post(
        config["url"],
        headers={
            "Authorization": f"Bearer {config['bearer']}",
            "Title": "Sedd update"
        },
        data=message,

    )

notification_providers = {
    "native": native,
    "ntfy": ntfy
}


def notify(message: str, config: SEDDConfig):

    provider = config.get_notifications_provider()

    if provider is None:
        logger.info(message)
        return

    notification_providers[provider](message, config.notifications)
