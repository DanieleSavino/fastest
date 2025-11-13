#include "alloca.h"

#define FASTEST_JOIN2(a, b) a##b
#define FASTEST_JOIN(a, b) FASTEST_JOIN2(a, b)

#define FASTEST_CUSTOMTEST(name, test, callback)                               \
  do {                                                                         \
    FASTEST_TestOutput out = {0};                                              \
    out.test_name = name;                                                      \
    struct timespec start, end;                                                \
    timespec_get(&start, TIME_UTC);                                            \
                                                                               \
    test(&out);                                                                \
                                                                               \
    timespec_get(&end, TIME_UTC);                                              \
    out.time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +      \
                  (end.tv_nsec - start.tv_nsec);                               \
                                                                               \
    void (*cb)(FASTEST_TestOutput *) = (callback);                             \
    if (cb != NULL)                                                            \
      cb(&out);                                                                 \
                                                                               \
    FASTEST_Print_result(&out);                                                \
  } while (0)

#define FASTEST_CUSTOMTEST_INLINE_HELPER(name, callback, BODY, id)             \
  static void FASTEST_JOIN(FASTEST_Inline_, id)(void)                          \
      __attribute__((constructor));                                            \
  static void FASTEST_JOIN(FASTEST_Inline_, id)(void) {                        \
    FASTEST_TestOutput *out = alloca(sizeof(FASTEST_TestOutput));              \
    *out = (FASTEST_TestOutput){0};                                            \
    out->test_name = name;                                                     \
    struct timespec start, end;                                                \
    timespec_get(&start, TIME_UTC);                                            \
    ({ BODY; });                                                               \
    timespec_get(&end, TIME_UTC);                                              \
    out->time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +     \
                   (end.tv_nsec - start.tv_nsec);                              \
                                                                               \
    void (*cb)(FASTEST_TestOutput *) = (callback);                             \
    if (cb != NULL)                                                            \
      cb(out);                                                                 \
                                                                               \
    FASTEST_Print_result(out);                                                 \
  }

#define FASTEST_CUSTOMTEST_INLINE(name, callback, BODY)                        \
  FASTEST_CUSTOMTEST_INLINE_HELPER(name, callback, BODY, __COUNTER__)

#define FASTEST_CUSTOMTEST_DINLINE_HELPER(name, CALLBACK_BODY, BODY, id)       \
  static void FASTEST_JOIN(FASTEST_Dinline_, id)(void)                         \
      __attribute__((constructor));                                            \
  static void FASTEST_JOIN(FASTEST_Dinline_, id)(void) {                       \
    FASTEST_TestOutput *out = alloca(sizeof(FASTEST_TestOutput));              \
    out->test_name = name;                                                     \
    struct timespec start, end;                                                \
    timespec_get(&start, TIME_UTC);                                            \
    ({ BODY; });                                                               \
    timespec_get(&end, TIME_UTC);                                              \
    out->time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +     \
                   (end.tv_nsec - start.tv_nsec);                              \
    ({ CALLBACK_BODY; });                                                      \
    FASTEST_Print_result(out);                                                 \
  }

#define FASTEST_CUSTOMTEST_DINLINE(name, CALLBACK_BODY, BODY)                  \
  FASTEST_CUSTOMTEST_DINLINE_HELPER(name, CALLBACK_BODY, BODY, __COUNTER__)
