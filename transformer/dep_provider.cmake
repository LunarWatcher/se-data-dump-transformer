cmake_minimum_required(VERSION 3.24)

set (ROOT_DIR ${CMAKE_SOURCE_DIR})
set (DEP_OVERRIDES ${ROOT_DIR}/cmake/)

macro(sedd_provide_dependency method package_name)

    if (NOT ("${DEP_OVERRIDES}" IN_LIST CMAKE_MODULE_PATH))
        list(APPEND CMAKE_MODULE_PATH "${DEP_OVERRIDES}")
    endif()

    message(STATUS "INSTALLING DEP: ${package_name} with lookup path ${CMAKE_MODULE_PATH}")
    find_package(${package_name} BYPASS_PROVIDER)
endmacro()

cmake_language(
SET_DEPENDENCY_PROVIDER
    sedd_provide_dependency
SUPPORTED_METHODS 
    FIND_PACKAGE
)
