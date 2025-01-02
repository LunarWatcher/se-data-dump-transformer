#pragma once

#include "data/ArchiveWriter.hpp"
#include "data/Transformer.hpp"
#include <optional>

namespace sedd {

class XMLTransformer : public Transformer {
private:
    std::shared_ptr<ArchiveWriter> writer;
    bool started = false;

    std::optional<DataDumpFileType::DataDumpFileType> ft;
public:
    XMLTransformer() : Transformer(false) {}

    void endFile() override;
    void beginFile(const ParserContext& ctx) override;
    void beginArchive(const ParserContext& ctx) override;
    void endArchive(const ParserContext& ctx) override;

    void parseLine(const pugi::xml_node& row, const ParserContext& ctx) override;

    std::string getDescription() override {
        return "This XML-formatted version of the data dump is NOT the same as the official version, as it has been "
            "reprocessed through github.com/LunarWatcher/se-data-dump-transformer. Unless otherwise noted in the data "
            "alterations section, the data itself is unmodified from the source data dump, aside formatting changes "
            "and potentially removing fingerprinting, depending on the source data dump.";
    }
};

}
