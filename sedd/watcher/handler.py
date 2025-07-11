import os

from watchdog.observers.api import BaseObserverSubclassCallable
from watchdog.events import FileSystemEventHandler

from .state import DownloadState
from ..utils import is_dump_file

from loguru import logger

class CleanupHandler(FileSystemEventHandler):
    download_state: DownloadState
    observer: BaseObserverSubclassCallable

    def __init__(self, observer: BaseObserverSubclassCallable, state: DownloadState):
        super()

        self.download_state = state
        self.observer = observer

    def on_created(self, event):
        file_name = os.path.basename(event.src_path)

        # # we can safely ignore part file creations
        if file_name.endswith('.part'):
            return

        if is_dump_file(file_name):
            logger.info(f"Download started: {file_name}")
            self.download_state.add(file_name)

    def on_moved(self, event):
        file_name: str = os.path.basename(event.dest_path)

        # we can safely ignore part file removals
        if file_name.endswith('.part'):
            return

        if is_dump_file(file_name):
            logger.info(f"Download finished: {file_name}")
            self.download_state.remove(file_name)
