cmake_minimum_required(VERSION 4.3)
project(ulid_fastgen_generic_tests
        LANGUAGES CXX
)


set(sources
        test_generic.cpp)

set(CMAKE_CXX_STANDARD 17)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

add_executable(ulid_fastgen_generic_tests)
target_sources(ulid_fastgen_generic_tests PRIVATE ${sources})
target_link_libraries(ulid_fastgen_generic_tests
        PRIVATE
        ulid_fastgen
)

