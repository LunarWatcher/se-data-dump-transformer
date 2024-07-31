#pragma once

#include "SQLiteCpp/Transaction.h"
#include "data/Schema.hpp"
#include "data/Transformer.hpp"
#include "data/util/ArchiveCache.hpp"

#include <SQLiteCpp/SQLiteCpp.h>

namespace sedd {

class SQLiteTransformer : public Transformer {
private:
    std::shared_ptr<SQLite::Database> db;
    std::shared_ptr<SQLite::Transaction> transaction;
    long long rowCount = 0;

    std::map<std::string, Schema::Field> currSchema;
    static inline ArchiveCache cache;

    void genTables();

public:
    SQLiteTransformer() : Transformer(true, ".sqlite3") {}
    void endFile() override;
    void beginFile(const ParserContext& ctx) override;
    void beginArchive(const ParserContext& ctx) override;
    void endArchive(const ParserContext& ctx) override;

    void parseLine(const pugi::xml_node& row, const ParserContext& ctx) override;
};

}
