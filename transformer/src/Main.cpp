#include <archive.h>
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>
#include "CLI/CLI.hpp"
#include "data/ArchiveParser.hpp"
#include "spdlog/cfg/helpers.h"

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

enum class TransformerType {
    JSON,

    DRY_RUN,
};

std::map<std::string, TransformerType> strToTransformer {
    {"json", TransformerType::JSON},
    {"noop", TransformerType::DRY_RUN},
};

int main(int argc, char* argv[]) {
    using namespace std::literals;
    
    CLI::App app{"sedd-transformer: A format transformer for the Stack Exchange data dump"};
    argv = app.ensure_utf8(argv);

    std::string downloadDir;
    app.add_option("-i,--input", downloadDir, "Path to the directory containing the data dump files")
        ->required(true);
    std::string transformer;
    app.add_option("-t,--transformer", transformer, 
                   "The transformer to use. Note that the `noop` transformer is special as it does nothing "
                   "to the input. Should only be used to verify the file parsing without running a parser.")
        ->required()
        ->transform(CLI::CheckedTransformer(strToTransformer, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    auto level = std::getenv("SPDLOG_LEVEL");

    spdlog::cfg::helpers::load_levels(level == nullptr ? "info" : level);

    std::filesystem::path directory = downloadDir;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        spdlog::info("Now processing {}", entry.path().string());

        auto parser = sedd::ArchiveParser(entry);
        parser.read();

        std::this_thread::sleep_for(10s);

    }

    std::this_thread::sleep_for(10s);
    return 0;
}
