#pragma once

#include <stdint.h>

// Quick api flags

#define FASTEST_ASSERT_EQ 0x1
#define FASTEST_ASSERT_NEQ 0x2
#define FASTEST_ASSERT_GT 0x4
#define FASTEST_ASSERT_GE 0x8
#define FASTEST_ASSERT_LT 0x10
#define FASTEST_ASSERT_LE 0x20

// Tests flags

#define FASTEST_FAIL_ERROR 0x40
#define FASTEST_FAIL_WARNING 0x80
#define FASTEST_FAIL_LOG 0x100

#define FASTEST_TIME_S 0x200
#define FASTEST_TIME_MS 0x400
#define FASTEST_TIME_US 0x800
#define FASTEST_TIME_NS 0x1000

#define FASTEST_MEM_TRACK 0x2000

// ==========================
// Fastest Status & Error Codes
// ==========================

// General statuses
#define FASTEST_SUCCESS            0x0001  // Test completed successfully
#define FASTEST_SKIPPED            0x0002  // Test skipped (e.g., not applicable on platform)
#define FASTEST_INCOMPLETE         0x0004  // Test was aborted or timed out before completion

// Assertion & logic errors
#define FASTEST_ERROR_ASSERT       0x0100  // Assertion failed
#define FASTEST_ERROR_UNEXPECTED   0x0200  // Unexpected result not matching expected output
#define FASTEST_ERROR_EXCEPTION    0x0400  // Exception, signal, or fatal error occurred (e.g., segfault)
#define FASTEST_ERROR_MEMORY       0x0800  // Memory leak or invalid free detected
#define FASTEST_ERROR_TIMEOUT      0x1000  // Test exceeded time limit

// Environment / resource errors
#define FASTEST_ERROR_RESOURCE     0x2000  // Resource allocation or I/O failure
#define FASTEST_ERROR_MPI          0x4000  // MPI communication or rank synchronization failure
#define FASTEST_ERROR_OMP          0x8000  // OpenMP-related concurrency error
#define FASTEST_ERROR_CUDA         0x10000 // CUDA device or kernel failure

// Internal framework errors
#define FASTEST_ERROR_INTERNAL     0x20000 // Internal Fastest bug or misuse
#define FASTEST_ERROR_UNKNOWN      0x40000 // Unknown failure (catch-all)
                                           //
#define FASTEST_DEFAULT_LOG 0x80000

typedef struct {
    const char *test_name;
    uint64_t test_flags;
    uint64_t exit_status;
    uint64_t allocation; 
    uint64_t deallocation;
    uint64_t time_ns;
} FASTEST_TestOutput;

static inline uint64_t FASTEST_Get_assert_mode(uint64_t flags) {
    if (flags & FASTEST_ASSERT_EQ)
        return FASTEST_ASSERT_EQ;
    if (flags & FASTEST_ASSERT_NEQ)
        return FASTEST_ASSERT_NEQ;
    if (flags & FASTEST_ASSERT_GT)
        return FASTEST_ASSERT_GT;
    if (flags & FASTEST_ASSERT_GE)
        return FASTEST_ASSERT_GE;
    if (flags & FASTEST_ASSERT_LT)
        return FASTEST_ASSERT_LT;
    if (flags & FASTEST_ASSERT_LE)
        return FASTEST_ASSERT_LE;
    return 0; // no assertion
}
