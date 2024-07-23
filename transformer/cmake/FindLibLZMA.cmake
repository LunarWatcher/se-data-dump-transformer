set (BUILD_TESTING OFF CACHE STRING "" FORCE)
set (BUILD_SHARED_LIBS ON)

include(FetchContent)

FetchContent_Declare(
    xz
    GIT_REPOSITORY https://github.com/tukaani-project/xz
    GIT_TAG v5.6.2
)
FetchContent_MakeAvailable(xz)
set(LIBLZMA_INCLUDE_DIRS ${xz_SOURCE_DIR}/src/liblzma/api/)
set (LIBLZMA_LIBRARY liblzma)

set (LIBLZMA_FOUND ON)
set (LIBLZMA_LIBRARIES liblzma)
