# Changelog

Only 2.2.0 and newer have explicitly recorded changelogs. For earlier versions, the releases may contain additional information.

## 2.2.0 (unreleased)

### Added
* `sedd` is now a script, so `python3 -m sedd` is no longer the only valid use form when installing from pypi, or when building and installing locally.
* `--detect <last upload date>` for automatically waiting for the data dump to update before proceeding with the download. For more details, please read [docs/Automated downloads.md](docs/Automated downloads.md) before using.
* `--unsupervised`: disables cloudflare notifications, resulting in a hard error instead. Useful if you're running with `xvfb-run` or whatever the fuck wayland has in a headless environment. If you're running on Linux or Windows, you probably won't run into unsolvable cloudflare blocks. If you do, you need to attach a display to solve the captchas.
* Notification provider `ntfy`. See `docs/Notifications.md` for information about it, and general notification setup.
* Extremely elementary captchca solver. All it does is try to click the checkbox. 
* Loguru for logging. A lot of places still use print statements, and these will be refactored Later:tm:

### Fixed
* Login captcha wall resulting in a notification loop

