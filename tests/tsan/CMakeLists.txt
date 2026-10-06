set(sources
        ${common_headers}
        test_threads.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

find_package(Threads REQUIRED)

add_executable(ulid_fastgen_tsan_tests ${sources})

target_compile_features(ulid_fastgen_tsan_tests PRIVATE cxx_std_17)
target_link_libraries(ulid_fastgen_tsan_tests
        PRIVATE
        ulid_fastgen::ulid_fastgen
        GTest::gtest_main
        Threads::Threads
)

include(GoogleTest)
gtest_discover_tests(ulid_fastgen_tsan_tests
        PROPERTIES
            ENVIRONMENT "TSAN_OPTIONS=halt_on_error=1:exitcode=66"
            FAIL_REGULAR_EXPRESSION "ThreadSanitizer: data race"
            TIMEOUT 300
)

