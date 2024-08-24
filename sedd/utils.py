from typing import Dict
import requests as r
from urllib.parse import urlparse
import os.path
import re


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


def get_file_name(site_or_url: str):
    file_name = f"{re.sub(r'https://', '', site_or_url)}.7z"

    file_name = re.sub(r'^alcohol', 'beer', file_name)
    file_name = re.sub(r'^mattermodeling', 'materials', file_name)
    file_name = re.sub(r'^communitybuilding', 'moderators', file_name)
    file_name = re.sub(r'^medicalsciences', 'health', file_name)
    file_name = re.sub(r'^psychology', 'cogsci', file_name)
    file_name = re.sub(r'^writing', 'writers', file_name)
    file_name = re.sub(r'^video', 'avp', file_name)
    file_name = re.sub(r'^meta\.(es|ja|pt|ru)\.', r'\1.meta.', file_name)

    return file_name


def check_file(base_path: str, file_name: str):
    try:
        res = os.stat(os.path.join(base_path, file_name))
        return res.st_size > 0
    except FileNotFoundError:
        return False


def remove_old_file(base_path: str, site_or_url: str):
    try:
        file_name = get_file_name(site_or_url)
        os.remove(os.path.join(base_path, file_name))
    except FileNotFoundError:
        pass


def is_file_downloaded(base_path: str, site_or_url: str):
    file_name = get_file_name(site_or_url)
    return check_file(base_path, file_name)
