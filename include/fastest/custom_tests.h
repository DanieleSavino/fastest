#pragma once
#include "fastest/test_list.h"

#define FASTEST_JOIN2(a, b) a##b
#define FASTEST_JOIN(a, b)  FASTEST_JOIN2(a, b)

/* ------------------------------------------------------------------ */
/*  Internal: shared registration logic                                */
/* ------------------------------------------------------------------ */

#define FASTEST_REGISTER_(name_, func_, callback_, flags_, id)                 \
  static void FASTEST_JOIN(FASTEST_reg_, id)(void)                             \
      __attribute__((constructor));                                             \
  static void FASTEST_JOIN(FASTEST_reg_, id)(void) {                           \
    FASTEST_list_t *list = NULL;                                               \
    if (FASTEST_list_getInstance(&list) != FASTEST_SUCCESS) return;           \
    FASTEST_SchedTest entry = {                                                \
        .func      = (func_),                                                  \
        .test_name = (name_),                                                  \
        .callback  = (callback_),                                              \
        .out       = { .test_flags = (flags_) },                              \
    };                                                                         \
    FASTEST_list_push(list, &entry);                                           \
  }

/* ------------------------------------------------------------------ */
/*  FASTEST_CUSTOMTEST                                                 */
/* ------------------------------------------------------------------ */

#define FASTEST_CUSTOMTEST(name, preload_flags, test, callback)                \
  FASTEST_REGISTER_(name, test, callback, preload_flags, __COUNTER__)

/* ------------------------------------------------------------------ */
/*  FASTEST_CUSTOMTEST_INLINE                                          */
/* ------------------------------------------------------------------ */

#define FASTEST_CUSTOMTEST_INLINE_HELPER(name, callback, preload_flags, BODY, id) \
  static void FASTEST_JOIN(FASTEST_body_, id)(FASTEST_TestOutput *out) {      \
    (void)out;                                                                 \
    BODY;                                                                      \
  }                                                                            \
  FASTEST_REGISTER_(name, FASTEST_JOIN(FASTEST_body_, id), callback, preload_flags, id)

#define FASTEST_CUSTOMTEST_INLINE(name, preload_flags, callback, BODY)         \
  FASTEST_CUSTOMTEST_INLINE_HELPER(name, callback, preload_flags, BODY, __COUNTER__)

/* ------------------------------------------------------------------ */
/*  FASTEST_CUSTOMTEST_DINLINE                                         */
/* ------------------------------------------------------------------ */

#define FASTEST_CUSTOMTEST_DINLINE_HELPER(name, preload_flags, CALLBACK_BODY, BODY, id) \
  static void FASTEST_JOIN(FASTEST_body_, id)(FASTEST_TestOutput *out) {      \
    (void)out;                                                                 \
    BODY;                                                                      \
  }                                                                            \
  static void FASTEST_JOIN(FASTEST_cb_, id)(FASTEST_TestOutput *out) {        \
    (void)out;                                                                 \
    CALLBACK_BODY;                                                             \
  }                                                                            \
  FASTEST_REGISTER_(name,                                                      \
                    FASTEST_JOIN(FASTEST_body_, id),                           \
                    FASTEST_JOIN(FASTEST_cb_,   id),                           \
                    preload_flags,                                              \
                    id)

#define FASTEST_CUSTOMTEST_DINLINE(name, preload_flags, CALLBACK_BODY, BODY)   \
  FASTEST_CUSTOMTEST_DINLINE_HELPER(name, preload_flags, CALLBACK_BODY, BODY, __COUNTER__)
