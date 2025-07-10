# Automated (ish) downloads [not implemented, spec only]

As of 2.2.0, `sedd` features an extra `--detect` flag. This flag takes a version string corresponding to what it said last on the data dump download page. Once it's detected that this version changes, the download continues as usual, then `sedd` quits. 

This option primarily exists to remove the need for separate notification systems that let you know if there's an update or not, as these systems have become increasingly fragile due to SE's increasingly aggressive Cloudflare configuration. The intended use-case is to manually start up a `--detect` process every quarter. 

The update also comes with an extra flag:
* `--unsupervised`, a boolean flag that disables notifications asking you to deal with cloudflare. If you're blocked by Cloudflare and automatic resolution fails, the rest of the download will also fail.

Additionally, upgrades have been made to the download completion supervision system to try to offer more robust download management. Before this update, it failed to detect and recover from failed downloads, and required manually interrupting, clearing `.part` files, and then restarting. It has been able to skip completed downloads for some time, so this part is still relatively targeted. However, considering their setup is incredibly flaky, especially when downloading the &gt;60GB `stackoverflow.com.7z` can arbitrarily fail, this is just general QOL.

The goal of `--detect` and `--unsupervised` is to further reduce the manual work required, but not to fully automate it for all eternity. `--detect` does not track future changes; it's only good for one data dump release. In general, you sho

## Usage examples
