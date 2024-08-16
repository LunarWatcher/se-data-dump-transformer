#include "JSONTransformer.hpp"
#include "data/ArchiveWriter.hpp"
#include "data/Schema.hpp"
#include "spdlog/spdlog.h"
#include "util/StringSanitiser.hpp"
#include <sstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace sedd {

void JSONTransformer::beginFile(const ParserContext& ctx) {
    this->writer->open(DataDumpFileType::toFilename(ctx.currType) + ".json");
    this->writer->write("[\n");
    started = false;
}

void JSONTransformer::endFile() {
    this->writer->write("\n]\n");
    started = false;
    this->writer->close();
}

void JSONTransformer::beginArchive(const ParserContext& ctx) {
    auto outputDir = ctx.conf.destDir / ctx.baseDomain;
    
    this->writer = std::make_shared<ArchiveWriter>(
        outputDir
    );
}

void JSONTransformer::endArchive(const ParserContext&) {
    this->writer->commit();
    this->writer = nullptr;
}

void JSONTransformer::parseLine(const pugi::xml_node& row, const ParserContext& ctx) {
    const auto& types = Schema::schema.at(ctx.currType);

    // Using nlohmann/json was not my first choice, and is not optimal here
    // Given the large amount of data, performance is incredibly important. 
    // Unfortunately:
    // * yyjson struggles with utf8, and fails to convert the 2024-06-30 data dump
    //   which came with unicode encoding errors.
    // * rapidjson has not released a new version since 2016, and has multiple vulnerabilities
    // * simdjson (currently) doesn't seem to support serialisation, only reading,
    //   which is useless here.
    //
    // Until yyjson starts working or simdjson is fixed, I guess we're doing a suboptimal
    // solution
    
    nlohmann::json root = nlohmann::json::object();

    for (const auto& attr : row.attributes()) {
        // TODO: check if the second condition is necessary or not
        if (attr.empty() || attr.value() == nullptr) {
            //yyjson_mut_obj_add_null(*jw, obj, attr.name());
            root[attr.name()] = nullptr;
            continue;
        }

        switch (types.at(attr.name()).type) {
        case Schema::LONG:
            //yyjson_mut_obj_add_int(*jw, obj, attr.name(), attr.as_llong());
            root[attr.name()] = attr.as_llong();
            break;
        case Schema::DOUBLE:
            //yyjson_mut_obj_add_real(*jw, obj, attr.name(), attr.as_double());
            root[attr.name()] = attr.as_double();
            break;
        case Schema::BOOL:
            //yyjson_mut_obj_add_bool(*jw, obj, attr.name(), attr.as_bool());
            root[attr.name()] = attr.as_bool();
            break;
        case Schema::STRING:
        case Schema::DATE:
            //yyjson_mut_obj_add_str(*jw, obj, attr.name(), stringLifecycleContainer.back().c_str());
            root[attr.name()] = StringSanitiser::cleanString(attr.as_string());
            break;
        [[unlikely]] default:
            throw std::runtime_error("Invalid type for field " + std::string(attr.name()));
        }
    }

    if (this->started) {
        this->writer->write(",\n");
    } else {
        this->started = true;
    }
    this->writer->write(root.dump());
}

}
