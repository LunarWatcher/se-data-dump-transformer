#pragma once

#include "archive.h"
#include "archive_entry.h"
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include "meta/ArchiveMacros.hpp"

namespace sedd {

class ArchiveWriter {
private:
    // Approx. 16MB
    constexpr static auto BLOCK_SIZE = size_t(4194304) * size_t(4);
    std::filesystem::path archiveName;
    std::filesystem::path tmpOutputDir;

    std::vector<std::string> files;
    archive* a;
    std::ofstream writer;

    bool createTempDir;

public:
    ArchiveWriter(
        const std::filesystem::path& basePath,
        bool createTempDir = true
    );
    ArchiveWriter(
        const std::filesystem::path& basePath,
        const std::filesystem::path& tmpOutputDir,
        bool createTempDir
    );
    ~ArchiveWriter();

    /**
     * Opens a managed text file for writing. Use write() to write to it, and call
     * close() when writing is completed.
     */
    void open(const std::string& filename);

    /**
     * Writes to the current open()ed file
     */
    void write(const std::string& entry);

    /**
     * Closes the current open()ed file.
     */
    void close();

    /**
     * Adds an externally managed file to the ArchiveWriter.
     * DO NOT use open, write, or close to modify these files.
     * It's entirely the responsibility of the invoking transformer to create the file.
     * The file will be opened and written to the archive when commit() is called.
     */
    void addBinaryFile(const std::string& filename);

    /**
     * Commits the saved files (either added with `addBinaryFile`, or managed text
     * files created with open() and write()) to the archive.
     */
    void commit();

};

}
