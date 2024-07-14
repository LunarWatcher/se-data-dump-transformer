#include <archive.h>
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>
#include "data/ArchiveParser.hpp"
#include "spdlog/cfg/helpers.h"

#include <spdlog/spdlog.h>

int main(int argc, char* argv[]) {
    using namespace std::literals;
    if (argc != 2) {
        std::cerr << "Usage: sedd-transformer ./path/to/download/directory" << std::endl;
        return -1;
    }
    auto level = std::getenv("SPDLOG_LEVEL");

    spdlog::cfg::helpers::load_levels(level == nullptr ? "info" : level);

    std::filesystem::path directory = argv[1];
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        spdlog::info("Now processing {}", entry.path().string());

        auto parser = sedd::ArchiveParser(entry);
        parser.read();

        std::this_thread::sleep_for(10s);

    }

    std::this_thread::sleep_for(10s);
    return 0;
}
