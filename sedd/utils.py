from typing import Dict
import requests as r
from urllib.parse import urlparse
import os.path

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

