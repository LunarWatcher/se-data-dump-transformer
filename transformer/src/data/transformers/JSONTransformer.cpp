#include "JSONTransformer.hpp"
#include "data/ArchiveWriter.hpp"
#include "data/Schema.hpp"
#include "spdlog/spdlog.h"
#include "util/StringSanitiser.hpp"
#include "wrappers/yyjson.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

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
    YYJsonWriter jw;
    yyjson_mut_val* obj = yyjson_mut_obj(*jw);
    if (obj == nullptr) {
        throw std::runtime_error("Failed to allocate JSON object");
    }
    yyjson_mut_doc_set_root(*jw, obj);

    const auto& types = Schema::schema.at(ctx.currType);

    // This read-only container is used to contain the strings from date and string types.
    // yyjson_mut_obj_add_str does not copy the string, so this is required to keep the strings valid
    std::vector<std::string> stringLifecycleContainer;
    stringLifecycleContainer.reserve(std::distance(row.attributes().begin(), row.attributes().end()));

    bool b = false;
    for (const auto& attr : row.attributes()) {
        if (attr.name() == std::string("Id") && attr.value() == std::string("336405")) {
            b = true;
        }
        // TODO: check if the second condition is necessary or not
        if (attr.empty() || attr.value() == nullptr) {
            yyjson_mut_obj_add_null(*jw, obj, attr.name());
            continue;
        }

        switch (types.at(attr.name()).type) {
        case Schema::LONG:
            yyjson_mut_obj_add_int(*jw, obj, attr.name(), attr.as_llong());
            break;
        case Schema::DOUBLE:
            yyjson_mut_obj_add_real(*jw, obj, attr.name(), attr.as_double());
            break;
        case Schema::BOOL:
            yyjson_mut_obj_add_bool(*jw, obj, attr.name(), attr.as_bool());
            break;
        case Schema::STRING:
        case Schema::DATE:
            stringLifecycleContainer.push_back(
                StringSanitiser::cleanString(attr.as_string())
            );
            if (b) {
                std::cout << "hi" << std::endl;
                b = false;
            }
            yyjson_mut_obj_add_str(
                *jw,
                obj, 
                attr.name(),
                stringLifecycleContainer.back().c_str()
            );
            break;
        [[unlikely]] default:
            throw std::runtime_error("Invalid type for field " + std::string(attr.name()));
        }
    }

    auto json = jw.write();
    if (json.error()) {
        std::stringstream ss;
        row.print(ss);

        spdlog::error("JSON write error: {}. attr value: {}", json.err.msg, ss.str());
        for (auto& string : stringLifecycleContainer) {
            spdlog::debug("String: {}", string);
            std::cerr << "Bytes: ";
            for (int c : string) { // NOLINT 
                std::cerr << std::hex << c << " ";
            }
            std::cerr << std::endl;
        }
        throw std::runtime_error("Failed to construct JSON string");
    }
    if (this->started) {
        this->writer->write(",\n");
    } else {
        this->started = true;
    }
    this->writer->write(std::string(json.str, json.len));
}

}
