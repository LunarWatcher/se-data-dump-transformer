from desktop_notifier import DesktopNotifier
import asyncio

from ..config import SEDDConfig


def native(message: str, _):
    asyncio.run(
        DesktopNotifier().send(
            title="The data dump downloader needs attention",
            message=f"{message}"
        )
    )


notification_providers = {
    "native": native
}


def notify(message: str, config: SEDDConfig):

    provider = config.get_notifications_provider()

    if provider is None:
        print(message)
        return

    notification_providers[provider](message, config["notifications"])
