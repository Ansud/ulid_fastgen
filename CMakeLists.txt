cmake_minimum_required(VERSION 4.3)

include("cmake/utils.cmake")

project(ulid_fastgen
        VERSION 0.1.0
        DESCRIPTION "Fast monotonic ULID generator"
        LANGUAGES C)

add_library(ulid_fastgen
        include/ulid_generator.hpp)
add_library(ulid_fastgen::ulid_fastgen ALIAS ulid_fastgen)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
        is_top_level)

option(FULID_BUILD_TESTS "Build ulid_fastgen tests" OFF)
option(ENABLE_ASAN "Enable address/ub sanitizer" OFF)
option(ENABLE_TSAN "Enable thread sanitizer" OFF)

set(C_STANDARD 17)
set(C_STANDARD_REQUIRED ON)

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

set(CMAKE_C_FLAGS "-O2")
set(CMAKE_CXX_FLAGS "-O2")

if (ENABLE_ASAN)
    set(CMAKE_C_FLAGS "-O1 -fsanitize=address,undefined -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "-O1 -fsanitize=address,undefined -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address,undefined")
elseif (ENABLE_TSAN)
    set(CMAKE_C_FLAGS "-O1 -fsanitize=thread -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "-O1 -fsanitize=thread -fno-omit-frame-pointer -g")
    set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=thread")
else ()
    set(CMAKE_C_FLAGS "-O2")
    set(CMAKE_CXX_FLAGS "-O2")
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

include(CMakePackageConfigHelpers)

target_sources(ulid_fastgen PRIVATE ${sources})
target_compile_definitions(ulid_fastgen
        PUBLIC
)

target_include_directories(ulid_fastgen
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_target_properties(ulid_fastgen PROPERTIES
        PUBLIC_HEADER "${public_headers}"
        VERSION ${PROJECT_VERSION})


if (FULID_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()