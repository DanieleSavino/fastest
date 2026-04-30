#include "fastest/logging.h" // ANSI colors
#include "fastest/tests.h"
#include <stdint.h>

void FASTEST_print_result(const FASTEST_TestOutput_t *out) {
    if (!out || !(out->exit_status & FASTEST_DEFAULT_LOG))
        return;
    const char *errstr =
        FASTEST_strexit(out->exit_status, out->test_flags);

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
