#include "data/ArchiveParser.hpp"
#include "data/Schema.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Check filetype type integrity", "[SanityCheck]") {

    for (int i = sedd::DataDumpFileType::BADGES; i != sedd::DataDumpFileType::UNKNOWN; ++i) {
        auto f = static_cast<sedd::DataDumpFileType::DataDumpFileType>(i);

        // Check if both string conversion functions have all the types
        REQUIRE_NOTHROW(sedd::DataDumpFileType::filetypeToStr(f));
        REQUIRE_NOTHROW(sedd::DataDumpFileType::toFilename(f));

        // Check if the schema has all the required fields
        REQUIRE_NOTHROW(sedd::Schema::schema.at(f));
    }

}
