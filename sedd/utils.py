from typing import Dict
import requests as r
from urllib.parse import urlparse
import os.path
import re
import sys

from .data.files_map import files_map, inverse_files_map
from .data.sites import sites


def extract_etag(url: str, etags: Dict[str, str]):
    res = r.get(
        url,
        stream=True
    )
    if res.status_code != 200:
        raise RuntimeError(f"Panic: failed to get {url}: {res.status_code}")

    etag = res.headers["ETag"]
    res.close()

    parsed_url = urlparse(url)
    path = parsed_url.path
    filename = os.path.basename(path)

    etags[filename] = etag

    print(f"ETag for {filename}: {etag}")


def get_file_name(site_or_url: str) -> str:
    domain = re.sub(r'https://', '', site_or_url)

    try:
        file_name = files_map[domain]
        return f'{file_name}.7z'
    except KeyError:
        return f'{domain}.7z'


def is_dump_file(file_name: str) -> bool:
    file_name = re.sub(r'\.7z$', '', file_name)

    try:
        inverse_files_map[file_name]
    except KeyError:
        origin = f'https://{file_name}'
        return origin in sites

    return True


def check_file(base_path: str, file_name: str) -> bool:
    try:
        res = os.stat(os.path.join(base_path, file_name))
        return res.st_size > 0
    except FileNotFoundError:
        return False


def archive_file(base_path: str, site_or_url: str) -> None:
    try:
        file_name = get_file_name(site_or_url)
        file_path = os.path.join(base_path, file_name)
        os.rename(file_path, f"{file_path}.old")
    except FileNotFoundError:
        pass


def cleanup_archive(base_path: str) -> None:
    try:
        file_entries = os.listdir(base_path)

        for entry in file_entries:
            if entry.endswith('.old'):
                entry_path = os.path.join(base_path, entry)
                os.remove(entry_path)
    except:
        print(sys.exc_info())


def is_file_downloaded(base_path: str, site_or_url: str) -> bool:
    file_name = get_file_name(site_or_url)
    return check_file(base_path, file_name)
