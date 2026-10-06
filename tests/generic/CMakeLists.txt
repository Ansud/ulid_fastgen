set(sources
        ${common_headers}
        test_generic.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

add_executable(ulid_fastgen_generic_tests ${sources})

target_compile_features(ulid_fastgen_generic_tests PRIVATE cxx_std_17)
target_link_libraries(ulid_fastgen_generic_tests
        PRIVATE
        ulid_fastgen::ulid_fastgen
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ulid_fastgen_generic_tests)

