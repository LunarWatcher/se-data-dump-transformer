from typing import Set


class DownloadState:
    # list of filenames pending download
    pending: Set[str] = set()

    def size(self):
        return len(self.pending)

    def empty(self):
        return self.size() == 0

    def add(self, file: str):
        self.pending.add(file)

    def remove(self, file: str):
        self.pending.remove(file)


download_state = DownloadState()
