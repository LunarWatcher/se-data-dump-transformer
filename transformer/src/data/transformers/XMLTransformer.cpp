#include "XMLTransformer.hpp"
#include "data/ArchiveParser.hpp"
#include "data/Transformer.hpp"
#include "spdlog/spdlog.h"
#include <format>

namespace sedd {

void XMLTransformer::beginArchive(const ParserContext& ctx) {
    auto outputDir = ctx.conf.destDir / ctx.baseDomain;
    
    this->writer = std::make_shared<ArchiveWriter>(
        outputDir
    );
}

void XMLTransformer::beginFile(const ParserContext& ctx) {
    auto filename = DataDumpFileType::toFilename(ctx.currType) + ".xml";
    spdlog::debug("Starting new file: {}", filename);

    this->writer->open(filename, FileAttr { 
        ctx.lastModified
    });
    this->writer->write(R"(<?xml version="1.0" encoding="utf-8"?>)");
    this->writer->write(R"(
<!--
License URLs:
  CC BY-SA 2.5: https://creativecommons.org/licenses/by-sa/2.5/
  CC BY-SA 3.0: https://creativecommons.org/licenses/by-sa/3.0/
  CC BY-SA 4.0: https://creativecommons.org/licenses/by-sa/4.0/
-->)");
    this->writer->write(std::format("\n<{}>\n", DataDumpFileType::filetypeToStr(ctx.currType)));

    ft = ctx.currType;
}

void XMLTransformer::endFile() {
    this->writer->write(std::format("</{}>\n", DataDumpFileType::filetypeToStr(ft.value())));
    this->writer->close();

    ft.reset();
}

void XMLTransformer::endArchive(const ParserContext&) {
    this->writer->commit();
    this->writer = nullptr;
}

void XMLTransformer::parseLine(const pugi::xml_node& row, const ParserContext&) {
    std::stringstream ss;
    row.print(ss, "  ");

    this->writer->write(std::format("  {}", ss.str()));
}

}
