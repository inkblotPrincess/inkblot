cmake_minimum_required(VERSION 4.1)
project(
    inkblot
    VERSION 0.1.0
    DESCRIPTION "C++23 Game"
    LANGUAGES CXX
)

option(INKBLOT_BUILD_APP   "Build application (exe)"  ${PROJECT_IS_TOP_LEVEL})
option(INKBLOT_BUILD_TESTS "Build tests"              ${PROJECT_IS_TOP_LEVEL})
option(INKBLOT_USE_VULKAN  "Use Vulkan for rendering" ${PROJECT_IS_TOP_LEVEL})

if(PROJECT_IS_TOP_LEVEL)
    if(INKBLOT_BUILD_TESTS)
        include(CTest)
    endif()

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

    foreach(CONFIG_TYPE ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER "${CONFIG_TYPE}" CONFIG_TYPE_UPPER)

        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/bin")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/bin")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE_UPPER} "${CMAKE_BINARY_DIR}/lib")
    endforeach()
endif()

include(cmake/sanitizers.cmake)
include(cmake/warnings.cmake)
include(cmake/project_options.cmake)

include(cmake/dependencies.cmake)

add_subdirectory(code)