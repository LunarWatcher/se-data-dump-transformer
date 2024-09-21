#include "InputPreprocessor.hpp"

#include "data/ArchiveParser.hpp"
#include "data/GlobalContext.hpp"

namespace sedd {

std::vector<std::filesystem::path> InputPreprocessor::screenArchives(GlobalContext& ctx) {
    std::vector<std::filesystem::path> dirIt;
    for (const auto& entry : std::filesystem::directory_iterator(ctx.sourceDir)) {
        if (entry.is_directory() || entry.path().extension() != ".7z") {
            continue;
        }
        dirIt.push_back(entry.path());
    }

    std::vector<std::filesystem::path> filteredDirs;
    for (const auto& entry : dirIt) {

        if (ctx.checkNesting) {
            ArchiveParser p(entry);

            auto newArchives = p.checkExtractSubarchives(
                ctx.subarchiveDir
            );

            if (!newArchives.empty()) {
                // The archive has subarchives
                filteredDirs.insert(
                    filteredDirs.end(),
                    newArchives.begin(),
                    newArchives.end()
                );
                continue;
            }
        }

        filteredDirs.emplace_back(entry);
    }

    return filteredDirs;
}

}
