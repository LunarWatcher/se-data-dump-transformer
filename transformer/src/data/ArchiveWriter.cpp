#include "ArchiveWriter.hpp"
#include "archive.h"
#include "archive_entry.h"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <ios>
#include <stdexcept>
#include <iostream>

namespace sedd {

ArchiveWriter::ArchiveWriter(const std::filesystem::path& basePath) : archiveName(basePath.string() + ".7z"), tmpOutputDir(basePath) {
    spdlog::debug("Opening archive {}", archiveName.string());
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

    std::filesystem::create_directories(this->tmpOutputDir);
}

ArchiveWriter::~ArchiveWriter() {
    spdlog::debug("Cleaning up ArchiveWriter");
    close();
    int r = archive_write_close(a);
    if (r != ARCHIVE_OK) {
        std::cerr << archive_error_string(a) << std::endl;
    }
    archive_write_free(a);
    std::filesystem::remove_all(this->tmpOutputDir);
}

void ArchiveWriter::commit() {

    for (auto& file : this->files) {
        spdlog::debug("Now committing {} to archive", file);
        archive_entry* currEntry = archive_entry_new();
        archive_entry_set_pathname(currEntry, file.c_str());
        archive_entry_set_filetype(currEntry, AE_IFREG);
        archive_entry_set_perm(currEntry, 0644);

        std::ifstream f(this->tmpOutputDir / file);

        std::streampos bytesStart = f.tellg();
        f.seekg(0, std::ios::end);
        std::streampos bytesEnd = f.tellg();

        f.clear();
        f.seekg(0);


        archive_entry_set_size(currEntry, bytesEnd - bytesStart);

        int r = archive_write_header(a, currEntry);
        if (r != ARCHIVE_OK) {
            std::cerr << archive_error_string(a) << std::endl;
        }

        while (f) {
            std::string buff, tmp;
            while (f && std::getline(f, tmp)) {
                buff += tmp + "\n";
                if (buff.size() > 65535) {
                    break;
                }
            }

            if (buff.size() == 0) break;
            r = archive_write_data(a, buff.data(), buff.size());
        }


        archive_entry_free(currEntry);
    }

}

void ArchiveWriter::open(const std::string& filename) {
    spdlog::debug("Opening file {}", filename);
    this->files.push_back(filename);
    writer.open(tmpOutputDir / filename);
    if (!writer) {
        spdlog::error("Failed to open {}", filename);
        throw std::runtime_error("Failed to open");
    }
}

void ArchiveWriter::write(const std::string& entry) {
    writer << entry;
}

void ArchiveWriter::close() {
    writer.close();
}

}
