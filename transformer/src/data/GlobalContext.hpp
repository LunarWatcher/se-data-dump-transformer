#pragma once

#include "data/filter/Filter.hpp"
#include <filesystem>

namespace sedd {

class Transformer;
struct GlobalContext {
    std::filesystem::path sourceDir;
    std::filesystem::path destDir;
    std::filesystem::path subarchiveDir;
    std::shared_ptr<Transformer> transformer;
    bool recover;
    bool checkNesting;

    std::vector<std::shared_ptr<Filter>> enabledFilters;
};

}
