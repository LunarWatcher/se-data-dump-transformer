set (BUILD_TESTING OFF CACHE STRING "" FORCE)

include(FetchContent)

FetchContent_Declare(
    xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz
    GIT_TAG v5.6.2
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(xz)
set(LIBLZMA_INCLUDE_DIRS ${xz_SOURCE_DIR}/src/liblzma/api/)
set (LIBLZMA_LIBRARY liblzma)

set (LIBLZMA_FOUND ON)
set (LIBLZMA_LIBRARIES ${LIBLZMA_LIBRARY})

set (HAVE_LZMA_STREAM_ENCODER_MT 1 CACHE STRING "" FORCE)
