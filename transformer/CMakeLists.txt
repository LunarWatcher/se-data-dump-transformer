cmake_minimum_required(VERSION 3.10)
project(sedd-transformer)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

set (ENABLE_TEST OFF CACHE STRING "" FORCE)

if (UNIX)
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
endif()

include(FetchContent)
FetchContent_Declare(
    archive
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

FetchContent_MakeAvailable(yyjson)
FetchContent_MakeAvailable(cli11)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(stc)
FetchContent_MakeAvailable(archive)
FetchContent_MakeAvailable(pugixml)

add_executable(sedd-transformer
    src/Main.cpp

)
add_library(sedd-src STATIC
    src/data/ArchiveParser.cpp
    src/data/ArchiveWriter.cpp

    src/data/transformers/JSONTransformer.cpp
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
)
target_link_libraries(
    sedd-transformer 
PUBLIC 
    sedd-src
)

add_subdirectory(tests EXCLUDE_FROM_ALL)
add_custom_target(test
    COMMAND tests
    DEPENDS tests
    COMMENT "Test the data dump transformer ")
