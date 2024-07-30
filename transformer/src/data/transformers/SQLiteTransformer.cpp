#include "SQLiteTransformer.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Transaction.h"
#include "data/ArchiveParser.hpp"
#include "data/Schema.hpp"
#include "data/transformers/JSONTransformer.hpp"
#include "data/util/ArchiveCache.hpp"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/spdlog.h"
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <variant>

namespace sedd {

void SQLiteTransformer::endFile() {
    this->transaction->commit();
}

void SQLiteTransformer::beginFile(const ParserContext& ctx) {
    this->transaction = std::make_shared<SQLite::Transaction>(*db);
    currSchema = Schema::schema.at(ctx.currType);
    cache.registerType(ctx.baseSiteName, ctx.currType);
}

void SQLiteTransformer::beginArchive(const ParserContext& ctx) {
    std::filesystem::path fn = ctx.conf.destDir / (ctx.baseSiteName + ".sqlite3");
    cache.initArchive(ctx, fn.filename().string());
    spdlog::debug("Opening {}", fn.string());
    this->db = std::make_shared<SQLite::Database>(
        fn,
        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
    );
    genTables();
}

void SQLiteTransformer::endArchive(const ParserContext& ctx) {
    this->transaction = nullptr;
    this->db = nullptr;

    cache.checkComplete(ctx.baseSiteName);
}

void SQLiteTransformer::parseLine(const pugi::xml_node& row, const ParserContext& ctx) {

    std::stringstream ss;
    std::vector<std::variant<int64_t, double, std::string, std::nullptr_t>> rawInputs;

    ss << "INSERT INTO " << ctx.currTypeStr << " (";

    bool prevField = false;
    size_t fieldCount = 0;
    for (auto& attr : row.attributes()) {
        ++fieldCount;
        auto name = std::string(attr.name());
        auto strval = std::string(attr.value());

        auto& fieldDescriptor = this->currSchema.at(name);
        if (!prevField) {
            prevField = true; 
        } else {
            ss << ", ";
        }

        ss << name;

        // If the field isn't nullable, fall back to the defaults from the XML parser
        if (strval.size() == 0 && fieldDescriptor.nullable) {
            rawInputs.push_back(nullptr);
        } else {
            switch (fieldDescriptor.type) {
            case Schema::LONG:
                rawInputs.push_back(attr.as_llong());
                break;
            case Schema::DOUBLE:
                rawInputs.push_back(attr.as_double());
                break;
            case Schema::STRING:
                rawInputs.push_back(strval);
                break;
            case Schema::DATE:
                rawInputs.push_back(strval);
                break;
            case Schema::BOOL:
                rawInputs.push_back(attr.as_bool());
                break;
            default:
                throw std::runtime_error("Unknown field type");
            }
        }
    }
    ss << ") VALUES (";
    prevField = false;
    for (size_t i = 0; i < fieldCount; ++i) {
        if (!prevField) {
            prevField = true;
        } else ss << ", ";
        ss << "?";
    }
    ss << ")";

    SQLite::Statement stmt(*db, ss.str());

    for (size_t i = 0; i < rawInputs.size(); ++i) {
        auto& v = rawInputs.at(i);
        std::visit([&](const auto& arg) {
            // Bind is 1-indexed
            stmt.bind(i + 1, arg);
        }, v);
    }

    stmt.exec();

    // This helps speed up writes by removing some of the IO bottleneck. Writing bigger chunks generally seems to be
    // better for performance, but obviously at the expense of more memory use.
    // I'm not sure if 100k is an optimal number for this, but this is open for exploration Later:tm:
    if (++rowCount > 100000) {
        rowCount = 0;
        transaction->commit();
        this->transaction = std::make_shared<SQLite::Transaction>(*db);
    }
}

void SQLiteTransformer::genTables() {
    for (const auto& [filetype, schema] : Schema::schema) {
        std::stringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS " << DataDumpFileType::filetypeToStr(filetype) << "(\n";
        bool hasField = false;

        for (const auto& [k, spec] : schema) {
            if (!hasField) {
                hasField = true;
            } else {
                ss << ",\n";
            }
            // Is the tab necessary? 
            // No.
            // Does it make .schema look better?
            // Yes.
            // :)
            ss << "\t" << k << " ";
            switch (spec.type) {
            case Schema::LONG:
                ss << "INTEGER ";
                break;
            case Schema::DOUBLE:
                ss << "REAL ";
                break;
            case Schema::STRING:
                ss << "TEXT ";
                break;
            case Schema::DATE:
                // SQLite dates are TEXT, REAL, or INTEGER, and converted into dates with date functions
                ss << "DATETIME ";
                break;
            case Schema::BOOL:
                ss << "BOOLEAN ";
                break;
            }

            if (!spec.nullable) ss << "NOT NULL ";
            if (spec.pk) ss << "PRIMARY KEY ";
        }

        ss << "\n)";
        spdlog::debug("Running {}", ss.str());
        db->exec(
            ss.str()
        );
    }
}

}
