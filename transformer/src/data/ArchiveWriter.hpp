#pragma once

#include "archive.h"
#include "archive_entry.h"
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>

namespace sedd {

class ArchiveWriter {
private:
    std::filesystem::path archiveName;
    std::filesystem::path tmpOutputDir;
    std::vector<std::string> files;
    archive* a;
    std::ofstream writer;

public:
    ArchiveWriter(const std::filesystem::path& basePath);
    ~ArchiveWriter();

    void open(const std::string& filename);
    void write(const std::string& entry);
    void close();
    void commit();

};

}
