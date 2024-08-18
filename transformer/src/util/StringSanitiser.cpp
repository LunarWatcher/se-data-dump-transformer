#include "StringSanitiser.hpp"
#include "spdlog/spdlog.h"
#include "stc/StringUtil.hpp"

#include <locale>
#include <sstream>
#include <string>
#include <utf8.h>

namespace sedd {

std::string StringSanitiser::cleanString(std::string in) {

    stc::string::replaceAll(in, std::string("\0", 1), "\uFFFD"); // NOLINT(bugprone-*)
    std::u8string v(
        in.cbegin(), in.cend()
    );
    if (utf8valid(v.data()) != 0 && utf8makevalid(v.data(), U'\uFFFD') != 0) {
        spdlog::error("input string: {}", in);
        throw std::runtime_error("utf8makevalid failed");
    }

    return {
        reinterpret_cast<const char*>(v.data()),
        v.size()
    };
}

std::string StringSanitiser::clearNullBytes(std::string in) {
    stc::string::replaceAll(in, "&#x00;", "&#xfffd;");
    return in;
}

}
