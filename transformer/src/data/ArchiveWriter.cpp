#include "ArchiveWriter.hpp"
#include "archive.h"
#include "archive_entry.h"
#include "data/ArchiveWriter.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include <stc/StdFix.hpp>

namespace sedd {

ArchiveWriter::ArchiveWriter(const std::filesystem::path& basePath, bool createTempDir) 
    : ArchiveWriter(basePath, basePath.string() + "-tmp", createTempDir) {}

// TODO: These paths make no sense. Refactor
ArchiveWriter::ArchiveWriter(
    const std::filesystem::path& basePath,
    const std::filesystem::path& tmpOutputDir,
    bool createTempDir
) :
    archiveName(basePath.string() + ".7z"),
    tmpOutputDir(tmpOutputDir),
    createTempDir(createTempDir)
{
    spdlog::debug("Opening output archive: {}", archiveName.string());
    // TODO: figure out if archive_write_new() works
    a = archive_write_new();
    int r = archive_write_set_format_7zip(a);
    if (r != ARCHIVE_OK) {
        std::cerr << archive_error_string(a) << std::endl;
    }
    archive_write_set_format_option(a, "7zip", "compression", "lzma2");
    archive_write_set_format_option(a, "7zip", "compression-level", "9");
    
    r = archive_write_open_filename(a, archiveName.string().c_str());
    if (r != ARCHIVE_OK) {
        std::cerr << archive_error_string(a) << std::endl;
    }

    if (createTempDir) {
        std::filesystem::create_directories(this->tmpOutputDir);
    }
}

ArchiveWriter::~ArchiveWriter() {
    spdlog::debug("Cleaning up ArchiveWriter");
    close();
    int r = archive_write_close(a);
    if (r != ARCHIVE_OK) {
        std::cerr << archive_error_string(a) << std::endl;
    }
    archive_write_free(a);
    if (createTempDir) {
        std::filesystem::remove_all(this->tmpOutputDir);
    } else {
        // TODO: This feels sketch
        for (auto& file : files) { 
            spdlog::debug("Binary file::Removing {}", (this->tmpOutputDir / file).string());
            std::filesystem::remove(this->tmpOutputDir / file);
        }
    }
}

void ArchiveWriter::commit() {

    for (auto& file : this->files) {
        spdlog::debug("Now committing {} to archive", file);
        archive_entry* currEntry = archive_entry_new();
        archive_entry_set_pathname(currEntry, file.c_str());
        archive_entry_set_filetype(currEntry, AE_IFREG);
        archive_entry_set_perm(currEntry, 0644);

        std::ifstream f(this->tmpOutputDir / file, std::ios_base::binary);

        std::streampos bytesStart = f.tellg();
        f.seekg(0, std::ios::end);
        std::streampos bytesEnd = f.tellg();

        f.clear();
        f.seekg(0, std::ios::beg);


        archive_entry_set_size(currEntry, bytesEnd - bytesStart);

        int r = archive_write_header(a, currEntry);
        if (r != ARCHIVE_OK) {
            std::cerr << archive_error_string(a) << std::endl;
            throw std::runtime_error("Failed to write header");
        }

        // while (true) because https://stackoverflow.com/a/59296668/6296561
        // Using while (f) terminates prematurely
        while (true) {
            char inbuff[65535];
            std::string buff;

            // TODO: Figure if it's better for performance to write larger chunks at once, or if writing lines directly is equally good
            f.read(inbuff, 65535);

            if (f.gcount() == 0) break;

            r = archive_write_data(a, inbuff, f.gcount());
        }

        r = archive_write_finish_entry(a);
        if (r != ARCHIVE_OK) {
            std::cerr << archive_error_string(a) << std::endl;
            throw std::runtime_error("Failed to finish archive");
        }
        archive_entry_free(currEntry);
    }

}

void ArchiveWriter::open(const std::string& filename) {
    if (!createTempDir) {
        throw std::runtime_error("Can't open() without a tempDir");
    }
    spdlog::debug("Opening file {}", filename);
    this->files.push_back(filename);
    writer.open(tmpOutputDir / filename);
    if (!writer) {
        spdlog::error("Failed to open {}", filename);
        throw std::runtime_error("Failed to open");
    }
}

void ArchiveWriter::write(const std::string& entry) {
    if (!writer) {
        throw std::runtime_error("Writer not open");
    }
    writer << entry;
}

void ArchiveWriter::close() {
    if (!writer) {
        return;
    }
    writer.close();
}    

void ArchiveWriter::addBinaryFile(const std::string& filename) {
    this->files.push_back(filename);
}

}
