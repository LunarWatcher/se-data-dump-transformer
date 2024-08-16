#include "ArchiveParser.hpp"
#include "data/GlobalContext.hpp"
#include "meta/ArchiveMacros.hpp"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <pugixml.hpp>

#include "Transformer.hpp"
#include "util/StringSanitiser.hpp"
#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <stdexcept>
#include <stc/StringUtil.hpp>
#include <cassert>
#include <unordered_map>
#include <fstream>

namespace sedd {

DataDumpFileType::DataDumpFileType DataDumpFileType::strToFiletype(const std::string& tag) {
    static std::unordered_map<std::string, DataDumpFileType> map = {
        {"badges", DataDumpFileType::BADGES},
        {"comments", DataDumpFileType::COMMENTS},
        {"posthistory", DataDumpFileType::POST_HISTORY},
        {"postlinks", DataDumpFileType::POST_LINKS},
        {"posts", DataDumpFileType::POSTS},
        {"tags", DataDumpFileType::TAGS},
        {"users", DataDumpFileType::USERS},
        {"votes", DataDumpFileType::VOTES},
    };

    return map.at(tag);
}
std::string DataDumpFileType::filetypeToStr(DataDumpFileType type) {
    static std::unordered_map<DataDumpFileType, std::string> map = {
        {DataDumpFileType::BADGES, "badges"},
        {DataDumpFileType::COMMENTS, "comments"},
        {DataDumpFileType::POST_HISTORY, "posthistory"},
        {DataDumpFileType::POST_LINKS, "postlinks"},
        {DataDumpFileType::POSTS, "posts"},
        {DataDumpFileType::TAGS, "tags"},
        {DataDumpFileType::USERS, "users"},
        {DataDumpFileType::VOTES, "votes"},
    };

    return map.at(type);
}

std::string DataDumpFileType::toFilename(DataDumpFileType type) {
    // TODO: the redundancy with filetypeToStr is stupid. Figure out a better system
    static std::unordered_map<DataDumpFileType, std::string> map = {
        {DataDumpFileType::BADGES, "Badges"},
        {DataDumpFileType::COMMENTS, "Comments"},
        {DataDumpFileType::POST_HISTORY, "PostHistory"},
        {DataDumpFileType::POST_LINKS, "PostLinks"},
        {DataDumpFileType::POSTS, "Posts"},
        {DataDumpFileType::TAGS, "Tags"},
        {DataDumpFileType::USERS, "Users"},
        {DataDumpFileType::VOTES, "Votes"},
    };

    return map.at(type);
}

ArchiveParser::ArchiveParser(const std::filesystem::path& path) 
    : 
        a(archive_read_new()),
        archivePath(path)
{
    SEDDARCHIVE_CHECK_ERROR(a, archive_read_support_format_7zip(a));
    SEDDARCHIVE_CHECK_ERROR(a, archive_read_support_filter_all(a));
    
    SEDDARCHIVE_CHECK_ERROR(a, archive_read_open_filename(a, path.string().c_str(), BLOCK_SIZE));

}

ArchiveParser::~ArchiveParser() {
    archive_read_free(a);
}

void ArchiveParser::read(const GlobalContext& conf) {
    archive_entry *entry;

    auto baseDomain = this->archivePath.filename().replace_extension().string();
    auto baseName = stc::string::split(baseDomain, '-', 1).at(0);

    ParserContext ctx {
        .baseDomain = baseDomain,
        .baseSiteName = baseName,
        .archivePath = this->archivePath,
        .currType = DataDumpFileType::UNKNOWN,
        .currTypeStr = "",
        .conf = conf,
    };

    if (conf.transformer) {
        auto path = conf.transformer->getOutputArchivePath(ctx);
        if (std::filesystem::exists(path) && std::filesystem::file_size(path) != 0) {
            spdlog::debug("Output archive for {} ({}) already exists", ctx.baseDomain, path.string());
            if (conf.recover) {
                spdlog::info("Recovery mode: skipping {}", ctx.baseDomain);
                return;
            } 
            spdlog::warn("Recovery mode disabled, but {} already exists. Removing...", path.string());
            std::filesystem::remove(path);
        }
    }

    if (conf.transformer) {
        conf.transformer->beginArchive(ctx);
    }


    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string entryName = archive_entry_pathname(entry);
        spdlog::info("Extracting {}/{}", ctx.baseDomain, entryName);

        ctx.currType = DataDumpFileType::UNKNOWN;
        ctx.currTypeStr = "";


        size_t readSize;
        la_int64_t offset;

        const void* buff;

        std::string incompleteBlock;
        bool isInLicenseBlock = false;

        // Incrementally read the data
        while (true) {
            int r = archive_read_data_block(a, &buff, &readSize, &offset);
            if (r == ARCHIVE_EOF) {
                break;
            } else if (r != ARCHIVE_OK) {
                std::cerr << "Error reading data: " << archive_error_string(a)
                    << ", r = " << r
                    << ", archive = " << ctx.baseDomain
                    << std::endl;
                throw std::runtime_error("Failed to read data");
            }
            std::string block(static_cast<const char*>(buff), readSize);
            std::string blockWithPrevious = incompleteBlock + block;
            incompleteBlock = "";

            bool completeBlock = blockWithPrevious.back() == '\n'
#ifdef __APPLE__
                || blockWithPrevious.back() == '\r'
#endif
                ;

            // For some reason, the archive uses Windows CRLF
            std::vector<std::string> lines = stc::string::split(blockWithPrevious, "\r\n");
            // If the block is incomplete
            if (!completeBlock) {
                incompleteBlock = lines.back();
                if (incompleteBlock.starts_with("</")) {
                    // We found a closing tag; this is always the root tag being closed
                    incompleteBlock = "";
                } else {
                    lines.pop_back();
                }
            }

            for (const auto& line : lines) {

                if (line.empty()) {
                    continue;
                }
                // Workaround for https://meta.stackexchange.com/a/401889/332043
                // in the August 2024 data dump, and in new data dumps after those.
                if (!isInLicenseBlock && line.starts_with("<!--")) {
                    isInLicenseBlock = true;
                    continue;
                } else if (isInLicenseBlock) {
                    if (line.starts_with("-->")) {
                        isInLicenseBlock = false;
                    }
                    continue;
                }
                auto openIdx = line.find('<');
                if (openIdx == std::string::npos || line.starts_with("</")) {
                    continue;
                }
                if (line.substr(openIdx).starts_with("<row")) {
                    if (ctx.currType == DataDumpFileType::UNKNOWN) {
                        std::cerr << "Failed to parse opening tag" << std::endl;
                        throw std::runtime_error("Failed to find opening tag before row content");
                    }

                    pugi::xml_document doc;
                    pugi::xml_parse_result res = doc.load_string(
                        StringSanitiser::clearNullBytes(line).c_str(),
                        (pugi::parse_default | pugi::encoding_utf8 | pugi::parse_ws_pcdata_single) & ~pugi::parse_escapes
                    );
                    if (!res) {
                        std::cerr << "Failed to parse line as XML: " << line << "\nReason: " << res.description() << std::endl;
                        throw std::runtime_error("Failed to parse line as XML");
                    }
                    const auto& node = doc.first_child();

                    if (conf.transformer) {
                        try {
                            conf.transformer->parseLine(node, ctx);
                        } catch (...) {
                            std::cout << "Raw bytes: ";
                            for (int b : line) { // NOLINT
                                std::cout << std::hex << b << " ";
                            }
                            std::cout << std::endl;

                            throw;
                        }
                    }
                } else if (ctx.currType == DataDumpFileType::UNKNOWN) {
                    //for (const auto& tag : KNOWN_TAGS) {
                        //if (line == "<" + tag + ">") {
                            //openingTag = tag;
                            //spdlog::debug("Found opening tag {}", tag);
                            //break;
                        //}
                    //}
                    // Filter out XML start headers
                    if (line.ends_with("?>")) {
                        continue;
                    }
                    if (line.starts_with("<") && line.ends_with(">")) {
                        std::string word = line.substr(1, line.size() - 2);
                        auto filetype = DataDumpFileType::strToFiletype(word);

                        ctx.currType = filetype;
                        ctx.currTypeStr = std::move(word);

                        if (conf.transformer) {
                            conf.transformer->beginFile(ctx);
                        }
                    }
                } else {
                    spdlog::warn("Unknown line: {}", line);
                }
            }

        }

        if (!incompleteBlock.empty()) {
            std::cerr << "Failed to fully parse file; found trailing block past EOF: " << incompleteBlock << std::endl;
            throw std::runtime_error("Failed to fully parse file");
        }
        if (conf.transformer) {
            conf.transformer->endFile();
        }

    }
    const auto* err = archive_error_string(a);
    if (err != nullptr) {
        spdlog::critical("{}", err);
        throw std::runtime_error(err);
    }

    if (conf.transformer) {
        conf.transformer->endArchive(ctx);
    }
}

std::vector<std::filesystem::path> ArchiveParser::checkExtractSubarchives(const std::filesystem::path& outputDir) {
    std::vector<std::filesystem::path> out;

    archive_entry *entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string entryName = archive_entry_pathname(entry);
        if (entryName.ends_with(".7z")) {
            if (out.empty()) {
                spdlog::info("{} is evil (nested .7z files)", this->archivePath.string());
            }
            spdlog::info("Unpacking {}", entryName);

            out.emplace_back(outputDir / entryName);

            if (std::filesystem::exists(out.back())) {
                spdlog::debug("Skipping {}; already unpacked", out.back().string());
                continue;
            }

            std::ofstream o(
                out.back(),
                std::ios_base::binary
            );
            int r;

            do {
                size_t readSize;
                int64_t offset;
                const void* buff;

                r = archive_read_data_block(
                    a,
                    &buff,
                    &readSize,
                    &offset
                );

                if (readSize == 0) {
                    break;
                }

                std::string block(static_cast<const char*>(buff), readSize);
                o << block;
                
            } while (r == ARCHIVE_OK);

            if (r == ARCHIVE_FATAL) {
                std::cerr << "Failed to extract archive from archive: " << archive_error_string(a) << std::endl;
                throw std::runtime_error("Failed to read archive");
            }
        } else {
            // Clear archive
            break;
        }
    }


    return out;
}

}
