#include "FabricatedDataV1.hpp"
#include "spdlog/spdlog.h"
#include <stdexcept>

namespace sedd {

bool FabricatedDataV1Filter::process(DataDumpFileType_t type, pugi::xml_node& row) {
    // The fabricated posts are always in Posts.xml
    if (type != DataDumpFileType_t::POSTS) {
        return false;
    }

    auto uid = row.attribute("OwnerUserId");

    // The v1 fabricated posts are always created by Community
    if (uid.empty() || uid.as_int() != -1) {
        return false;
    }

    // The v1 fabricated posts always have one of two very specific, large IDs
    auto postId = row.attribute("Id");
    auto typeId = row.attribute("PostTypeId");

    if (postId.as_string() == std::string_view{"1000000001"}) {
        if (typeId.as_string() != std::string_view{"1"}) {
            [[unlikely]]
            spdlog::critical(
                "Found bogus ID, but PostTypeId differs. Expected 1, found {}. The watermark has changed", 
                typeId.as_string()
            );
            throw std::runtime_error("Critical: watermark has changed");
        }
        return true;
    } else if (postId.as_string() == std::string_view{"1000000010"}) {
        if (typeId.as_string() != std::string_view{"2"}) {
            [[unlikely]]
            spdlog::critical(
                "Found bogus ID, but PostTypeId differs. Expected 2, found {}. The watermark has changed", 
                typeId.as_string()
            );
            throw std::runtime_error("Critical: watermark has changed");
        }
        return true;
    } else if (postId.as_string() == std::string_view{""}) {
        [[unlikely]]
        spdlog::critical("Failed to find the Id, or the Id field is blank. Is the data corrupt?");
    }

    return false;
}

std::string FabricatedDataV1Filter::getSummary() {
    return "Starting with the 2025-06-30 data dump, Stack Exchange started poisoning the data dumps with bogus data, "
        "making it harder to actually work with the data dumps. It and potential future watermarking methods, and "
        "info about the watermarks can be found "
        "[here](https://github.com/LunarWatcher/se-data-dump-transformer/blob/master/docs/meta/Known watermarks.md). "
        "This version of the data dump has been processed with a filter that removes the v1 watermarks.";
}

}
