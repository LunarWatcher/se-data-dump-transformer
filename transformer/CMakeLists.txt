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

list (APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set (ENABLE_TEST OFF CACHE STRING "" FORCE)

set(SQLITECPP_RUN_CPPLINT OFF CACHE STRING "" FORCE)

#set (CMAKE_FIND_DEBUG_MODE TRUE)
# For Some Reason, I have to run this here in spite of not directly
# consuming this here. 
# If not, the xz configuration fails because some fucking library 
# introduces -Werror into the global CMAKE_C_FLAGS (possibly just
# CMAKE_C_FLAGS_DEBUG, but whatever)
find_package(LibLZMA REQUIRED)
#set (CMAKE_FIND_DEBUG_MODE FALSE)

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

FetchContent_MakeAvailable(sqlitecpp)
FetchContent_MakeAvailable(yyjson)
FetchContent_MakeAvailable(cli11)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(stc)
FetchContent_MakeAvailable(pugixml)

FetchContent_GetProperties(libarchive)
if(NOT libarchive_POPULATED)
  FetchContent_Populate(libarchive)

  add_subdirectory(${libarchive_SOURCE_DIR} ${libarchive_BINARY_DIR})

endif()

if (NOT WIN32)
    if (SEDD_SANITISE)
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
    endif()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
else()
    if (SEDD_SANITISE)
        set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /fsanitize=address")
    endif()
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O3")

    add_definitions(-DNOMINMAX=1)
endif()

add_executable(sedd-transformer
    src/Main.cpp

)
add_library(sedd-src STATIC
    src/data/ArchiveParser.cpp
    src/data/ArchiveWriter.cpp

    src/data/transformers/JSONTransformer.cpp
    src/data/transformers/SQLiteTransformer.cpp

    src/data/util/ArchiveCache.cpp
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
