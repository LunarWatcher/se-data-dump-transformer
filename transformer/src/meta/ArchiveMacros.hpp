#pragma once

#define SEDDARCHIVE_CHECK_ERROR(a, expr) { \
        int r = expr; \
        if (r != ARCHIVE_OK) { \
            std::cerr << archive_error_string(a) << std::endl; \
            throw std::runtime_error("Archive error"); \
        } \
    }
