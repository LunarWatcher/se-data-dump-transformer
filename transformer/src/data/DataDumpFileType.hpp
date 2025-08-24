#pragma once

#include <string>

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
        UNKNOWN
    };

    extern DataDumpFileType strToFiletype(const std::string&);
    extern std::string filetypeToStr(DataDumpFileType);
    extern std::string toFilename(DataDumpFileType);
}
using DataDumpFileType_t = DataDumpFileType::DataDumpFileType;
}
