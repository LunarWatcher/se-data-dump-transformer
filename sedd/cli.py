import argparse

from os import getcwd
from os.path import join


class SEDDCLIArgs(argparse.Namespace):
    skip_loaded: bool
    keep_consent: bool
    output_dir: str
    dry_run: bool


parser = argparse.ArgumentParser(
    prog="sedd",
    description="Automatic (unofficial) SE data dump downloader for the anti-community data dump format",
)

parser.add_argument(
    "-s", "--skip-loaded",
    required=False,
    default=False,
    action="store_true",
    dest="skip_loaded"
)

parser.add_argument(
    "-k", "--keep-consent",
    required=False,
    dest="keep_consent",
    action="store_true",
    default=False
)

parser.add_argument(
    "-o", "--outputDir",
    required=False,
    dest="output_dir",
    default=join(getcwd(), "downloads")
)

parser.add_argument(
    "--dry-run",
    required=False,
    default=False,
    action="store_true",
    dest="dry_run"
)


def parse_cli_args() -> SEDDCLIArgs:
    return parser.parse_args()
