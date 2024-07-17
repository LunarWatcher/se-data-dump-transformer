#include "JSONTransformer.hpp"
#include "data/ArchiveWriter.hpp"

namespace sedd {

void JSONTransformer::beginFile(const ParserContext& ctx) {
    this->writer->open(DataDumpFileType::toFilename(ctx.currType) + ".json");
    this->writer->write("[\n");
}

void JSONTransformer::endFile() {
    this->writer->write("]");
    this->writer->close();
}

void JSONTransformer::beginArchive(const ParserContext& ctx) {
    auto outputDir = ctx.conf.destDir / ctx.site;
    
    this->writer = std::make_shared<ArchiveWriter>(
        outputDir
    );
}

void JSONTransformer::endArchive(const ParserContext& ctx) {
    this->writer->commit();
    this->writer = nullptr;
}

void JSONTransformer::parseLine(const pugi::xml_node& row, const ParserContext& ctx) {
    this->writer->write("This is where a line would go\n");
}

}
