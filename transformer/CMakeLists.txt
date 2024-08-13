cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/dep_provider.cmake")
project(sedd-transformer)

set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0144 NEW)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)
set (BUILD_SHARED_LIBS ON)

option (SEDD_SANITISE "Whether or not to enable -fsanitize" OFF)
option (LINT "Whether or not to enable clang-tidy checks" OFF)
option (LINT_WARNINGS_ARE_ERRORS "Whether or not to set -warnings-as-errors" OFF)

list (APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}/cmake/")
list (APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/cmake/")

set (ENABLE_TEST OFF CACHE STRING "" FORCE)

set(SQLITECPP_RUN_CPPLINT OFF CACHE STRING "" FORCE)

#set (CMAKE_FIND_DEBUG_MODE TRUE)
# For some reason, these have to be defined here, or the identical
# find_packages in libarchive fail to work.
# Probably a skill issue, but if it works...
find_package(LibLZMA REQUIRED)
find_package(ZSTD REQUIRED)
#set (CMAKE_FIND_DEBUG_MODE FALSE)

# Required to keep zstd from exploding from https://github.com/facebook/zstd/blob/fdfb2aff39dc498372d8c9e5f2330b692fea9794/lib/common/pool.c#L103
set (ENABLE_WERROR OFF CACHE STRING "" FORCE)

set (SPDLOG_FMT_EXTERNAL ON CACHE STRING "" FORCE)

include(FetchContent)

FetchContent_Declare(
    libarchive
    GIT_REPOSITORY https://github.com/libarchive/libarchive
    GIT_TAG v3.7.4
)
FetchContent_Declare(
    stc
    GIT_REPOSITORY https://github.com/LunarWatcher/stc
)
FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.14.1
)
FetchContent_Declare(pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml
    GIT_TAG v1.14
)
FetchContent_Declare(
    cli11
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2
)
FetchContent_Declare(
    yyjson
    GIT_REPOSITORY https://github.com/ibireme/yyjson
    GIT_TAG 0.10.0
)
FetchContent_Declare(
    sqlitecpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp
    GIT_TAG 3.3.1
)

FetchContent_Declare(fmt 
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2)

FetchContent_MakeAvailable(
    fmt
    sqlitecpp
    yyjson
    cli11
    spdlog
    stc
    pugixml
    libarchive
)



if (NOT WIN32)
    if (SEDD_SANITISE)
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
    endif()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
else()
    if (SEDD_SANITISE)
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
    endif()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

    add_definitions(-DNOMINMAX=1)
endif()

add_executable(sedd-transformer
    src/Main.cpp

)
add_library(sedd-src STATIC
    src/data/ArchiveParser.cpp
    src/data/ArchiveWriter.cpp
    src/data/Transformer.cpp

    src/data/transformers/JSONTransformer.cpp
    src/data/transformers/SQLiteTransformer.cpp

    src/data/util/ArchiveCache.cpp

    src/util/InputPreprocessor.cpp
)
target_compile_options(
    sedd-src
PRIVATE
    $<$<AND:$<CONFIG:Debug>,$<NOT:$<PLATFORM_ID:Windows>>>:-Wall -Wextra>
)

target_include_directories(sedd-src PUBLIC src)
target_link_libraries(
    sedd-src
PUBLIC
    archive 
    stc 
    spdlog::spdlog
    pugixml
    CLI11::CLI11
    yyjson
    SQLiteCpp
)

# TODO: Figure out if poolstl is a viable replacement for this intel crap
if (UNIX)
    find_package(
        TBB
    )
    if (TBB_FOUND)
        message(STATUS "Found libtbb -- linking")
        target_link_libraries(sedd-src PUBLIC TBB::tbb)

        if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++")
        endif()
    else()
        message(WARNING "Failed to find tbb. Multithreading is not supported")
    endif()
endif()

if (LINT)
    find_program(
        CLANGTIDY
        NAMES "clang-tidy" "clang-tidy.exe"
        REQUIRED
    )

    set (CLANG_TIDY_COMMAND "${CLANGTIDY}" "--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy")
    if (LINT_WARNINGS_ARE_ERRORS)
        list (APPEND CLANG_TIDY_COMMAND  -warnings-as-errors=*)
    endif()

    if (WIN32)
        list (APPEND CLANG_TIDY_COMMAND --extra-arg=/EHsc)
    endif()

    message(STATUS "Using CLANG_TIDY_COMMAND=${CLANG_TIDY_COMMAND}")
    set_target_properties(sedd-src PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()

target_link_libraries(
    sedd-transformer 
PUBLIC 
    sedd-src
)

add_subdirectory(tests EXCLUDE_FROM_ALL)
add_custom_target(test
    COMMAND tests
    DEPENDS tests
    COMMENT "Test the data dump transformer")
