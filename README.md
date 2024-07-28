# Stack Exchange data dump downloader and transformer

[![Data dump transformer build](https://github.com/LunarWatcher/se-data-dump-transformer/actions/workflows/transformer.yml/badge.svg)](https://github.com/LunarWatcher/se-data-dump-transformer/actions/workflows/transformer.yml) [![Stackapps listing](https://img.shields.io/badge/StackApps%20listing-FF9900)](https://stackapps.com/q/10591/69829)

**NOTE:** This repo does not yet gather the data dump, as it has not yet been released in the new format. It currently contains the scaffolding required to deal with some of SE's bullshit, to make sure it can be quickly adapted to actually download the data dump parts when they become available.

**Disclaimer:** This project is not affiliated with Stack Exchange, Inc.

## Background

This section contains background on why this project exists. If you know and/or don't care, feel free to skip to the next section.

In June 2023, Stack Exchange [briefly cancelled the data dump](https://meta.stackexchange.com/q/389922/332043), and backpedalled after a significant amount of backlash from the community. The status-quo of uploads to archive.org was restored. In the slightly more than a year between June 2023 and July 2024, it looked like they were staying off that path. Notably, they made a [logical shift to the _exact_ dates involved in the upload](https://meta.stackexchange.com/q/398279/) to deal with archive.org being slow. In December 2023, they [announced a delay in the upload](https://meta.stackexchange.com/q/395197/) likely to avoid speculation that another cancellation was happening. 

We appeared to be out of the woods. But this repo wouldn't exist if that was the case now, would it?

### 2024 data dump restriction attempt

In July 2024, Stack Exchange announced the first restrictions on the data dump, by moving it in-house and actively discouraging archive.org reuploads, [likely in violation of the CC-By-SA license](https://meta.stackexchange.com/a/401326/332043)

The current revision can be read [here](https://meta.stackexchange.com/q/401324/332043)

Here's what's happening:

* SE is moving the data dump from archive.org to their own infrastructure
* They're discontinuing the archive.org dump, which makes it significantly harder to archive the data if SE, for example, were to go out of business 
    * In addition to discontinuing the archive.org dump, they're imposing significant restrictions on the data dump
* They're doing the first revision **without the possibility to download the entire data dump** in one click, a drastic QOL reduction from the current situation.

This is an opinionated summary of the reason why; SE wants to capitalise on AI companies that need training data, and have decided that the community doesn't matter in that process. The current process, while not nearly as restricting as rev. 1 and 2, is a symptom of precisely one thing; Stack Exchange doesn't care about its users, but rather cares about finding new ways to profit off user data.

**Stack Exchange, Inc. is now the single biggest threat to the community**, and to the platform's user-generated and [permissively-licensed content](https://stackoverflow.com/help/licensing) that the community has spent countless hours creating precisely _because_ the data is public.

That is why this project exists; this is meant to automate the data dump download process for non-commercial license-compliant use, since Stack Exchange, Inc. couldn't be bothered adding a "download all" button from day 1. 

As an added bonus, since this project already exists, there's an accompanying system to automatically convert the data dump to other formats. In my experience, the vast majority of applications building on the data dump do not work directly with the XML. Other, more convenient data formats are often created as an intermediate.  Aside using it as an intermediate for various forms of analysis, there are a couple major examples of other distribution forms that are listed later in this README.

While these are preprocessed distributions of the data dump, this project is also meant to help converting to these various formats. While unlikely to replace the source code for either of these two examples, I hope the transformer system here can get rid of boilerplate for other projects. 

## Known archives of new data dumps

This is meant to be a list of all known archives and transformed versions of the new data dump. If one exists for the latest version already, that means you don't have to use any of these tools in the first place (though using the downloader in particular is encouraged, to make it harder for SE to abuse their PII access to blame individuals).

### Complete data dump archives

A [different project](https://communitydatadump.com/index.html) is currently maintaining a list of both the source data dumps (XML), as well as other distributions. At the time of writing, it only consists of the historical XML dumps, making it identical to [a post on meta.SE](https://meta.stackexchange.com/a/224922/332043). Other distributions will appear as more tools like the transformer in this repo get more and better.

### Other data dump distributions and conversion conversion tools

For completeness (well, sort of, none of these lists are exhaustive), this is a list of incomplete archives (archives that limit the number of included tables and/or sites)

* Brent Ozar's [MSSQL version](https://www.brentozar.com/archive/2015/10/how-to-download-the-stack-overflow-database-via-bittorrent/) (torrent; [open-source converter](https://github.com/BrentOzarULTD/soddi); Stack Overflow only; all tables)
* Jason Punyon's [SQLite version](https://seqlite.puny.engineering/) (no torrent; closed-source converter[^1]; posts only)

[^1]: I've been unable to find the generator code, but I've also been unable to find a statement confirming that it's closed-source. It's possible it is open-source, but if it is, it's hard to find the source

## Using the downloader


Note that it's stongly encouraged that you use a venv. To set one up, run `python3 -m venv env`. After that, you'll need to activate it with one of the activation scripts. Run the appropriate one for your operating system. If you're not sure what the scripts are called, you can find them in `./env/bin`

### Requirements

* `pip3 install -r requirements.txt`
* Lots of storage. The 2024Q1 data dump was 92GB
* A display you can access somehow (physical or virtual, but you need to be able to see it) to be able to solve captchas


### Config, running, and what to expect

#### Configuring and starting

1. Make sure you have all the requirements from the Requirements section.
2. Copy `config.example.json` to `config.json`
3. Open `config.json`, and edit in the values. The values are described within the JSON file itself.
4. Run the extractor with `python3 -m sedd`. If you're on Windows, you may need to run `python -m sedd` instead. 

##### Download modes (not yet implemented)

There are two download modes:
* `key TBA`: Starts downloading data dumps as soon as the URLs become available, but at the expense of download performance of individual files. This means up to ~365 concurrent downloads, though that number will go down rather quickly due to the many small data dumps.

    This is both the default **and the (unofficially) recommended way** to download the data dumps.

    If SE wanted to avoid this, they could've [bothered implementing combined main + meta downloads](https://meta.stackexchange.com/questions/401324/announcing-a-change-to-the-data-dump-process?cb=1#comment1340364_401324), or even better, a "download all" button, before pushing this utter crap.
* `key TBA`: Downloads one data dump at a time, maximising the download speed for each individual data dump. Recommended if you're on an unstable or slow internet connection, or want to start converting the dump progressively as new entries appear.

#### Captchas and other misc. barriers

This software is designed around Selenium, a browser automation tool. This does, however, mean that the program can be stopped by various bot defenses. This would happen even if you downloaded all the [~183 data dumps](https://stackexchange.com/sites#questionsperday) fully by hand, because it's a _lot_ of repeated operations. 

This is where notification systems come in; expecting you to sit and watch for potentially a significant number of hours is not a good use of time. If anything happens, you'll be notified, so you don't have to continuously watch the program. Currently, only a native desktop notifier is supported, but support for other notifiers may be added in the future.

#### Storage space

As of Q1 2024, the data dump was a casual 93GB in compressed size. If you have your own system to transform the data dump after downloading, you only need to worry about the raw size of the data dump.

However, if you use the built-in transformer pipeline, you'll need to expect a _lot_ more data use. 

The output, by default, is compressed back into 7z if dealing with a file-based transformer. Due to this, an intermediate file write is performed prior to compressing back into a .7z. At runtime, you need:

* The compressed data dump; at least 92GB and increasing with each dump
* The compressed converted data dump; depending on compression rates for the specific format, this anywhere from a little less than the original size to significantly larger
* A significant amount of space for intermediate files. While these will be deleted as soon as they're done and compressed, they'll take up a significant amount of space on the disk in the meanwhile

Note that the transformer pipeline is executed separately; see the transformer section below.

#### Execution time 

One of the major downsides with the way this project functions is that it's subject to Cloudflare bullshit. This means that the total time to download is `(combined size of data dumps) / (internet speed) + (rate limiting) + (navigation overhead) + (time to solve captchas)`. While navigation overhead and rate limiting (hopefully) doesn't account for a significant share of time, it can potentially be significant. It's certainly a slower option than archive.org's torrent.

## Using the transformer

Once you've downloaded the data dumps, you may want to transform it into a more usable format than the data dump offers by default. This is where the transformer component comes in.

### Docker

Coming Soon:tm:

### Native

#### Requirements
* C++20 compiler
* CMake 3.10 or newer

Binary dependencies (stc, libarchive, spdlog, and pugixml) are automatically handled by CMake using FetchContent. Unlike the downloader, this component can run without a display.

#### Running
TL;DR:
```
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
* `json`
* `sqlite`
    * Note: All data related to a site is merged into a single database

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


