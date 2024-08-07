#pragma once

#include <string>

namespace sedd::License {

/**
 * This string is dumped into a dedicated file inside each output archive. 
 * While not an optimal way to do so, it at least ensures the distribution
 * units always include the license.
 *
 * Newer versions of the data dump include the license within each XML file:
 *     https://meta.stackexchange.com/a/401889/332043
 * but this is not viable for all formats.
 * JSON, for example, does not support comments, and the structure of the JSON
 * file disallows a dedicated license object. The database formats could
 * potentially get a dedicated table to licenses, but I do question the
 * usefulness of that over just having a text file with a description.
 * Because it'll be part of every archive, it will bloat the data dump
 * slightly, but arguably not more so than the XML comments in the source data.
 */
static inline std::string dataDumpLicense = R"(The contents of this data dump are licensed under various versions of CC-By-SA.
These are:
* CC-By-SA 2.5: https://creativecommons.org/licenses/by-sa/2.5/
* CC-By-SA 3.0: https://creativecommons.org/licenses/by-sa/3.0/
* CC-By-SA 4.0: https://creativecommons.org/licenses/by-sa/4.0/

While some of the content includes an explicit license for that post, comment, or revision as a dedicated field,
not all do. https://stackoverflow.com/help/licensing contains the rough timestamps if you need to determine the
versions manually, and the version of the data dump you're looking at doesn't include them explicitly.

Note that at least one data dump (April 2024) has been known to lack the ContentLicenseField. This is a problem
with the source data provided by Stack Overflow, Inc.

This converted form of the data dump is under the same license as the CC-By-SA content within it.
Additionally, the converted format respects the original data dump format as much as possible.
Aside varying data types and certain formats (databases) merging sites into one single output
archive, any errors in the data are also errors in the source material.

There are _many_ such errors in the source material, and it isn't possible to list or find them
all. 
)";

}
