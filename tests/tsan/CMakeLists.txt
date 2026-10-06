#
# Thread sanitizer tests.
#
# The generator is lock free (a CAS loop over an _Atomic ulid_t), so the whole
# point of these tests is to hammer it from several threads at once under TSan.
# That requires the library itself to be instrumented, which is why the target
# links ulid_fastgen_tsan (built by the root CMakeLists) instead of the regular
# library. If the whole build is already instrumented via ENABLE_TSAN, the
# regular library is fine.
#
if (TARGET ulid_fastgen_tsan)
    set(tsan_library ulid_fastgen_tsan)
elseif (ENABLE_TSAN)
    set(tsan_library ulid_fastgen::ulid_fastgen)
else ()
    message(STATUS "No thread sanitizer instrumented ulid_fastgen available, skipping the tsan tests")
    return()
endif ()

set(sources
        test_threads.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

find_package(Threads REQUIRED)

add_executable(ulid_fastgen_tsan_tests ${sources})

target_compile_features(ulid_fastgen_tsan_tests PRIVATE cxx_std_17)
target_link_libraries(ulid_fastgen_tsan_tests
        PRIVATE
        ${tsan_library}
        Threads::Threads
)

add_test(NAME ulid_fastgen_tsan
        COMMAND ulid_fastgen_tsan_tests)

# TSan exits with 66 on a report unless told otherwise; halt_on_error makes the
# very first data race fail the test instead of letting it run to completion.
set_tests_properties(ulid_fastgen_tsan PROPERTIES
        ENVIRONMENT "TSAN_OPTIONS=halt_on_error=1:exitcode=66"
        FAIL_REGULAR_EXPRESSION "ThreadSanitizer: data race"
        TIMEOUT 300)
