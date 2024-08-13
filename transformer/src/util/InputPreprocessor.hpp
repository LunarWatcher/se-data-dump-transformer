#pragma once

#include "data/GlobalContext.hpp"
#include <vector>
#include <filesystem>

namespace sedd::InputPreprocessor {

extern std::vector<std::filesystem::path> screenArchives(GlobalContext& ctx);

}
