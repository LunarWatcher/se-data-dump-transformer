#pragma once

#include "Transformer.hpp"
#include <archive.h>

#include <filesystem>
#include <string>
#include <vector>

namespace sedd {

class ArchiveParser {
private:
    constexpr static auto BLOCK_SIZE = 10240;
    inline static std::vector<std::string> KNOWN_TAGS = {
        "badges",
        "comments",
        "posthistory",
        "postlinks",
        "posts",
        "tags",
        "users",
        "votes"
    };

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
