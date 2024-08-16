#pragma once

#include <string>

namespace sedd::StringSanitiser {

extern std::string cleanString(std::string in);
extern std::string clearNullBytes(std::string in);

}
