#include "data/Transformer.hpp"
#include "data/filter/AnswerBotFilter.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Verify answer bot removal", "[AIBotFilter]") {
    using namespace std::literals;

    sedd::AnswerBotFilter filter;

    std::string docstr = R"(
    <?xml version="1.0" encoding="utf-8"?>
    <posts>
        <row UserId="-2" />
        <row UserId="-1" />
        <row UserId="69420" />
        <row UserId="" />
        <row />
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
    REQUIRE(nodes.size() == 5);
    REQUIRE(nodes.at(0).attribute("UserId").as_string() == "-2"s);
    REQUIRE(nodes.at(1).attribute("UserId").as_string() == "-1"s);
    REQUIRE(nodes.at(2).attribute("UserId").as_string() == "69420"s);
    REQUIRE(nodes.at(3).attribute("UserId").as_string() == ""s);
    REQUIRE(nodes.at(4).attribute("UserId").empty());

    // Validate the filter itself
    // Note: this is a legacy filter, and the type is unused. POSTS was used because that's what's used here, but it
    // really does not matter.
    // Only row 1 should be discarded
    REQUIRE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(0)));
    // UserID -1 should not be discarded
    REQUIRE_FALSE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(1)));
    // Positive UIDs should not be discarded
    REQUIRE_FALSE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(2)));
    // Blank and non-existent UIDs should not be discarded
    REQUIRE_FALSE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(3)));
    REQUIRE_FALSE(filter.process(sedd::DataDumpFileType_t::POSTS, nodes.at(4)));

}
