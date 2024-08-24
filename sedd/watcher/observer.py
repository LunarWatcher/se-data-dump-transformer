from threading import current_thread, main_thread

from .handler import CleanupHandler
from .state import DownloadState

# watchdog has trouble with new Python versions not having MutableSet
import collections
from sys import version_info

if version_info.major == 3 and version_info.minor >= 10:
    from collections.abc import MutableSet
    collections.MutableSet = collections.abc.MutableSet
else:
    from collections import MutableSet

from watchdog.observers import Observer


def register_pending_downloads_observer(output_dir: str):
    if current_thread() is main_thread():
        observer = Observer()
        state = DownloadState()
        handler = CleanupHandler(observer, state)

        observer.schedule(handler, output_dir, recursive=True)
        observer.start()

        return state, observer
