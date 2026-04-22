#pragma once
#include "fastest/test_list.h"

#define FASTEST_JOIN2(a, b) a##b
#define FASTEST_JOIN(a, b)  FASTEST_JOIN2(a, b)

/* ------------------------------------------------------------------ */
/*  Internal: shared registration logic                                */
/* ------------------------------------------------------------------ */

#define FASTEST_REGISTER_(name_, func_, callback_, id)                         \
  static void FASTEST_JOIN(FASTEST_reg_, id)(void)                             \
      __attribute__((constructor));                                             \
  static void FASTEST_JOIN(FASTEST_reg_, id)(void) {                           \
    FASTEST_list_t *list = NULL;                                               \
    if (FASTEST_list_getInstance(&list) != FASTEST_SUCCESS) return;           \
    FASTEST_SchedTest entry = {                                                \
        .func      = (func_),                                                  \
        .test_name = (name_),                                                  \
        .callback  = (callback_),                                              \
        .out       = {0},                                                      \
    };                                                                         \
    FASTEST_list_push(list, &entry);                                           \
  }

/* ------------------------------------------------------------------ */
/*  FASTEST_CUSTOMTEST                                                 */
/*  test is already a FASTEST_Func                                     */
/* ------------------------------------------------------------------ */

#define FASTEST_CUSTOMTEST(name, test, callback)                               \
  FASTEST_REGISTER_(name, test, callback, __COUNTER__)

/* ------------------------------------------------------------------ */
/*  FASTEST_CUSTOMTEST_INLINE                                          */
/*  Body is an inline block; wraps it in a named static function       */
/* ------------------------------------------------------------------ */

#define FASTEST_CUSTOMTEST_INLINE_HELPER(name, callback, BODY, id)            \
  static void FASTEST_JOIN(FASTEST_body_, id)(FASTEST_TestOutput *out) {      \
    (void)out;                                                                 \
    BODY;                                                                      \
  }                                                                            \
  FASTEST_REGISTER_(name, FASTEST_JOIN(FASTEST_body_, id), callback, id)

#define FASTEST_CUSTOMTEST_INLINE(name, callback, BODY)                        \
  FASTEST_CUSTOMTEST_INLINE_HELPER(name, callback, BODY, __COUNTER__)

/* ------------------------------------------------------------------ */
/*  FASTEST_CUSTOMTEST_DINLINE                                         */
/*  Both body and callback are inline blocks                           */
/* ------------------------------------------------------------------ */

#define FASTEST_CUSTOMTEST_DINLINE_HELPER(name, CALLBACK_BODY, BODY, id)      \
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
                    id)

#define FASTEST_CUSTOMTEST_DINLINE(name, CALLBACK_BODY, BODY)                  \
  FASTEST_CUSTOMTEST_DINLINE_HELPER(name, CALLBACK_BODY, BODY, __COUNTER__)
