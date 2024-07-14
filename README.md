# Stack Exchange data dump downloader and transformer

**NOTE:** This repo does not yet gather the data dump, as it has not yet been released in the new format. It currently contains the scaffolding required to deal with some of SE's bullshit, to make sure it can be quickly adapted to actually download the data dump parts when they become available.

**Disclaimer:** This project is not affiliated with Stack Exchange, Inc.

## Background

This section contains background on why this project exists. If you know and/or don't care, skip to the next section.

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

That is why this project exists; this is meant to automate the data dump download process. Stack Exchange will likely try to take another stab at further data dump restrictions at some point in the future, which is why it's more important now than ever to make sure there's decentralised archives of as much of the data as possible.


## Using the downloader

Note that it's stongly encouraged that you use a venv. To set one up, run `python3 -m venv env`. After that, you'll need to activate it with one of the activation scripts. Run the appropriate one for your operating system. If you're not sure what the scripts are called, you can find them in `./env/bin`

### Requirements

* `pip3 install -r requirements.txt`


### Config, running, and what to expect

#### Configuring and starting

1. Make sure you have all the requirements from the Requirements section.
2. Copy `config.example.json` to `config.json`
3. Open `config.json`, and edit in the values. The values are described within the JSON file itself.
4. Run the extractor with `python3 -m sedd`. If you're on Windows, you may need to run `python -m sedd` instead. 

#### Captchas and other misc. barriers

This software is designed around Selenium, a browser automation tool. This does, however, mean that the program can be stopped by various bot defenses. This would happen even if you downloaded all the [~182 data dumps](https://stackexchange.com/sites#questionsperday) fully by hand, because it's a _lot_ of repeated operations. 

This is where notification systems come in; expecting you to sit and watch for potentially a significant number of hours is not a good use of time. If anything happens, you'll be notified, so you don't have to continuously watch the program. Currently, only a native desktop notifier is supported, but support for other notifiers may be added in the future.

#### Storage space

As of Q1 2024, the data dump was a casual 93GB in compressed size. If you have your own system to transform the data dump after downloading, you only need to worry about the raw size of the data dump.

However, if you use the built-in transformer pipeline, you'll need to expect a _lot_ more data use. 

The output, by default, is compressed back into 7z if dealing with a file-based transformer. Due to this, an intermediate file write is performed prior to compressing back into a .7z

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

Binary dependencies (stc, libarchive, spdlog, and pugixml) are automatically handled by CMake using FetchContent.


#### Running
TL;DR:
```
cd transformer
mkdir build
cd build
# Option 1: debug:
cmake .. -DCMAKE_BUILD_Type=Debug 
# Option 2: release mode; strongly recommended for anything that needs the performance:
cmake .. -DCMAKE_BUILD_Type=Release
# ---
# Replace 8 with the number of cores/threads you have
cmake --build . -j 8

# Note: this only works after running the Python downloader
# For early testing, I've been populating this folder with 
# files from the old archive.org data dump.
# The last argument is the path to the downloaded data
# *UNIX:
./sedd-transformer ../../downloads
# Windows
.\sedd-transformer.exe ..\..\downloads
```

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


