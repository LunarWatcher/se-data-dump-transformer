#pragma once

#include <stdexcept>
#include <yyjson.h>

namespace sedd {

class YYJsonStr {
public:
    const char* str;
    yyjson_write_err err;

    YYJsonStr(yyjson_mut_doc* doc) : str(
        yyjson_mut_write_opts(doc, 0, nullptr, nullptr, &err)
    ) {}

    YYJsonStr(YYJsonStr&& other) noexcept : str(other.str) {
        other.str = nullptr;
    }
    ~YYJsonStr() {
        if (str != nullptr) {
            free((void*) str);
        }
    }

    /**
     * Returns whether or not the write passed.
     * Semi-undefined behaviour after moving
     */
    bool error() {
        return str == nullptr;
    }
};

/**
 * Light-weight wrapper around the yyjson_mut_doc to let RAII deal with memory management
 */
class YYJsonWriter {
public:
    yyjson_mut_doc* doc;

    YYJsonWriter() : doc(yyjson_mut_doc_new(nullptr)) {
        if (doc == nullptr) {
            throw std::runtime_error("Failed to allocate yyjson doc");
        }
    }

    YYJsonWriter(YYJsonWriter&&other) noexcept : doc(other.doc) {
        other.doc = nullptr;
    }

    ~YYJsonWriter() {
        if (doc != nullptr) {
            yyjson_mut_doc_free(doc);
        }
    }

    YYJsonStr write() {
        return {doc};
    }

    decltype(doc) operator*() {
        return doc;
    }
};

}
