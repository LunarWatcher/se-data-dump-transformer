#pragma once

#include <filesystem>

namespace sedd {

class Transformer;
struct GlobalContext {
    std::filesystem::path sourceDir;
    std::filesystem::path destDir;
    std::filesystem::path subarchiveDir;
    std::shared_ptr<Transformer> transformer;
    bool recover;
};

}
