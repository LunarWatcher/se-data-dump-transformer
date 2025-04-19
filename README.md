# Stack Exchange data dump downloader and transformer

[![Data dump transformer build](https://github.com/LunarWatcher/se-data-dump-transformer/actions/workflows/transformer.yml/badge.svg)](https://github.com/LunarWatcher/se-data-dump-transformer/actions/workflows/transformer.yml) [![Stackapps listing](https://img.shields.io/badge/StackApps%20listing-FF9900)](https://stackapps.com/q/10591/69829)

**Disclaimer:** This project is not affiliated with Stack Exchange, Inc.

## Background

This section contains background on why this project exists. If you know and/or don't care, feel free to skip to the next section.

In June 2023, Stack Exchange [briefly cancelled the data dump](https://meta.stackexchange.com/q/389922/332043), and backpedalled after a significant amount of backlash from the community. The status-quo of uploads to archive.org was restored. In the slightly more than a year between June 2023 and July 2024, it looked like they were staying off that path. Notably, they made a [logical shift to the _exact_ dates involved in the upload](https://meta.stackexchange.com/q/398279/) to deal with archive.org being slow. In December 2023, they [announced a delay in the upload](https://meta.stackexchange.com/q/395197/) likely to avoid speculation that another cancellation was happening.

We appeared to be out of the woods. But this repo wouldn't exist if that was the case now, would it?

### 2024 data dump restriction attempt

In July 2024, Stack Exchange announced the first restrictions on the data dump, by moving it in-house and actively discouraging archive.org reuploads, [likely in violation of the CC-By-SA license](https://meta.stackexchange.com/a/401326/332043)

The current revision can be read [here](https://meta.stackexchange.com/q/401324/332043)

Here's what's happening:

- SE is moving the data dump from archive.org to their own infrastructure
- They're discontinuing the archive.org dump, which makes it significantly harder to archive the data if SE, for example, were to go out of business
  - In addition to discontinuing the archive.org dump, they're imposing significant restrictions on the data dump
- They're doing the first revision **without the possibility to download the entire data dump** in one click, a drastic QOL reduction from the current situation.

This is an opinionated summary of the reason why; SE wants to capitalise on AI companies that need training data, and have decided that the community doesn't matter in that process. The current process, while not nearly as restricting as rev. 1 and 2, is a symptom of precisely one thing; Stack Exchange doesn't care about its users, but rather cares about finding new ways to profit off user data.

**Stack Exchange, Inc. is now the single biggest threat to the community**, and to the platform's user-generated and [permissively-licensed content](https://stackoverflow.com/help/licensing) that the community has spent countless hours creating precisely _because_ the data is public.

That is why this project exists; this is meant to automate the data dump download process for non-commercial license-compliant use, since Stack Exchange, Inc. couldn't be bothered adding a "download all" button from day 1.

As an added bonus, since this project already exists, there's an accompanying system to automatically convert the data dump to other formats. In my experience, the vast majority of applications building on the data dump do not work directly with the XML. Other, more convenient data formats are often created as an intermediate. Aside using it as an intermediate for various forms of analysis, there are a couple major examples of other distribution forms that are listed later in this README.

While these are preprocessed distributions of the data dump, this project is also meant to help converting to these various formats. While unlikely to replace the source code for either of these two examples, I hope the transformer system here can get rid of boilerplate for other projects.

## Known archives of new data dumps

### Complete data dump archives

A [different project](https://communitydatadump.com/index.html) is currently maintaining a list of both the source data dumps (XML), as well as other distributions. It includes both historical versions of the data dump, as well as new versions uploaded under the new anti-community scheme.

Note that since someone is uploading an unofficial version to archive.org, you may not need to use the downloader at all. However, to make sure this access continues, I strongly encourage you to download directly from SE anyway if you can -- this helps decrease the chance the uploader is identified and blocked by SE, which will turn into a problem for archival efforts in the long term. It may also decrease the chances SE points to low usage numbers as an excuse to axe the data dump entirely.[^4]

[^4]: There's no guarantee the data dump will continue existing anymore - removing as many justifications to axe the data dump as possible may become increasingly important at some point. Unfortunately, if it is, we won't find out until it's too late by seeing the data dump get axed.

### Other tools

This list contains converter tools that work on all sites and all tables.

| Maintainer | Format(s)               | First-party torrent available | Converter                                                            |
| ---------- | ----------------------- | ----------------------------- | -------------------------------------------------------------------- |
| Maxwell175 | SQLite, Postgres, MSSQL | Partially[^2]                 | [AGPL-3.0](https://github.com/Maxwell175/StackExchangeDumpConverter) |

### Other data dump distributions and conversion tools

For completeness (well, sort of, none of these lists are exhaustive), this is a list of incomplete archives (archives that limit the number of included tables and/or sites)

| Maintainer   | Format                                                                                                         | Torrent available | Converter                                              | Site(s)             | Tables     |
| ------------ | -------------------------------------------------------------------------------------------------------------- | ----------------- | ------------------------------------------------------ | ------------------- | ---------- |
| Brent Ozar   | [MSSQL](https://www.brentozar.com/archive/2015/10/how-to-download-the-stack-overflow-database-via-bittorrent/) | Yes               | [MIT-licensed](https://github.com/BrentOzarULTD/soddi) | Stack Overflow only | All tables |
| Jason Punyon | [SQLite](https://seqlite.puny.engineering/)                                                                    | No                | Closed-source[^1]                                      | All sites           | Posts only |

## Using the downloader

Note that it's stongly encouraged that you use a venv. To set one up, run `python3 -m venv env`. After that, you'll need to activate it with one of the activation scripts. Run the appropriate one for your operating system. If you're not sure what the scripts are called, you can find them in `./env/bin`

### Warnings

#### Cloudflare loops

> TL;DR: If you get a full-page Cloudflare block that loops back to itself after completing the capcha, connect to a VPN, or download the data dump from unofficial community reuploads

If you get a full-page Cloudflare block, and solving the captcha redirects you right back to the cloudflar eblock page even if you complete it correctly, you have to switch to a VPN in another country. For reasons beyond me, SE hard-blocks automated browsers _from specific countries only_. One of the verified blocked countries used for this test was Singapore. 

If you get slapped with a Cloudflare loop, the only option for now is to use a VPN in another country. Switzerland and Norway have both been verified to work at the time of writing. Fascinatingly, using a VPN makes no difference on the looping; it's purely country-based, not anti-VPN-based. The loop has been verified on both a residential IP and a datacenter IP (VPN).

I unfortunately do not (and cannot) write a complete list of countries affected by this bullshit, so you have to test this manually. If you do not have access to a VPN, check https://communitydatadump.com/ or https://academictorrents.com/collection/stack-exchange-data-dumps for archived (unofficial) versions uploaded by the community. They're usually uploaded within a few days to a couple weeks, and unless you're downloading directly from the archive.org version, is significantly faster and more stable than downloading from SE themselves.

>[!NOTE]
> The technical reason for this happening is that Cloudflare detects Selenium as a bot. This is by design in Geckodriver and Chrome, due to a standard that says automated browsers must identify themselves as automated. Thanks to increasingly aggressive anti-bot changes (particularly from SE), this means that this perfectly valid use of Selenium gets slapped to death by Cloudflare.
> 
> Attempts have been made to bypass these checks (see the bot-stealth-mode branch on GitHub), but these have not worked against Cloudflare. Likely, the only fix is to patch Geckodriver to remove `navigator.webdriver` and other identifiers at the lowest level possible. Normal, non-automated Firefox is not even presented with the Cloudflare screen of death, so it's only if the automation environment is detected. Fixing this, however, is a massive problem that goes far outside the bounds of this project, so unless workarounds are required for the downloader to work, it'll be left as a wontfix.
>
> If you've worked with these problems before, and have an idea how to fix them, pull requests are very much welcome. Note that switching to chromium for undetected-chromium is not an option due to heavy use of ublock origin to block several elements that cover significant parts of the screen.

#### Download instability, particularly of `stackoverflow.com.7z`

> TL;DR: If `stackoverflow.com.7z` fails to download:
> 1. You're on a good connection: consider connecting to a VPN before retrying.
> 2. You're on a bad connection: consider downloading via the unofficial community torrent instead. It's significantly more resistant to problems caused by bad internet connections.

If you keep running into the download of `stackoverflow.com.7z` failing, you may need to connect to a VPN. For reasons that are not clear (but that are likely down to SE being horrible at implementing things that work), `stackoverflow.com.7z` can just randomly fail. This assumes your device has a stable internet connection for the duration of the download, but arbitrarily fails anyway.

For some reason, connecting with a VPN offers just enough extra stability to avoid whatever causes the failures. This is especially the case if you're on a relatively slow (100Mbps) network, as longer download times increases the chance of failure. If you're on a generally unstable network, a VPN may help, but if it doesn't, it will be significantly easier to download the unofficial community reuploads, as these are always available as torrents. Torrents are generally more resistant to full failure on bad network connections, and can download single missing pieces rather than forcing you to redownload the entire 68+GB thing. 

### Requirements

- Python 3.10 or newer[^3]
- `pip3 install -r requirements.txt`
- Lots of storage. The 2024Q1 data dump was 92GB compressed.
- A display you can access somehow (physical or virtual, but you need to be able to see it) to be able to solve captchas
- Email and password login for Stack Exchange - Google, Facebook, GitHub, and other login methods are not supported, and will not be supported.
  - If you don't have this, see [this meta question](https://meta.stackexchange.com/a/1847/332043) for instructions.
- Firefox installed
  - Snap and flatpak users may run into problems; it's strongly recommended to have a non-snap/flatpak installation of Firefox and Geckodriver.
    - Known errors:
      - "The geckodriver version may not be compatible with the detected firefox version" - update Firefox and Geckodriver. If this still doesn't work, consider switching to a non-snap installation of Firefox and Geckodriver.
      - "Your Firefox profile cannot be loaded" - One of Geckodriver or Firefox is Snap-based, while the other is not. [Consider switching to a non-snap installation](https://stackoverflow.com/a/72531719/6296561) of Firefox, or verifying that your PATH is set correctly.
  - If you need to manaully install Geckodriver (which shouldn't normally be necessary; it's often bundled with Firefox in one way or another), the binaries are on [GitHub](https://github.com/mozilla/geckodriver/releases)

The downloader does **not** support Docker due to the display requirement.

### Config, running, and what to expect

#### Configuring and starting

1. Make sure you have all the requirements from the Requirements section.
2. Copy `config.example.json` to `config.json`
3. Open `config.json`, and edit in the values.
4. Run the extractor with `python3 -m sedd`. If you're on Windows, you may need to run `python -m sedd` instead.

##### Config option values

###### `notifications.provider`
Supported values:
* `"native"`: Uses your operating system's native notification method
* `null`: Disables notifications 

#### CLI options

Exractor CLI supports the following configuration options:

| Short | Long                   | Type     | Default           | Description                                                                                                                                                                 |
| ----- | ---------------------- | -------- | ----------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `-o`  | `--outputDir <path>`   | Optional | `<cwd>/downloads` | Specifies the directory to download the archives to.                                                                                                                        |
| `-k`  | `--keep-consent`       | Optional | `false`           | Whether to keep OneTrust's consent dialog. If set, you are responsible for getting rid of it yourself (uBlock can handle that for you too).                                 |
| `-s`  | `--skip-loaded <path>` | Optional | -                 | Whether to skip over archives that have already been downloaded. An archive is considered to be downloaded if the output directory has one already & the file is not empty. |
| -     | `--dry-run`            | Optional | -                 | Whether to actually download the archives. If set, only traverses the network's sites.                                                                                      |

#### Captchas and other misc. barriers

This software is designed around Selenium, a browser automation tool. This does, however, mean that the program can be stopped by various bot defenses. This would happen even if you downloaded all the [~183 data dumps](https://stackexchange.com/sites#questionsperday) fully by hand, because it's a _lot_ of repeated operations.

This is where notification systems come in; expecting you to sit and watch for potentially a significant number of hours is not a good use of time. If anything happens, you'll be notified, so you don't have to continuously watch the program. Currently, only a native desktop notifier is supported, but support for other notifiers may be added in the future.

#### Storage space

As of Q1 2024, the data dump was a casual 93GB in compressed size. If you have your own system to transform the data dump after downloading, you only need to worry about the raw size of the data dump.

However, if you use the built-in transformer pipeline, you'll need to expect a _lot_ more data use.

The output, by default, is compressed back into 7z if dealing with a file-based transformer. Due to this, an intermediate file write is performed prior to compressing back into a .7z. At runtime, you need:

- The compressed data dump; at least 92GB and increasing with each dump
- The compressed converted data dump; depending on compression rates for the specific format, this anywhere from a little less than the original size to significantly larger
- A significant amount of space for intermediate files. While these will be deleted as soon as they're done and compressed, they'll take up a significant amount of space on the disk in the meanwhile

Note that the transformer pipeline is executed separately; see the transformer section below.

#### Execution time

One of the major downsides with the way this project functions is that it's subject to Cloudflare bullshit. This means that the total time to download is `(combined size of data dumps) / (internet speed) + (rate limiting) + (navigation overhead) + (time to solve captchas)`. While navigation overhead and rate limiting (hopefully) doesn't account for a significant share of time, it can potentially be significant. It's certainly a slower option than archive.org's torrent.

## Using the transformer

Once you've downloaded the data dumps, you may want to transform it into a more usable format than the data dump offers by default. This is where the transformer component comes in.

### Docker

This section assumes you have Docker installed, with [docker-compose-v2](https://docs.docker.com/compose/migrate/).

A [default compose file](./docker-compose.yml) is provided for convenience. If you want to use a different one, set the `COMPOSE_FILE` environment variable in a custom `.env` file in the project's root directory, or provide it on the command line. To start the container (and build it if not done already), run:

```bash
docker compose up
```

By default, this binds `downloads` and `out` in the current working directory to the container. If you want to change these paths, update the `volumes` attribute mapping of the `transformer` service in your compose file (or the default docker-compose.yml file if not using a custom one).

#### Environment variables

The following environment variables are used:

| Name               | Values         | Default | Description               |
| ------------------ | -------------- | ------- | ------------------------- |
| `SEDD_OUTPUT_TYPE` | `json\|sqlite` | `json`  | Any upported output type. |
| `SPDLOG_LEVEL`     | log level      | `info`  | Sets the logging level.   |

If you want to override the defaults, either set them in a custom `.env` file in the project's root directory, or, if you have a UNIX shell (i.e. not cmd or PowerShell; Windows users can use Git Bash), you can explicitly provide them on the command line:

```bash
SEDD_OUTPUT_TYPE=sqlite docker compose up
```

If you want to rebuild the container, pass the `--build` flag to the docker command.

If you insist on using cmd or PowerShell instead of a good shell, setting the variables is left as an exercise to the reader.

### Native

#### Requirements

- C++20 compiler
- CMake 3.10 or newer

Other dependencies (stc, libarchive, spdlog, and pugixml) are automatically handled by CMake using FetchContent. Unlike the downloader, this component can run without a display.

#### Running

TL;DR:

```bash
cd transformer
mkdir build
cd build
# Option 1: debug:
cmake .. -DCMAKE_BUILD_TYPE=Debug
# Option 2: release mode; strongly recommended for anything that needs the performance:
cmake .. -DCMAKE_BUILD_TYPE=Release
# ---
# Replace 8 with the number of cores/threads you have
cmake --build . -j 8

# Note: this only works after running the Python downloader
# For early testing, I've been populating this folder with
# files from the old archive.org data dump.
# The last argument is the path to the downloaded data
# *UNIX:
./sedd-transformer -i ../../downloads -t [formatter type]
# Windows
.\sedd-transformer.exe -i ..\..\downloads -t [formatter type]
```

Pass `--help` to see the available formatters for your current version of the data dump transformer.

### Supported transformers

Currently, the following transformers are supported:

- `json`
- `sqlite`
  - Note: All data related to a site is merged into a single database

## Language rationale

While I really didn't want to split the system over two programming languages, this is unfortunately the best way to go about it.

C++ does not really support Selenium, which is effectively a requirement for the download process. There are bindings, but all of them appear to be out-of-date, and I don't feel like writing an entire system for selenium

Python, on the other hand, infuriatingly doesn't support 7z streaming, at least not in a convenient format. There's the `libarchive` package, but it refuses to build. `python-libarchive` allegedly does, but [Windows support is flaky](https://github.com/smartfile/python-libarchive/issues/38), so the transformer might've had to be separated from the downloader anyway. There's py7zr, which does work everywhere, but it [doesn't support 7z streaming](https://github.com/miurahr/py7zr/issues/579).

7z and XML streaming are both _critical_ for the processing pipeline. If you plan to convert the entire data dump, you'll eventually run into `stackoverflow.com-PostHistory.7z`, which is 39GB compressed, and **181GB uncompressed** in the 2024 Q1 data dump. As time passes, this will likely continue to grow, and the absurd amounts of RAM required to just tank the full size [is barely supported on modern and _very_ high-end hardware](https://www.reddit.com/r/buildapc/comments/17hqk3k/what_happened_to_256gb_ram_capacity_motherboards/). Finding someone able to tank that is going to be difficult for the vast majority of people.

Consequently, direct `libarchive` support is beneficial, and rather than writing an entire new python wrapper (or taking over an existing one), it's easier to just write that part in C++. Also, since it might be easier to run this particular part in a Docker container to avoid downloading build tools on certain systems, having it be fully headless is an advantage.

On the bright side, this should mean faster processing compared to Python.

## License

The code is under the MIT license; see the `LICENSE` file.

The data downloaded and produced is under various versions of [CC-By-SA](https://stackoverflow.com/help/licensing), as per Stack Exchange's licensing rules, in addition to whatever extra rules they try to impose on the data dump.

[^1]: I've been unable to find the generator code, but I've also been unable to find a statement confirming that it's closed-source. It's possible it is open-source, but if it is, it's hard to find the source
[^2]: Only Postgres at the time of writing, with more planned
[^3]: Might work with earlier versions, but these are untested and not supported
