import argparse

from os import getcwd
from os.path import join


class SEDDCLIArgs(argparse.Namespace):
    skip_loaded: bool
    keep_consent: bool
    output_dir: str
    dry_run: bool
    disable_undetected: bool
    verbose: bool
    detect: str | None
    unsupervised: bool

    wipe_part_files: bool


parser = argparse.ArgumentParser(
    prog="sedd",
    description="Automatic (unofficial) SE data dump downloader for the anti-community data dump format",
)

parser.add_argument(
    "-s", "--skip-loaded",
    required=False,
    default=False,
    action="store_true",
    dest="skip_loaded",
    help="Whether or not to skip files that already exist during the download process"
)

parser.add_argument(
    "-k", "--keep-consent",
    required=False,
    dest="keep_consent",
    action="store_true",
    default=False,
)

parser.add_argument(
    "-o", "--outputDir",
    required=False,
    dest="output_dir",
    default=join(getcwd(), "downloads"),
    help="Where to output the downloads to"
)

parser.add_argument(
    "-g", "--disable-undetected-geckodriver",
    required=False,
    dest="disable_undetected",
    action="store_true",
    default=False,
    help=(
        "Whether or not to enable undetected-geckodriver-lw, which is mostly "
        "the same as the normal geckodriver, but hides webdriver information "
        "to make the downloads more reliable."
    )
)

parser.add_argument(
    "--dry-run",
    required=False,
    default=False,
    action="store_true",
    dest="dry_run",
    help="Goes through all the same steps as a real download, but without actually downloading anything"
)
parser.add_argument(
    "-v",
    required=False,
    default=False,
    action="store_true",
    dest="verbose",
    help=(
        "Enable verbose logging. Only affects selenium and potentially "
        "some other dependencies using Python's `logging` package"
    )
)

parser.add_argument(
    "-N,--no-wipe-part-files",
    required=False,
    default=True,
    action="store_false",
    dest="wipe_part_files",
    help=(
        "When supplied, .part files in the folder won't be wiped. "
        "By default, wiping part files is enabled, as leaving it interferes "
        "with a download error recovery mechanism. Supplying this flag "
        "disables download error recovery, as the system has no way to "
        "differentiate between a .part file that was there before the "
        "download started"
    )
)

parser.add_argument(
    "--detect",
    required=False,
    default=None,
    dest="detect",
    help=(
        "When supplied with a month or a full timestamp corresponding to the "
        "previous data dump upload, sedd waits for the upload date to change "
        "before downloading"
    )
)
parser.add_argument(
    "-u,--unsupervised",
    required=False,
    default=False,
    action="store_true",
    dest="unsupervised",
    help="Disables cloudflare notifications, and hard errors instead"
)


def parse_cli_args() -> SEDDCLIArgs:
    return parser.parse_args()
