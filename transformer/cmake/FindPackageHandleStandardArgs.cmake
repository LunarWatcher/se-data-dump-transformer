# I hate this so fucking much
function(FIND_PACKAGE_HANDLE_STANDARD_ARGS _NAME _FIRST_ARG)
    if ("${_NAME}" STREQUAL "ZSTD")
        message(INFO "Hijacking FIND_PACKAGE_HANDLE_STANDARD_ARGS for ZSTD")
        find_package(${_NAME} REQUIRED)
    else()
        set (OLD_CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
        set (CMAKE_MODULE_PATH "")

        include(FindPackageHandleStandardArgs)

        find_package_handle_standard_args(${_NAME} ${_FIRST_ARG})

        set (CMAKE_MODULE_PATH "${OLD_CMAKE_MODULE_PATH}")
    endif()

endfunction()