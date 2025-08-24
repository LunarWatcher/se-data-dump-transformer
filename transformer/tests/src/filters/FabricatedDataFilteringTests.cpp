#include "data/DataDumpFileType.hpp"
#include "data/Transformer.hpp"
#include "data/filter/FabricatedDataV1.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Verify v1 fabricated data removal", "[FabricatedDataV1]") {
    using namespace std::literals;

    sedd::FabricatedDataV1Filter filter;

    std::string docstr = R"(
    <?xml version="1.0" encoding="utf-8"?>
    <posts>
        <row OwnerUserId="69420" />
        <row Id="1000000001" PostTypeId="1" CreationDate="2025-06-01T01:00:00.100" Score="1" ViewCount="100" Body="Stack Exchange hates their community" OwnerUserId="-1" LastEditorUserId="-1" LastEditDate="2025-06-01T01:00:00.100" LastActivityDate="2025-06-01T01:00:00.100" Title="irrelevant" Tags="trans rights" AnswerCount="1" CommentCount="0" />
        <row Id="1000000010" PostTypeId="2" CreationDate="2025-06-01T01:03:15.100" Score="1" Body="Fucking bullshit" OwnerUserId="-1" LastEditorUserId="-1" LastEditDate="2025-06-01T01:03:15.100" LastActivityDate="2025-06-01T01:03:15.100" CommentCount="0" />
    </posts>)";

    pugi::xml_document doc;
    REQUIRE(
        doc.load_string(
            docstr.c_str(),
            (pugi::parse_default | pugi::encoding_utf8 | pugi::parse_ws_pcdata_single) & ~pugi::parse_escapes
        )
    );

    // Why the fuck is root() empty??
    auto root = doc.first_child();
    REQUIRE(root.name() == std::string("posts"));

    std::vector<pugi::xml_node> nodes;

    for (auto& child : root.children()) {

        nodes.push_back(child);
    }

    // Validate parsing for the test
    REQUIRE(nodes.size() == 3);

    REQUIRE_FALSE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(0)));

    INFO(nodes.at(1).attribute("OwnerUserId").as_string());
    INFO(nodes.at(1).attribute("Id").as_string());
    REQUIRE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(1)));

    INFO(nodes.at(2).attribute("OwnerUserId").as_string());
    INFO(nodes.at(2).attribute("Id").as_string());
    REQUIRE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(2)));

    // Other types should not be affected
    REQUIRE_FALSE(filter.process(sedd::DataDumpFileType_t::POST_HISTORY, nodes.at(2)));
}
