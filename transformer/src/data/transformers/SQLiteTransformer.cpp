#include "SQLiteTransformer.hpp"
#include "SQLiteCpp/Database.h"
#include "SQLiteCpp/Transaction.h"
#include "data/Schema.hpp"
#include "data/transformers/JSONTransformer.hpp"
#include "spdlog/fmt/bundled/format.h"
#include <memory>

namespace sedd {

void SQLiteTransformer::endFile() {
    this->transaction->commit();
}

void SQLiteTransformer::beginFile(const ParserContext&) {
    this->transaction = std::make_shared<SQLite::Transaction>(*db);
}

void SQLiteTransformer::beginArchive(const ParserContext& ctx) {
    this->db = std::make_shared<SQLite::Database>(
        ctx.baseSiteName,
        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE
    );
    this->transaction = std::make_shared<SQLite::Transaction>(*db);
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
    for (const auto& [type, schema] : Schema::schema) {
        std::stringstream ss;
        ss << "CREATE TABLE IF NOT EXISTS " << type << "(";


        ss << ")";
        db->exec(
            ss.str()
        );
    }
}


}
