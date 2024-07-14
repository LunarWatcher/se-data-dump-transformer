#pragma once

#include <pugixml.hpp>

namespace sedd {

class Transformer {
public:
    virtual void parseLine(const pugi::xml_node& row) = 0;
};

}
