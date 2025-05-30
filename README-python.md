# SE Data Dump Downloader


For more comprehensive information, please read the [main README](https://github.com/LunarWatcher/se-data-dump-transformer/tree/master) on GitHub. This README contains an abridged version of the main README specifically aimed at Pypi users. 

For usage problems not listed in this readme, see the main README. If no information exists, please open an issue on GitHub - keeping the tool accessible to everyone is a priority.

---

The SE Data Dump Downloader (abbreviated `sedd`) is a command line Selenium-based utility for downloading the entire Stack Exchange data dump in their new [anti-community format](https://stackoverflow.com/help/data-dumps), since they decided not to bother providing an official "download all" button. It's one of two components that operate on the data dump in the second project, the other being the (non-python-based) SE data dump transformer - a project that converts the data dump from the not-so-useful official `.xml` format to some other formats. The pypi package is exclusively for the downloader, and does not ship with a copy of the transformer. See the main README if you're looking for the transformer.


For the pypi version, you can download it with:
```python3
pip3 install sedd
```

Note that there are some additional steps before you can start using it, that are detailed in this README.

## Configuration

`sedd` requires a special `config.json` file in the current working directory. There's a template available [on GitHub](https://github.com/LunarWatcher/se-data-dump-transformer/blob/master/config.example.json).

The only two fields you _need_ to fill out in the template is the email and password fields with credentials for a Stack Exchange account. You need to be logged in to download the data dumps, so the downloader needs the credentials to log in on your behalf. It doesn't matter if you're logged into SE elsewhere, as Selenium automatically creates a blank profile every time it starts, which won't include any cookies from SE, which means login is required.

> [!tip]
>
> The downloader can automatically create new accounts in the network for you, if you don't have all 180-whatever accounts on every site in the network already. You can also create these by hand if you prefer for some reason, but you are not required to have all 180+ accounts before using the downloader.

## System requirements and pitfalls

`sedd` is exclusively Firefox-based, due to Chromium completely gutting support for uBlock Origin and custom filters. You need Firefox installed on your system to use `sedd`.

> [!note]
> On Linux and Windows-based systems, geckodriver is [slightly modified](https://pypi.org/project/undetected-geckodriver-lw/). This is an anti-anti-bot measure meant to prevent Cloudflare loops. If you're on macOS and get sent in a captcha loop, it's recommended you switch to Windows or Linux - a Linux VM is also an option if you have no way out of Apple's closed-down ecosystem.

Note that Ubuntu users, or other people who (for whatever reason) choose to use the Snap version of Firefox, have to jump through some extra hoops. The native version of Firefox is strongly encouraged, but if you run into problems with the snap version of Firefox and can't or won't switch, you need to define `export SE_GECKODRIVER=/snap/bin/geckodriver`. Selenium can and will find the snap version of `geckodriver` on its own, but for reasons I simply don't understand, it will still fail with several arbitrary errors. 

### Cloudflare issues or download issues.

Stack Exchange has configured Cloudflare to be _highly_ aggressive, especially to certain countries. You will almost certainly run into captchas, and the downloader is designed to deal with this. After an initial attempt  to solve the captcha on its own, you'll be notified (provided you don't disable the notification provider in `config.json`) and asked to solve it manually. 

If, at this point, it appears to succeed, but you're redirected back  to a full-screen Cloudflare captcha wall, you've likely run into a Cloudflare loop. See [the main README](https://github.com/LunarWatcher/se-data-dump-transformer/tree/master?tab=readme-ov-file#cloudflare-loops) for further help. If this doesn't help, please open an issue.

If the downloads start fine, but later suddenly fail for no good reason, you're likely running into general download instability. This especially applies to `stackoverflow.com.7z`, as its massive size simply increases the chance you wait for it long enough that it flakes out. See [the main README](https://github.com/LunarWatcher/se-data-dump-transformer/tree/master?tab=readme-ov-file#download-instability-particularly-of-stackoverflowcom7z) for further help.

The "Warnings" section in the README may contain additional information about other failure modes not listed here in the future. 

## Using the downloader

With `./config.json` in the current working directory and Firefox installed, you can now run the downloader with:
```python3
sedd 
```

For command line flags, see `sedd --help`, or [the main readme](https://github.com/LunarWatcher/se-data-dump-transformer/tree/master?tab=readme-ov-file#cli-options).

