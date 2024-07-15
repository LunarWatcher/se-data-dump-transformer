#pragma once

#include <archive.h>

#include <filesystem>
#include <string>
#include <vector>

namespace sedd {

namespace DataDumpFileType {
    enum DataDumpFileType {
        BADGES,
        COMMENTS,
        POST_HISTORY,
        POST_LINKS,
        POSTS,
        TAGS,
        USERS,
        VOTES,
        _UNKNOWN
    };

    extern DataDumpFileType strToFiletype(const std::string&);
    extern std::string filetypeToStr(DataDumpFileType);
}

struct ParserContext {
    /**
     * Site URL, for example stackoverflow.com. Extracted from the source archive
     */
    std::string site;

    /**
     * The type of the file being parsed and passed onto the transformer
     */
    DataDumpFileType::DataDumpFileType currType = DataDumpFileType::_UNKNOWN;

    /**
     * Same as currType, but as a string
     */
    std::string currTypeStr;
    
};

class Transformer;
class ArchiveParser {
private:
    constexpr static auto BLOCK_SIZE = 10240;

    archive* a;
    Transformer* transformer;

    std::filesystem::path archivePath,
        outputPath;

    std::vector<std::string> files;

public:
    ArchiveParser(const std::filesystem::path& archivePath);
    ~ArchiveParser();

    void read();
};

}
