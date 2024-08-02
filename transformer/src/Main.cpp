#include <archive.h>
#include <atomic>
#include <execution>
#include <filesystem>
#include <memory>
#include <mutex>
#include <semaphore>
#include "CLI/CLI.hpp"
#include "data/ArchiveParser.hpp"
#include "data/GlobalContext.hpp"
#include "data/transformers/SQLiteTransformer.hpp"
#include "spdlog/cfg/helpers.h"
#include <stc/StringUtil.hpp>

#include "data/transformers/JSONTransformer.hpp"

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>
#include <map>

enum class TransformerType {
    JSON,
    SQLITE,

    DRY_RUN,
};

class InvalidTransformer : public sedd::Transformer {};

std::map<std::string, TransformerType> strToTransformer {
    {"json", TransformerType::JSON},
    {"sqlite", TransformerType::SQLITE},
    {"noop", TransformerType::DRY_RUN},
};

std::shared_ptr<sedd::Transformer> getTransformer(TransformerType type) {
#define SEDD_TRANSFORMER(type, ret) {TransformerType::type, []() { spdlog::info("Using transformer " #type); return ret; }}

    static auto map = std::map<TransformerType, std::function<std::shared_ptr<sedd::Transformer>()>> {
        SEDD_TRANSFORMER(JSON, std::make_shared<sedd::JSONTransformer>()),
        SEDD_TRANSFORMER(SQLITE, std::make_shared<sedd::SQLiteTransformer>()),

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

    TransformerType transformerType;
    app.add_option("-t,--transformer", transformerType, 
                   "The transformer to use. Note that the `noop` transformer is special as it does nothing "
                   "to the input. Should only be used to verify the file parsing without running a parser.")
        ->required()
        ->transform(CLI::CheckedTransformer(strToTransformer, CLI::ignore_case));

    unsigned int threads = 1;
    app.add_option(
        "-j,--threads",
        threads, 
        "How many threads to use for processing. Note that more threads use more RAM"
    )
        ->required(false)
        ->default_val(1)
        ->check(CLI::PositiveNumber);
    bool recover = true;
    app.add_option(
        "-r,--recover",
        recover,
        "Whether or not to recover from errors"
    )
        ->required(false)
        ->default_val(recover);

    CLI11_PARSE(app, argc, argv);

    auto level = std::getenv("SPDLOG_LEVEL");

    spdlog::cfg::helpers::load_levels(level == nullptr ? "info" : level);

    sedd::GlobalContext baseCtx {
        .sourceDir = downloadDir,
        .destDir = [&]() -> std::filesystem::path {
            if (parseDir.size()) return parseDir;
            return std::filesystem::path(downloadDir) / "../out";
        }(),
        .transformer = nullptr,
        .recover = recover
    };

    std::filesystem::create_directories(baseCtx.destDir);
    spdlog::info("Configuration:");
    spdlog::info("Files: [source = {}, dest = {}]", baseCtx.sourceDir.string(), baseCtx.destDir.string());

    // At least MSVC doesn't like using directory_iterator directly in a parallel for_each because
    //      Parallel algorithms require forward iterators or stronger.
    // Which doesn't make sense to me, but it is what it is
    std::vector<std::filesystem::path> dirIt;
    for (auto& entry : std::filesystem::directory_iterator(baseCtx.sourceDir)) {
        if (entry.is_directory() || entry.path().extension() != ".7z") {
            continue;
        }
        dirIt.push_back(entry.path());
    }


    std::counting_semaphore concurrencyGuard(threads);

    std::map<std::string, std::mutex> locks;
    std::mutex lockGuard;

    std::atomic<int> processed = 0;

    std::for_each(
#ifndef __APPLE__
        // Crapple? No parallel execution for you, because Apple Clang still doesn't support it
        std::execution::par,
#endif
        dirIt.begin(), dirIt.end(),
        [&](const auto& entry) {
            auto siteID = stc::string::split(entry.filename().replace_extension().string(), '-', 1)[0];
            spdlog::debug("Preparing to lock {}", siteID);
            // Acquire the site-specific mutex
            // Not sure if this is necessary or if mutex acqusition is atomic, but better safe than sorry
            // Also, this has functionally 0 overhead commpared to the archive processing. Literally hours
            // to compress a file - sheesh
            std::unique_lock<std::mutex> mapLock(lockGuard);
            auto& siteMutex = locks[siteID];
            mapLock.unlock();

            // Lock the site-specific mutex. For most sites, no threads will be suspended here
            std::lock_guard l(siteMutex);

            // Limit concurrency to the specified number of threads
            concurrencyGuard.acquire();

            spdlog::info("Now processing {}", entry.string());

            auto parser = sedd::ArchiveParser(entry);
            // Dupe the global context and reinit the transformer
            sedd::GlobalContext ctx = baseCtx;
            ctx.transformer = getTransformer(transformerType);
            parser.read(ctx);

            concurrencyGuard.release();

            spdlog::info("{}/{} done", processed.fetch_add(1), dirIt.size());
        }
    );

#ifdef _WIN32
    // No errors made me miss a segfault
    // How people work with this garbage OS without going insane is beyond me
    spdlog::info("Windows hides certain error categories and just makes the exit look normal and expected (outside MSVS anyway), so this is your confirmation that everything worked fine");
#endif

    return 0;
}
