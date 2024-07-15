#pragma once

#include "data/Transformer.hpp"
#include <filesystem>

namespace sedd {

class JSONTransformer : public Transformer {
public:
    JSONTransformer();

    virtual void parseLine(const pugi::xml_node& row, const ParserContext& ctx) = 0;
};

}
