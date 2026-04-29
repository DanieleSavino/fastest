#include "fastest/logging.h" // ANSI colors
#include "fastest/tests.h"
#include <stdint.h>

// Helper to convert exit_status and flags to string
static inline const char *FASTEST_ErrorToString(uint64_t exit_status,
                                                uint64_t flags) {
    if (exit_status & FASTEST_SUCCESS)
        return "SUCCESS";

    if (exit_status & FASTEST_ERROR_ASSERT) {
        if (flags & FASTEST_ASSERT_EQ)
            return "ASSERT_EQ failed";
        if (flags & FASTEST_ASSERT_NEQ)
            return "ASSERT_NEQ failed";
        if (flags & FASTEST_ASSERT_GT)
            return "ASSERT_GT failed";
        if (flags & FASTEST_ASSERT_GE)
            return "ASSERT_GE failed";
        if (flags & FASTEST_ASSERT_LT)
            return "ASSERT_LT failed";
        if (flags & FASTEST_ASSERT_LE)
            return "ASSERT_LE failed";
        return "ASSERT failed";
    }

    if (exit_status & FASTEST_ERROR_UNEXPECTED)
        return "Unexpected result";
    if (exit_status & FASTEST_ERROR_EXCEPTION)
        return "Exception occurred";
    if (exit_status & FASTEST_ERROR_MEMORY)
        return "Memory error";
    if (exit_status & FASTEST_ERROR_TIMEOUT)
        return "Timeout";
    if (exit_status & FASTEST_ERROR_RESOURCE)
        return "Resource error";
    if (exit_status & FASTEST_ERROR_MPI)
        return "MPI error";
    if (exit_status & FASTEST_ERROR_OMP)
        return "OpenMP error";
    if (exit_status & FASTEST_ERROR_CUDA)
        return "CUDA error";
    if (exit_status & FASTEST_ERROR_INTERNAL)
        return "Internal framework error";
    if (exit_status & FASTEST_ERROR_UNKNOWN)
        return "Unknown error";
    if(exit_status & FASTEST_ERROR_COLLISION)
        return "Collision in test names";

    return "Unknown status";
}

void FASTEST_PrintError(uint64_t exit_status) {
    ERROR_PRINTF("%s", FASTEST_ErrorToString(exit_status, 0xFFFFFFFF));
}

void FASTEST_Print_result(const FASTEST_TestOutput *out) {
    if (!out || !(out->exit_status & FASTEST_DEFAULT_LOG))
        return;
    const char *errstr =
        FASTEST_ErrorToString(out->exit_status, out->test_flags);

    // Print success / error / warning / log based on flags
    if (out->exit_status & FASTEST_SUCCESS) {
        SUCCESS_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET, out->test_name);
    } else {
        // Check which failure mode to use
        if (out->test_flags & FASTEST_FAIL_ERROR) {
            ERROR_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": %s", out->test_name,
                         errstr);
        } else if (out->test_flags & FASTEST_FAIL_WARNING) {
            WARNING_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": %s",
                           out->test_name, errstr);
        } else if (out->test_flags & FASTEST_FAIL_LOG) {
            LOG_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": %s", out->test_name,
                       errstr);
        }
    }

    // Time reporting
    if (out->test_flags & (FASTEST_TIME_S | FASTEST_TIME_MS | FASTEST_TIME_US |
                           FASTEST_TIME_NS)) {
        if (out->test_flags & FASTEST_TIME_S) {
            LOG_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": [time: %.6fs]",
                       out->test_name, (double)out->time_ns / 1e9);
        } else if (out->test_flags & FASTEST_TIME_MS) {
            LOG_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": [time: %.3fms]",
                       out->test_name, (double)out->time_ns / 1e6);
        } else if (out->test_flags & FASTEST_TIME_US) {
            LOG_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": [time: %.3fus]",
                       out->test_name, (double)out->time_ns / 1e3);
        } else if (out->test_flags & FASTEST_TIME_NS) {
            LOG_PRINTF(FASTEST_BOLD "%s" FASTEST_RESET ": [time: %luns]",
                       out->test_name, out->time_ns);
        }
    }

    // Memory tracking
    if (out->test_flags & FASTEST_MEM_TRACK) {
        if (out->allocation != out->deallocation) {
            ERROR_PRINTF(FASTEST_BOLD
                         "%s" FASTEST_RESET
                         "Memory leaks found: [alloc=%lu, dealloc=%lu]",
                         out->test_name, out->allocation, out->deallocation);
        } else {
            SUCCESS_PRINTF(FASTEST_BOLD
                           "%s" FASTEST_RESET
                           "No memory leaks found: [alloc=%lu, dealloc=%lu]",
                           out->test_name, out->allocation, out->deallocation);
        }
    }
}
