#define CATCH_CONFIG_RUNNER
#include "catch2/catch_session.hpp"

int main(int argc, const char* argv[]) {
    return Catch::Session().run(argc, argv);
}
