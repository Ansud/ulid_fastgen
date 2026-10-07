# A bit weird way to check does the -mcx16 flag need to be added
function(__execute_compilation_cx16)
    include(CheckCSourceCompiles)

    set(cx16_probe_code [[
        #include <stdatomic.h>

        #if defined(__GNUC__) && !defined(__clang__)
            #ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16
                #error "128-bit CAS is not lock-free on this target in GCC"
            #endif
        #else
            typedef struct {
                unsigned long long a, b;
            } _ulid_probe_t;

            _Static_assert(
                __atomic_always_lock_free(sizeof(_ulid_probe_t), 0),
               "128-bit atomics are not lock free on this target"
            );
        #endif

        int main(void) { return 0; }
    ]])

    check_c_source_compiles("${cx16_probe_code}" CX16_COMPILE_STATUS)

    if (CX16_COMPILE_STATUS)
        set(DISCOVERED_CX16_FLAG "" PARENT_SCOPE)
        return()
    endif ()

    # Update required flags
    set(PROPOSED_CX16_FLAG "-mcx16")
    set(CMAKE_REQUIRED_FLAGS "${PROPOSED_CX16_FLAG}")

    # No same variable, check_c_source_compiles checks that it is DEFINED and do nothing
    check_c_source_compiles("${cx16_probe_code}" CX16_COMPILE_STATUS_WITH_FLAGS)

    # Both compilation failed...
    if (CX16_COMPILE_STATUS_WITH_FLAGS)
        set(DISCOVERED_CX16_FLAG ${PROPOSED_CX16_FLAG} PARENT_SCOPE)
        return()
    endif()

    message(
        FATAL_ERROR
        "ulid_fastgen requires lock-free 128-bit atomics, which this target lacks"
    )
endfunction()

function(__check_cx16_flag)
    if (DEFINED CACHE{ULID_FASTGEN_CX16_FLAG})
        return()
    endif ()

    __execute_compilation_cx16()

    set(ULID_FASTGEN_CX16_FLAG "${DISCOVERED_CX16_FLAG}" CACHE INTERNAL "Flag to enable CMPXCHG16 on x86 arch")
endfunction()


macro(check_cmpxchg16_flag_needed)
    __check_cx16_flag()
endmacro()
