cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.12)
if(NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(sl-5-6-osi-trace-file-writer)

set(OSMPVERSION "1.2.0" CACHE STRING "OSMP Version String")

# OSI
add_subdirectory(lib/open-simulation-interface)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/lib/open-simulation-interface)
get_directory_property(OSI_VERSION_MAJOR DIRECTORY lib/open-simulation-interface DEFINITION VERSION_MAJOR)
get_directory_property(OSI_VERSION_MINOR DIRECTORY lib/open-simulation-interface DEFINITION VERSION_MINOR)
get_directory_property(OSI_VERSION_PATCH DIRECTORY lib/open-simulation-interface DEFINITION VERSION_PATCH)
set(OSIVERSION "${OSI_VERSION_MAJOR}.${OSI_VERSION_MINOR}.${OSI_VERSION_PATCH}")

# fmi
include_directories(lib/fmi2/headers)

# asam-osi-utilities
set(USE_EXTERNAL_OSI ON)
add_subdirectory(lib/asam-osi-utilities EXCLUDE_FROM_ALL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/asam-osi-utilities/include)
set_target_properties(OSIUtilities PROPERTIES POSITION_INDEPENDENT_CODE ON)


set(FMU_INSTALL_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Target directory for generated FMU")

add_subdirectory(src/)