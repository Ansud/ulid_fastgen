cmake_minimum_required(VERSION 3.23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json")

project(ulid_fastgen
        VERSION 0.1.0
        DESCRIPTION "Fast monotonic ULID generator"
        LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(utils)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
        is_top_level)

option(FULID_BUILD_TESTS "Build ulid_fastgen tests (with ASan/UBSan")
option(FULID_BUILD_TESTS_TSAN "Build the ulid_fastgen TSan tests")
option(FULID_INSTALL "Generate ulid_fastgen install rules")

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Default is Debug, set CMAKE_BUILD_TYPE=Release to get properly optimized release version
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)
endif ()

set(ULID_FASTGEN_ASAN_FLAGS -fsanitize=address,undefined -fno-omit-frame-pointer -g)
set(ULID_FASTGEN_TSAN_FLAGS -fsanitize=thread -fno-omit-frame-pointer -g)

# Tests should have more flags
if (FULID_BUILD_TESTS AND FULID_BUILD_TESTS_TSAN)
    message(FATAL_ERROR "ASAN and TSAN can not be used together")
elseif (FULID_BUILD_TESTS)
    add_compile_options(${ULID_FASTGEN_ASAN_FLAGS})
    add_link_options(${ULID_FASTGEN_ASAN_FLAGS})
elseif (FULID_BUILD_TESTS_TSAN)
    add_compile_options(${ULID_FASTGEN_TSAN_FLAGS})
    add_link_options(${ULID_FASTGEN_TSAN_FLAGS})
endif ()

set(public_headers
        include/ulid_generator.h
)

set(sources
        ${public_headers}
        src/random.h
        src/random.c
        src/ulid_generator.c
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

add_library(ulid_fastgen ${sources})
add_library(ulid_fastgen::ulid_fastgen ALIAS ulid_fastgen)

# The lib must use C17 because of _Atomic and friends
target_compile_features(ulid_fastgen PUBLIC c_std_17)

target_include_directories(ulid_fastgen
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_target_properties(ulid_fastgen PROPERTIES
        PUBLIC_HEADER "${public_headers}"
        C_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME ulid_fastgen)

if (FULID_BUILD_TESTS OR FULID_BUILD_TESTS_TSAN)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (FULID_INSTALL)
    install(TARGETS ulid_fastgen
            EXPORT ulid_fastgen_targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

    set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/ulid_fastgen")

    install(EXPORT ulid_fastgen_targets
            FILE ulid_fastgen-targets.cmake
            NAMESPACE ulid_fastgen::
            DESTINATION "${config_install_dir}")

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/ulid_fastgen-config-version.cmake"
            COMPATIBILITY SameMajorVersion)

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ulid_fastgen-config.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/ulid_fastgen-config.cmake"
            INSTALL_DESTINATION "${config_install_dir}")

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/ulid_fastgen-config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/ulid_fastgen-config-version.cmake"
            DESTINATION "${config_install_dir}")
endif ()

if (is_top_level AND CMAKE_EXPORT_COMPILE_COMMANDS)
    add_custom_target(ulid_fastgen_compile_commands ALL
            COMMAND "${CMAKE_COMMAND}" -E create_symlink
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/compile_commands.json"
            COMMENT "Linking compile_commands.json into the source tree"
            VERBATIM)
endif ()
