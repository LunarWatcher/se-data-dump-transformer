# Error recovery methods


## Capchas 

You may run into captchas, and sedd does try to "solve" it by clicking the checkbox. This semi-often fails since captchas are designed to prevent precisely this. 

No real captcha solving or bypassing is implemented. Instead, you'll get a notification when you have to solve a captcha that simply clicking the checkbox couldn't solve. 

Per the root-level README, you may run into captcha loops. Jumping on a VPN has been observed to help. Cloudflare has been set up to be intentionally aggressive in certain countries. It's unclear which or how many countries this affects.

## Download failures

The download is flaky and cannot be resumed. As of 2.4.0, sedd includes an attempt to identify such failures.

This relies on identifying `.part` files, so the first step involves deleting all `.part` files in a folder. **If you're using a shared download folder, you may want to disable this**, but this also disables error handling. It's strongly recommended to use a dedicated folder for the download to avoid problems. If you want to disable it, and recovery of this particular error with it, pass `--no-wipe-part-files`.

This particular system works because, on failure, the `.part` files are not deleted. Failure can therefore be identified by whether or not the `.part` file increases in size over a given window, which is hard-coded to 60 seconds to allow for various categories of networking blips that recover automatically.

As long as this error recovery method is enabled, on failure, the affected sites will be retried. Observationally, this primarily affects `stackoverflow.com.7z`, which also is the biggest one. On my stable fibre connection that hasn't dropped out on me in months, it often takes several tries and around 200-400GB in wasted bandwidth due to SE-sided failures.

You can partly mitigate this with a VPN for networking reasons I'm not going to pretend to understand, but there's limits to how much this helps. This error recovery system deals with the errors the way SE wants them to be handled in their new system; by redownloading the entire god damn thing until it eventually completes, since there's no other way to recover the download.

In the future, this system may be replaced by a non-webdriver client doing the actual download and an attempt to do byte-based resuming with a lot of client-sided hacks, but this is complicated enough that it's pushed _far_ down the road.

(I really miss the days of first-party torrents that were very trivially resumable.)

## Cloudflare outages

Unhandled at this time. Sedd will crash hard if this happens. This has only been observed once in 1.5 years through a pure coincidence blip occurring at the same time as a request to the download page.
