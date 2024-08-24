from threading import current_thread, main_thread
from watchdog.observers import Observer

from .handler import CleanupHandler
from .state import DownloadState


def register_pending_downloads_observer(output_dir: str):
    if current_thread() is main_thread():
        observer = Observer()
        state = DownloadState()
        handler = CleanupHandler(observer, state)

        observer.schedule(handler, output_dir, recursive=True)
        observer.start()

        return state, observer
