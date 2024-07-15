#pragma once

#include <pugixml.hpp>
#include "ArchiveParser.hpp"

namespace sedd {

class Transformer {
public:
    virtual void parseLine(const pugi::xml_node& row, const ParserContext& ctx) = 0;
};

}
