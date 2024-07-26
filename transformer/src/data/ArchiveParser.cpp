#include "ArchiveParser.hpp"
#include "data/GlobalContext.hpp"
#include "spdlog/spdlog.h"
#include <pugixml.hpp>

#include "Transformer.hpp"

#include <archive.h>
#include <archive_entry.h>
#include <iostream>
#include <stdexcept>
#include <stc/StringUtil.hpp>
#include <cassert>
#include <unordered_map>

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
    : archivePath(path),
    a(archive_read_new())
{
    archive_read_support_format_7zip(a);
    archive_read_support_filter_all(a);
    
    int r = archive_read_open_filename(a, path.string().c_str(), BLOCK_SIZE);
    if (r != ARCHIVE_OK) {
        std::cerr << "Failed to read archive with error code = " << r << ": " << archive_error_string(a) << std::endl;
        throw std::runtime_error("Failed to read archive");
    }

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
        .conf = conf
    };

    if (conf.transformer) {
        conf.transformer->beginArchive(ctx);
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        std::string entryName = archive_entry_pathname(entry);
        spdlog::info("Extracting {}/{}", ctx.baseDomain, entryName);

        ctx.currType = DataDumpFileType::_UNKNOWN;
        ctx.currTypeStr = "";


        size_t readSize;
        la_int64_t offset;

        const void* buff;

        std::string incompleteBlock = "";
        // Incrementally read the data
        while (true) {
            int r = archive_read_data_block(a, &buff, &readSize, &offset);
            if (r == ARCHIVE_EOF) {
                break;
            } else if (r != ARCHIVE_OK) {
                std::cerr << "Error reading data: " << archive_error_string(a) << std::endl;
                throw std::runtime_error("Failed to read data");
            }
            std::string block(static_cast<const char*>(buff), readSize);
            std::string blockWithPrevious = incompleteBlock + block;
            incompleteBlock = "";

            bool completeBlock = blockWithPrevious.back() == '\n';

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
                if (line.size() == 0) {
                    continue;
                }
                auto openIdx = line.find('<');
                if (openIdx == std::string::npos || line.starts_with("</")) {
                    continue;
                }
                if (line.substr(openIdx).starts_with("<row")) {
                    if (ctx.currType == DataDumpFileType::_UNKNOWN) {
                        std::cerr << "Failed to parse opening tag" << std::endl;
                        throw std::runtime_error("Failed to find opening tag before row content");
                    }

                    pugi::xml_document doc;
                    pugi::xml_parse_result res = doc.load_string(line.c_str());
                    if (!res) {
                        std::cerr << "Failed to parse line as XML: " << line << "\nReason: " << res.description() << std::endl;
                        throw std::runtime_error("Failed to parse line as XML");
                    }
                    const auto& node = doc.first_child();

                    if (conf.transformer) {
                        conf.transformer->parseLine(node, ctx);
                    }
                } else if (ctx.currType == DataDumpFileType::_UNKNOWN) {
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

        if (incompleteBlock != "") {
            std::cerr << "Failed to fully parse file; found trailing block past EOF: " << incompleteBlock << std::endl;
            throw std::runtime_error("Failed to fully parse file");
        }
        if (conf.transformer) {
            conf.transformer->endFile();
        }

    }
    auto err = archive_error_string(a);
    if (err != nullptr) {
        spdlog::critical("{}", err);
        throw std::runtime_error(err);
    }

    if (conf.transformer) {
        conf.transformer->endArchive(ctx);
    }
}

}
