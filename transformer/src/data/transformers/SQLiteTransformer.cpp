#include "SQLiteTransformer.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Transaction.h"
#include "data/Schema.hpp"
#include "data/transformers/JSONTransformer.hpp"
#include "spdlog/fmt/bundled/format.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace sedd {

void SQLiteTransformer::endFile() {
    this->transaction->commit();
}

void SQLiteTransformer::beginFile(const ParserContext&) {
    this->transaction = std::make_shared<SQLite::Transaction>(*db);
}

void SQLiteTransformer::beginArchive(const ParserContext& ctx) {
    std::string fn = (ctx.conf.destDir / (ctx.baseSiteName + ".sqlite3")).string();
    spdlog::debug("Opening {}", fn);
    this->db = std::make_shared<SQLite::Database>(
        fn,
        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
    );
    genTables();
}

void SQLiteTransformer::endArchive(const ParserContext&) {
    this->transaction = nullptr;
    this->db = nullptr;
}

void SQLiteTransformer::parseLine(const pugi::xml_node& row, const ParserContext& ctx) {

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
                ss << "TEXT ";
                break;
            case Schema::BOOL:
                // SQLite bools are ints
                ss << "INTEGER ";
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
