#pragma once

#include <stdint.h>
#include <time.h>

#include "logging.h"
#include "tests.h"

#define FASTEST_QUICKTEST(name, func, expected, flags, ...)                    \
  ({                                                                           \
    FASTEST_TestOutput _out = {0};                                             \
    _out.test_name = name;                                                     \
    _out.test_flags = flags;                                                   \
                                                                               \
    struct timespec start, end;                                                \
    if (flags & (FASTEST_TIME_S | FASTEST_TIME_MS | FASTEST_TIME_US |          \
                 FASTEST_TIME_NS)) {                                           \
      timespec_get(&start, TIME_UTC);                                          \
    }                                                                          \
                                                                               \
    /* Call the function */                                                    \
    __auto_type _ret = func(__VA_ARGS__);                                      \
                                                                               \
    if (flags & (FASTEST_TIME_S | FASTEST_TIME_MS | FASTEST_TIME_US |          \
                 FASTEST_TIME_NS)) {                                           \
      timespec_get(&end, TIME_UTC);                                            \
      _out.time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +   \
                     (end.tv_nsec - start.tv_nsec);                            \
    }                                                                          \
                                                                               \
    /* Handle assertion */                                                     \
    switch (FASTEST_Get_assert_mode(flags)) {                                  \
    case FASTEST_ASSERT_EQ:                                                    \
      if (_ret == expected)                                                    \
        _out.exit_status |= FASTEST_SUCCESS;                                   \
      else                                                                     \
        _out.exit_status |= FASTEST_ERROR_ASSERT;                              \
      break;                                                                   \
    case FASTEST_ASSERT_NEQ:                                                   \
      if (_ret != expected)                                                    \
        _out.exit_status |= FASTEST_SUCCESS;                                   \
      else                                                                     \
        _out.exit_status |= FASTEST_ERROR_ASSERT;                              \
      break;                                                                   \
    case FASTEST_ASSERT_GT:                                                    \
      if (_ret > expected)                                                     \
        _out.exit_status |= FASTEST_SUCCESS;                                   \
      else                                                                     \
        _out.exit_status |= FASTEST_ERROR_ASSERT;                              \
      break;                                                                   \
    case FASTEST_ASSERT_GE:                                                    \
      if (_ret >= expected)                                                    \
        _out.exit_status |= FASTEST_SUCCESS;                                   \
      else                                                                     \
        _out.exit_status |= FASTEST_ERROR_ASSERT;                              \
      break;                                                                   \
    case FASTEST_ASSERT_LT:                                                    \
      if (_ret < expected)                                                     \
        _out.exit_status |= FASTEST_SUCCESS;                                   \
      else                                                                     \
        _out.exit_status |= FASTEST_ERROR_ASSERT;                              \
      break;                                                                   \
    case FASTEST_ASSERT_LE:                                                    \
      if (_ret <= expected)                                                    \
        _out.exit_status |= FASTEST_SUCCESS;                                   \
      else                                                                     \
        _out.exit_status |= FASTEST_ERROR_ASSERT;                              \
      break;                                                                   \
    }                                                                          \
                                                                               \
    _out.exit_status |= FASTEST_DEFAULT_LOG;                                   \
    FASTEST_Print_result(&_out);                                               \
  })
