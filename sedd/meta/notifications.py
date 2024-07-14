from desktop_notifier import DesktopNotifier
import asyncio

def native(message: str, _):
    asyncio.run(
        DesktopNotifier().send(
            title = "The data dump downloader needs attention",
            message = f"{message}"
        )
    )

notification_providers = {
    "native": native
}
def notify(message: str, config):

    provider = config["notifications"]["provider"]
    if provider is None:
        print(message)
        return

    notification_providers[provider](message, config["notifications"])


