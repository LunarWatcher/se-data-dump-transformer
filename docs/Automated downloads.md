# Automated (ish) downloads [not implemented, spec only]

As of 2.2.0, `sedd` features an extra `--detect` flag. The `--detect` flag takes a string referring to the **last upload date**, not the expected upload date. This is because the last upload date is set in stone, while the upload of the next data dump can be on any day. Unfortunately, the data dump access page does not specify the data dump version, just the upload date.

Suggested shorthands for this flag are:
* `Jan`
* `Apr`
* `Jul`
* `Oct`
You can also manually go to the data dump access page and copy the "Last uploaded" date. There's no restrictions on what's put in the string, but there is an attempt to verify that you've put in something sane. Examples (pypi-based installation syntax; add `python3 -m` in front for non-pypi installs):

* `sedd --detect Apr`
* `sedd --detect "Apr 01, 2025"`

Usually, these options are identical. The only major difference is whether or not the style can be used if there's two data dumps uploaded in the same month.

Once it's detected that this version changes, the download continues as usual, then `sedd` quits. The refresh interval is approximately every 6 hours (+-1 hour) to reduce the chance of cloudflare interference.

This option primarily exists to remove the need for separate notification systems that let you know if there's an update or not, as these systems have become increasingly fragile due to SE's increasingly aggressive Cloudflare configuration. The intended use-case is to manually start up a `--detect` process every quarter. 

The update also comes with an extra flag:
* `--unsupervised`, a boolean flag that disables notifications asking you to deal with cloudflare. If you're blocked by Cloudflare and automatic resolution fails, the download will be considered failed.

The goal of `--detect` and `--unsupervised` is to further reduce the manual work required, but not to fully automate it for all eternity. `--detect` does not track future changes; it's only good for one data dump release. In general, you shouldn't run `--detect` for months; SE has a lot of dials they could fuck around with that increase the chance sedd gets fully blocked to the point of being unusable.
