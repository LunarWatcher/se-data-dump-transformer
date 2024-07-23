#include <archive.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "CLI/CLI.hpp"
#include "data/ArchiveParser.hpp"
#include "data/GlobalContext.hpp"
#include "spdlog/cfg/helpers.h"

#include "data/transformers/JSONTransformer.hpp"

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <map>

enum class TransformerType {
    JSON,

    DRY_RUN,
};

std::map<std::string, TransformerType> strToTransformer {
    {"json", TransformerType::JSON},
    {"noop", TransformerType::DRY_RUN},
};

std::shared_ptr<sedd::Transformer> getTransformer(TransformerType type) {
#define SEDD_TRANSFORMER(type, ret) {TransformerType::type, []() { spdlog::info("Using transformer " #type); return ret; }}

    static auto map = std::map<TransformerType, std::function<std::shared_ptr<sedd::Transformer>()>> {
        SEDD_TRANSFORMER(JSON, std::make_shared<sedd::JSONTransformer>()),

        SEDD_TRANSFORMER(DRY_RUN, nullptr),
    };

    return map.at(type)();
}

int main(int argc, char* argv[]) {
    using namespace std::literals;
    
    CLI::App app{"sedd-transformer: A format transformer for the Stack Exchange data dump"};
    argv = app.ensure_utf8(argv);

    std::string downloadDir;
    app.add_option("-i,--input", downloadDir, "Path to the directory containing the data dump files")
        ->required(true);
    std::string parseDir;
    app.add_option("-o,--output", parseDir, "Path to the directory to dump output files to")
        ->required(false);

    TransformerType transformer;
    app.add_option("-t,--transformer", transformer, 
                   "The transformer to use. Note that the `noop` transformer is special as it does nothing "
                   "to the input. Should only be used to verify the file parsing without running a parser.")
        ->required()
        ->transform(CLI::CheckedTransformer(strToTransformer, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    auto level = std::getenv("SPDLOG_LEVEL");

    spdlog::cfg::helpers::load_levels(level == nullptr ? "info" : level);

    sedd::GlobalContext ctx {
        .sourceDir = downloadDir,
        .destDir = [&]() -> std::filesystem::path {
            if (parseDir.size()) return parseDir;
            return std::filesystem::path(downloadDir) / "../out";
        }(),
        .transformer = getTransformer(transformer),
    };

    std::filesystem::create_directories(ctx.destDir);
    spdlog::info("Configuration:");
    spdlog::info("Files: [source = {}, dest = {}]", ctx.sourceDir.string(), ctx.destDir.string());

    for (const auto& entry : std::filesystem::directory_iterator(ctx.sourceDir)) {
        spdlog::info("Now processing {}", entry.path().string());

        auto parser = sedd::ArchiveParser(entry);
        parser.read(ctx);
    }

    return 0;
}
