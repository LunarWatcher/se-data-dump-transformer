#pragma once

#include "data/GlobalContext.hpp"
#include <archive.h>

#include <filesystem>
#include <string>
#include <vector>

namespace sedd {

namespace DataDumpFileType {
    enum DataDumpFileType {
        // If badges stops being first, update the CheckSchema test
        BADGES,
        COMMENTS,
        POST_HISTORY,
        POST_LINKS,
        POSTS,
        TAGS,
        USERS,
        VOTES,
        // Must always be last; place any other values ahead of this
        _UNKNOWN
    };

    extern DataDumpFileType strToFiletype(const std::string&);
    extern std::string filetypeToStr(DataDumpFileType);
    extern std::string toFilename(DataDumpFileType);
}

struct ParserContext {
    /**
     * Site URL, for example stackoverflow.com. Extracted from the source archive
     */
    std::string site;
    std::filesystem::path archivePath;

    /**
     * The type of the file being parsed and passed onto the transformer
     */
    DataDumpFileType::DataDumpFileType currType = DataDumpFileType::_UNKNOWN;

    /**
     * Same as currType, but as a string
     */
    std::string currTypeStr;
    
    const GlobalContext& conf;
};

class Transformer;
class ArchiveParser {
private:
    constexpr static auto BLOCK_SIZE = 10240;

    archive* a;

    std::filesystem::path archivePath;

    std::vector<std::string> files;

public:
    ArchiveParser(const std::filesystem::path& archivePath);
    ~ArchiveParser();

    void read(const GlobalContext& conf);
};

}
