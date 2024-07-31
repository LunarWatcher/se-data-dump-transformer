#include "Transformer.hpp"
#include "data/ArchiveParser.hpp"

namespace sedd {

Transformer::Transformer(bool shouldUnifyInputs, const std::string& extension) : shouldUnifyInputs(shouldUnifyInputs), extension(extension) {}

std::filesystem::path Transformer::getOutputArchivePath(const ParserContext& ctx) {
    return ctx.conf.destDir / ((shouldUnifyInputs ? ctx.baseSiteName : ctx.baseDomain) + extension + ".7z");
}

}
