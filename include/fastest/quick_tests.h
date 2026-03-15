#pragma once

#include <stdint.h>
#include <time.h>

#include "fastest/logging.h"
#include "fastest/tests.h"

#define FASTEST_QUICKTEST_DELEGATE(name, func, expected, flags, ...)                    \
  ({                                                                           \
    FASTEST_TestOutput _out = {0};                                             \
    _out.test_name  = name;                                                    \
    _out.test_flags = flags;                                                   \
                                                                               \
    struct timespec start, end;                                                \
    if (flags & (FASTEST_TIME_S | FASTEST_TIME_MS |                            \
                 FASTEST_TIME_US | FASTEST_TIME_NS))                           \
      timespec_get(&start, TIME_UTC);                                          \
                                                                               \
    __auto_type _ret = func(__VA_ARGS__); /* Invoke test target */             \
                                                                               \
    if (flags & (FASTEST_TIME_S | FASTEST_TIME_MS |                            \
                 FASTEST_TIME_US | FASTEST_TIME_NS)) {                         \
      timespec_get(&end, TIME_UTC);                                            \
      _out.time_ns = (uint64_t)(end.tv_sec  - start.tv_sec)  * 1000000000ULL + \
                     (end.tv_nsec - start.tv_nsec);                            \
    }                                                                          \
                                                                               \
    /* Assertions */                                                           \
    switch (FASTEST_Get_assert_mode(flags)) {                                  \
      case FASTEST_ASSERT_EQ:  (_ret == expected) ?                            \
                               (_out.exit_status |= FASTEST_SUCCESS) :         \
                               (_out.exit_status |= FASTEST_ERROR_ASSERT);     \
                               break;                                          \
      case FASTEST_ASSERT_NEQ: (_ret != expected) ?                            \
                               (_out.exit_status |= FASTEST_SUCCESS) :         \
                               (_out.exit_status |= FASTEST_ERROR_ASSERT);     \
                               break;                                          \
      case FASTEST_ASSERT_GT:  (_ret >  expected) ?                            \
                               (_out.exit_status |= FASTEST_SUCCESS) :         \
                               (_out.exit_status |= FASTEST_ERROR_ASSERT);     \
                               break;                                          \
      case FASTEST_ASSERT_GE:  (_ret >= expected) ?                            \
                               (_out.exit_status |= FASTEST_SUCCESS) :         \
                               (_out.exit_status |= FASTEST_ERROR_ASSERT);     \
                               break;                                          \
      case FASTEST_ASSERT_LT:  (_ret <  expected) ?                            \
                               (_out.exit_status |= FASTEST_SUCCESS) :         \
                               (_out.exit_status |= FASTEST_ERROR_ASSERT);     \
                               break;                                          \
      case FASTEST_ASSERT_LE:  (_ret <= expected) ?                            \
                               (_out.exit_status |= FASTEST_SUCCESS) :         \
                               (_out.exit_status |= FASTEST_ERROR_ASSERT);     \
                               break;                                          \
    }                                                                          \
                                                                               \
    _out.exit_status |= FASTEST_DEFAULT_LOG;                                   \
    FASTEST_Print_result(&_out);                                               \
  })

#define FASTEST_QUICKTEST(name, expr, flags)                                      \
({                                                                                \
    FASTEST_TestOutput _out = {0};                                                \
    _out.test_name  = name;                                                       \
    _out.test_flags = flags;                                                      \
                                                                                  \
    __auto_type result = 0;                                                       \
    struct timespec start, end;                                                   \
                                                                                  \
    /* Timed evaluation of raw expression */                                      \
    if (flags & (FASTEST_TIME_S|FASTEST_TIME_MS|FASTEST_TIME_US|FASTEST_TIME_NS)) \
        timespec_get(&start, TIME_UTC);                                           \
                                                                                  \
    result = (expr);                                                              \
                                                                                  \
    if (flags & (FASTEST_TIME_S|FASTEST_TIME_MS|FASTEST_TIME_US|FASTEST_TIME_NS)) {\
        timespec_get(&end, TIME_UTC);                                             \
        _out.time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +    \
                       (end.tv_nsec - start.tv_nsec);                             \
    }                                                                             \
                                                                                  \
    /* Assertions based only on boolean truth */                                  \
    switch (FASTEST_Get_assert_mode(flags)) {                                     \
        case FASTEST_ASSERT_EQ:                                                   \
        case FASTEST_ASSERT_NEQ:                                                  \
        case FASTEST_ASSERT_GT:                                                   \
        case FASTEST_ASSERT_GE:                                                   \
        case FASTEST_ASSERT_LT:                                                   \
        case FASTEST_ASSERT_LE:                                                   \
            if (!result)                                                          \
                _out.exit_status |= FASTEST_ERROR_ASSERT;                         \
            else                                                                  \
                _out.exit_status |= FASTEST_SUCCESS;                              \
    }                                                                             \
                                                                                  \
    _out.exit_status |= FASTEST_DEFAULT_LOG;                                      \
    FASTEST_Print_result(&_out);                                                  \
})
