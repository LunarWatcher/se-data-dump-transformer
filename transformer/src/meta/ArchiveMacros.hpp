#pragma once

#define SEDDARCHIVE_CHECK_ERROR(a, expr) \
    if ((expr) != ARCHIVE_OK) { \
        std::cerr << "Archive error: " << archive_error_string(a) << std::endl; \
        throw std::runtime_error("Archive error: " + std::string(archive_error_string(a))); \
    } 

#define SEDDARCHIVE_CHECK_ERROR_NEG(a, expr) \
    if ((expr) < 0) { \
        std::cerr << "Archive error: " << archive_error_string(a) << std::endl; \
        throw std::runtime_error("Archive error: " + std::string(archive_error_string(a))); \
    } 
