set (BUILD_TESTING OFF CACHE STRING "" FORCE)

include(FetchContent)

FetchContent_Declare(
    xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz
    GIT_TAG v5.8.1
    PATCH_COMMAND git reset --hard && git apply ${CMAKE_CURRENT_SOURCE_DIR}/3p-patches/xz.patch
    EXCLUDE_FROM_ALL
)


FetchContent_MakeAvailable(xz)
set(LIBLZMA_INCLUDE_DIRS ${xz_SOURCE_DIR}/src/liblzma/api/)
set (LIBLZMA_LIBRARY liblzma___external)

set (LIBLZMA_FOUND ON)
set (LIBLZMA_LIBRARIES ${LIBLZMA_LIBRARY})

set (HAVE_LZMA_STREAM_ENCODER_MT 1 CACHE STRING "" FORCE)
