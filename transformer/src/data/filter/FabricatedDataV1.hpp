#pragma once

#include "data/ArchiveParser.hpp"
#include "data/filter/Filter.hpp"

namespace sedd {

class FabricatedDataV1Filter : public Filter {
public:
    FabricatedDataV1Filter() : Filter(
        "--filter-fabricated-v1",
        "Whether or not to filter out v1 of the fabricated data. This version was used from 2025-06-30. "
        "See docs/meta/Known watermarks.md"
    ) {}

    bool process(DataDumpFileType_t type, pugi::xml_node& row) override;
    std::string getSummary() override;
};

}
