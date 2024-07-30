#include "ArchiveCache.hpp"
#include "data/ArchiveParser.hpp"
#include "data/util/ArchiveCache.hpp"
#include "data/Schema.hpp"
#include <spdlog/spdlog.h>

namespace sedd {

void ArchiveCache::initArchive(const ParserContext& ctx, const std::string& binFile) {
    std::lock_guard l(mut);
    if (siteColumnMaps.contains(ctx.baseSiteName)) {
        return;
    }

    SiteCache cache {
        .writer = std::make_shared<ArchiveWriter>(
            ctx.conf.destDir / binFile,
            ctx.conf.destDir,
            false
        ),
        .exhaustedFiletypes = {}
    };

    cache.writer->addBinaryFile(binFile);

    siteColumnMaps[ctx.baseSiteName] = cache;
}

void ArchiveCache::registerType(const std::string& site, DataDumpFileType::DataDumpFileType type) {
    std::lock_guard l(mut);
    siteColumnMaps.at(site).exhaustedFiletypes.insert(type);
}

void ArchiveCache::checkComplete(const std::string& site) {
    std::lock_guard l(mut);
    {
        const auto& schema = Schema::schema;
        const auto& siteCache = siteColumnMaps.at(site);

        if (schema.size() != siteCache.exhaustedFiletypes.size()) {
            spdlog::debug("{} is not ready to commit (found {}, need {})",
                          site,
                          siteCache.exhaustedFiletypes.size(),
                          schema.size());
            return;
        }

        // No point in checking if the values are identical; the set means that if the sizes match,
        // all the types are added
        
        spdlog::info("Committing binary file for {}", site);
        siteCache.writer->commit();
        spdlog::info("Committing {} done", site);
    }

    siteColumnMaps.erase(siteColumnMaps.find(site));

}

}
