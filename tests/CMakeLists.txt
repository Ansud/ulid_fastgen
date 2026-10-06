string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
        is_top_level)

if (is_top_level)
    cmake_minimum_required(VERSION 3.23)
    project(ulid_fastgen_tests LANGUAGES C CXX)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Generate compile_commands.json")

    enable_testing()
    find_package(ulid_fastgen REQUIRED)
endif ()

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(include/)

if (FULID_BUILD_TESTS)
    add_subdirectory(generic)
elseif (FULID_BUILD_TESTS_TSAN)
    add_subdirectory(tsan)
endif ()
