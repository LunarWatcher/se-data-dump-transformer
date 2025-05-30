set (ZSTD_BUILD_STATIC ON CACHE STRING "" FORCE)
set (ZSTD_BUILD_SHARED OFF CACHE STRING "" FORCE)
set (ZSTD_BUILD_PROGRAMS OFF CACHE STRING "" FORCE)

include(FetchContent)

FetchContent_Declare(
    zstd_ext
    GIT_REPOSITORY https://github.com/facebook/zstd
    GIT_TAG v1.5.2
    SOURCE_SUBDIR build/cmake
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(
    zstd_ext
)

set (ZSTD_FOUND ON)
set (ZSTD_INCLUDE_DIR ${zstd_ext_SOURCE_DIR}/lib)
set (ZSTD_LIBRARY libzstd_static)

# Required to make check_function_exists work
# on the bright side, no new nasty hacks like cmake/FindPackageHandleStandardArgs.cmake required to make
# that function cooperate
set (HAVE_LIBZSTD 1 CACHE STRING "Have function ZSTD_decompressStream " FORCE)
set (HAVE_ZSTD_compressStream 1 CACHE STRING "Have function ZSTD_compressStream" FORCE)
