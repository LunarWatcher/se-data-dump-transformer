# Changelog

Only 2.2.0 and newer have explicitly recorded changelogs. For earlier versions, the releases may contain additional information.

## 2.4.1 (2026-01-06)

### Fixed

* Error recovery for meta URLs only worked for sites with their own domains, and meta.stackexchange.com, because I forgot `*.stackexchange.com` sites have metas in the form `sitename.meta.stackexchange.com` and not `meta.sitename.stackexchange.com`

## 2.4.0 (2026-01-06)

### Added

* `--no-wipe-part-files` and a system to deal with download error recovery by watching `.part` files. This allows downloads to deal with cloudflare and SE being flaky by retrying the download once it fails. The previous system was unable to detect failures
* Standalone doc file on error recovery methods and some known failure modes

## 2.3.0 (2025-08-24)

### Added
* Documentation containing all known hashes as of the 2024-06-30 data dumps (https://github.com/LunarWatcher/se-data-dump-transformer/blob/master/docs/meta/Hashes.md)
* Documentation for watermarking (https://github.com/LunarWatcher/se-data-dump-transformer/blob/master/docs/meta/Known%20watermarks.md)
* Transformer: `--filter-fabricated-v1` for removing the aforementioned v1 watermark.

### Changed
* Transformer (internals): `Filter::process` now takes an explicit argument containing the data dump filetype to allow filters operating only specific files.

## 2.2.2 (2025-07-16)

### Fixed
* `exc_info()` returns a tuple, so the `print_exception()` in `main.py` was always a noop. Though `exc_info()` does contain the right thing to pass to `print_exception()`, python 3.11 has `sys.exception()`. The three places using error logging now use `traceback.print_exception(sys.exception())` so the error is actually properly logged. This also bumps the requirement from 3.10 to 3.11

## 2.2.1 (2025-07-15)

### Fixed
* `--detect` mode crash when the data dump download page inexplicably 500s

## 2.2.0 (2025-07-11)

### Added
* `sedd` is now a script, so `python3 -m sedd` is no longer the only valid use form when installing from pypi, or when building and installing locally.
* `--detect <last upload date>` for automatically waiting for the data dump to update before proceeding with the download. For more details, please read [docs/Automated downloads.md](docs/Automated downloads.md) before using.
* `--unsupervised`: disables cloudflare notifications, resulting in a hard error instead. Useful if you're running with `xvfb-run` or whatever the fuck wayland has in a headless environment. If you're running on Linux or Windows, you probably won't run into unsolvable cloudflare blocks. If you do, you need to attach a display to solve the captchas.
* Notification provider `ntfy`. See `docs/Notifications.md` for information about it, and general notification setup.
* Extremely elementary captchca solver. All it does is try to click the checkbox. 
* Loguru for logging. A lot of places still use print statements, and these will be refactored Later:tm:

### Changed
* The changelog is now maintained in an actual file instead of in GH releases

### Fixed
* Login captcha wall resulting in a notification loop

