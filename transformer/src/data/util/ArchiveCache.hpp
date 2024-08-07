#pragma once

#include "data/ArchiveParser.hpp"
#include "data/ArchiveWriter.hpp"
#include <set>
#include <unordered_map>
#include <string>

namespace sedd {

struct SiteCache {
    std::shared_ptr<ArchiveWriter> writer;
    std::set<DataDumpFileType::DataDumpFileType> exhaustedFiletypes;
};

/**
 * Utility class for archive creation following a different format than the
 * source archive structure.
 * 
 * This is almost exclusively aimed at transformers converting all files into a
 * single file, which is likely primarily databasses. This is an issue with
 * Stack Overflow -- being split over several source archives requires a
 * different cache strategy to join them into a single file, especially if the
 * overall structure of the data dump changes in the new anti-community data
 * dump format.
 *
 * 
 */
class ArchiveCache {
private:
    std::unordered_map<std::string, SiteCache> siteColumnMaps;
public:

    /**
     * Initialises a cache entry.
     * The caller does not need to make sure this is only invoked once; if the
     * internal cache has the site populated, this function does nothing.
     */
    void initArchive(const ParserContext& ctx, const std::string& binFile);

    /**
     * Called when a new entry is added to the binary.
     */
    void registerType(const std::string& normalisedSiteID, DataDumpFileType::DataDumpFileType ft);

    /**
     * Checks if the archive is completed, and compresses the file and removes the source file if it is.
     * Does nothing otherwise.
     */
    void checkComplete(const std::string& site);

};

}
