cmake_minimum_required(VERSION 4.3)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
        is_top_level)

if(is_top_level)
    enable_testing()
    find_package(ulid_fastgen REQUIRED)
endif()

add_subdirectory(generic)
#add_subdirectory(tsan)
