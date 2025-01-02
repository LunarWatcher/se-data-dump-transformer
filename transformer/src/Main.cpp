#include <archive.h>
#include <atomic>
#include <execution>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <semaphore>
#include "CLI/CLI.hpp"
#include "data/ArchiveParser.hpp"
#include "data/ArchiveWriter.hpp"
#include "data/GlobalContext.hpp"
#include "data/filter/AnswerBotFilter.hpp"
#include "data/filter/Filter.hpp"
#include "data/transformers/SQLiteTransformer.hpp"
#include "data/transformers/XMLTransformer.hpp"
#include "meta/MetaFiles.hpp"
#include <stc/StringUtil.hpp>
#include <fmt/format.h>

#include "data/transformers/JSONTransformer.hpp"
#include "spdlog/cfg/helpers-inl.h"
#include "spdlog/common.h"
#include "util/InputPreprocessor.hpp"

#include <spdlog/spdlog.h>
#include <map>

enum class TransformerType {
    JSON,
    SQLITE,
    XML,

    DRY_RUN,
};

std::map<std::string, TransformerType> strToTransformer {
    {"json", TransformerType::JSON},
    {"sqlite", TransformerType::SQLITE},
    {"xml", TransformerType::XML},
    {"noop", TransformerType::DRY_RUN},
};

std::shared_ptr<sedd::Transformer> getTransformer(TransformerType type) {
#define SEDD_TRANSFORMER(type, ret) {TransformerType::type, []() { spdlog::info("Using transformer " #type); return ret; }}

    static auto map = std::map<TransformerType, std::function<std::shared_ptr<sedd::Transformer>()>> {
        SEDD_TRANSFORMER(JSON, std::make_shared<sedd::JSONTransformer>()),
        SEDD_TRANSFORMER(SQLITE, std::make_shared<sedd::SQLiteTransformer>()),
        SEDD_TRANSFORMER(XML, std::make_shared<sedd::XMLTransformer>()),

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
    app.add_option("-t,--transformer", 
                   transformerType, 
                   "The transformer to use. Note that the `noop` transformer is special as it does nothing "
                   "to the input. Should only be used to verify the file parsing without running a parser.")
        ->required()
        ->transform(CLI::CheckedTransformer(strToTransformer, CLI::ignore_case));

    unsigned int threads = 1;
    app.add_option(
        "-j,--threads",
        threads, 
        "How many threads to use for processing. Note that more threads use more RAM. Also note  that it isn't possible to use more threads"
        " than the hardware allows"
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

    bool includeReadme = true;
    app.add_flag(
        "--readme,!--no-readme",
        includeReadme,
        "Whether or not to include a README in the output directory. "
        "To best comply with CC-By-SA, this option does not affect the LICENSE file included "
        "in each archive"
    )
        ->required(false)
        ->default_val(true);

    bool checkNesting = true;
    app.add_flag(
        "--check-nesting,!--no-nesting",
        checkNesting,
        "Whether or not to check for nested .7zs. DO NOT SET TO FALSE UNLESS YOU KNOW THE ARCHIVE IS GOOD! "
        "Setting this to false when there is nesting will have unintended consequences. All this option does is "
        "save a few seconds during setup. Unless you know what you're doing, leave this at the default value."
    )
        ->required(false)
        ->default_val(checkNesting);

    std::string logLevel;
    app.add_option(
        "-l,--log-level",
        logLevel,
        "The level to use for logging. Defaults to info."
    )
        ->required(false)
        ->default_val("info")
        ->check(
            CLI::IsMember(
                {"critical", "debug", "info", "warning", "error"},
                CLI::ignore_case, CLI::ignore_space
            )
        );

    sedd::OutputCompressionFormat outFormat;
    app.add_option(
        "-f,--output-format",
        outFormat,
        "The output format to use for compression. Currently unused as only .7z is supported"
    )
        ->required(false)
        ->default_val(sedd::OutputCompressionFormat::SEVENZIP)
        ->transform(CLI::CheckedTransformer(sedd::strToCompFormat, CLI::ignore_case));

    std::vector<std::shared_ptr<sedd::Filter>> filters = {
        std::make_shared<sedd::AnswerBotFilter>()
    };

    for (auto& filter : filters) {
        filter->init(app);
    }

    CLI11_PARSE(app, argc, argv);
    spdlog::cfg::helpers::load_levels(logLevel);

    std::erase_if(filters, [](const auto& filter) {
        return !filter->isEnabled();
    });
    spdlog::info("{} filters are enabled", filters.size());

    sedd::GlobalContext baseCtx {
        .sourceDir = downloadDir,
        .destDir = [&]() -> std::filesystem::path {
            if (!parseDir.empty()) {
                return parseDir;
            }
            return std::filesystem::path(downloadDir) / "../out";
        }(),
        .subarchiveDir = std::filesystem::path(downloadDir) / "_subarchives",
        .transformer = nullptr,
        .recover = recover,
        .checkNesting = checkNesting,
        .enabledFilters = filters
    };

    std::filesystem::create_directories(baseCtx.destDir);
    std::filesystem::create_directories(baseCtx.subarchiveDir);
    spdlog::info("Configuration:");
    spdlog::info("Files: [source = {}, dest = {}]", baseCtx.sourceDir.string(), baseCtx.destDir.string());
    spdlog::info("Running on {} thread(s)", threads);
    spdlog::info("Checking for .7z nesting? {}", checkNesting);

    // At least MSVC doesn't like using directory_iterator directly in a parallel for_each because
    //      Parallel algorithms require forward iterators or stronger.
    // Which doesn't make sense to me, but it is what it is
    spdlog::info("Preparing archive list...");
    std::vector<std::filesystem::path> dirIt = sedd::InputPreprocessor::screenArchives(baseCtx);
    spdlog::info("Archives ready.");

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

            spdlog::info("Progress: {}/{} conversions done", ++processed, dirIt.size());
        }
    );

    if (includeReadme && transformerType != TransformerType::DRY_RUN) {
        std::stringstream filterSummary;

        if (filters.size() == 0) {
            filterSummary << "No (intentional) changes to the data have been made by the processing system.";
        } else {
            filterSummary << "Some changes have been made to the data:\n";
            for (const auto& filter : filters) {
                filterSummary << "- " << filter->getSummary() << "\n";
            }
        }

        spdlog::info("Outputting README.md...");
        std::ofstream o(
            baseCtx.destDir / "README.md"
        );
        o << fmt::format(
            sedd::MetaFiles::readme,
            std::find_if(
                strToTransformer.begin(), strToTransformer.end(),
                 [&](const auto& p) {
                     return p.second == transformerType;
                 })->first,
            getTransformer(transformerType)->getDescription(),
            filterSummary.str()
        ) << std::endl;
    }

#ifdef _WIN32
    // No errors made me miss a segfault
    // How people work with this garbage OS without going insane is beyond me
    spdlog::info("Windows hides certain error categories and just makes the exit look normal and expected (outside MSVS anyway), so this is your confirmation that everything worked fine");
#endif

    return 0;
}
