#include "util/StringSanitiser.hpp"
#include <catch2/catch_test_macros.hpp>

// -1 to account for the null byte in a const char*
#define FORCE_NULLSTR(str) std::string(str, sizeof(str) - 1) 

TEST_CASE("Check invalid codepoint conversion", "[Fixes]") {
    REQUIRE(
        sedd::StringSanitiser::cleanString(FORCE_NULLSTR("test \0 test")) == "test \uFFFD test" // NOLINT
    );
}

TEST_CASE("Check valid unicode handling", "[Fixes]") {
    REQUIRE(
        sedd::StringSanitiser::cleanString(R"(<row Id="12" UserId="9" Name="Teacher" Date="2010-08-05T19:51:16.710" Class="3" />)")
        == R"(<row Id="12" UserId="9" Name="Teacher" Date="2010-08-05T19:51:16.710" Class="3" />)"
    );
}
