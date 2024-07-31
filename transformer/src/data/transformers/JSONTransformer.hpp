#pragma once

#include "data/ArchiveWriter.hpp"
#include "data/Transformer.hpp"

namespace sedd {

class JSONTransformer : public Transformer {
private:
    std::shared_ptr<ArchiveWriter> writer;
    bool started = false;
public:
    JSONTransformer() : Transformer(false) {}

    void endFile() override;
    void beginFile(const ParserContext& ctx) override;
    void beginArchive(const ParserContext& ctx) override;
    void endArchive(const ParserContext& ctx) override;

    void parseLine(const pugi::xml_node& row, const ParserContext& ctx) override;
};

}
