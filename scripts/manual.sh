#!/usr/bin/bash

# This function only works as-is as long as the URLs don't expire. Alternate code is provided if the URL expiration is
# reduced back to 30 seconds again. Wget is able to resume even when the URL changes, which at least FF doesn't appear
# to be.
#
# You should only use this if you plan to use the full data dump download (introduced in October 2026), or for
# downloading specifically Stack Overflow. This is purely meant as a fallback if SE breaks the bigger downloads. I have
# personally had tebibytes of failed downloads using the method SE intends normal users to use, which only wastes my
# time. This is much better at retries, but at the cost of needing to manually get the URL.

function download-se-dump() {
    url="$1"
    out="$2"
    if [[ "$url" == "" || "$out" == "" ]]; then
        echo "usage: download-se-dump url outfile.7z"
        exit 1
    fi

    while ! wget -c -O "$out" "$url"; do
        notify-send "Failed to download $out; resuming"
    done
    # Alternate form: use this if the URL expiration drops back down to 30 seconds
    # if ! wget -c -O "$out" "$url"; do
    #     notify-send "Failed to download $out. Please rerun with a new URL"
    #     exit 1
    # done

    notify-send "Finished downloading $out"
}

# Exists for direct calls to this script, which isn't intended behaviour.
download-se-dump "$1" "$2"
