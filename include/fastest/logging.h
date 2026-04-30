#pragma once

#include <stdio.h>
#include "fastest/tests.h"

/* ------------------------------------------------------------------ */
/*  ANSI formatting                                                    */
/* ------------------------------------------------------------------ */

#define FASTEST_RESET      "\033[0m"
#define FASTEST_BOLD       "\033[1m"
#define FASTEST_DIM        "\033[2m"
#define FASTEST_ITALIC     "\033[3m"
#define FASTEST_UNDERLINE  "\033[4m"

#define FASTEST_GREEN      "\033[92m"
#define FASTEST_YELLOW     "\033[93m"
#define FASTEST_RED        "\033[91m"
#define FASTEST_MAGENTA    "\033[35m"
#define FASTEST_BLUE       "\033[94m"

/* ------------------------------------------------------------------ */
/*  Logging macros                                                     */
/* ------------------------------------------------------------------ */

#define SUCCESS_PRINTF(fstring, ...)                                            \
    fprintf(stdout,                                                             \
            FASTEST_BOLD FASTEST_GREEN "[FASTEST SUCCESS] " FASTEST_RESET      \
            fstring "\n", ##__VA_ARGS__)

#define WARNING_PRINTF(fstring, ...)                                            \
    fprintf(stderr,                                                             \
            FASTEST_BOLD FASTEST_YELLOW "[FASTEST WARNING] " FASTEST_RESET     \
            fstring "\n", ##__VA_ARGS__)

#define LOG_PRINTF(fstring, ...)                                                \
    fprintf(stderr,                                                             \
            FASTEST_BOLD FASTEST_BLUE "[FASTEST LOG] " FASTEST_RESET           \
            fstring "\n", ##__VA_ARGS__)

#define ERROR_PRINTF(fstring, ...)                                              \
    fprintf(stderr,                                                             \
            FASTEST_BOLD FASTEST_RED "[FASTEST ERROR] " FASTEST_RESET          \
            fstring "\n", ##__VA_ARGS__)

#ifdef DEBUG
#   define DEBUG_PRINTF(fstring, ...)                                           \
        fprintf(stderr,                                                         \
                FASTEST_BOLD FASTEST_MAGENTA "[FASTEST DEBUG] " FASTEST_RESET  \
                fstring "\n", ##__VA_ARGS__)
#else
#   define DEBUG_PRINTF(fstring, ...) ;
#endif

/* ------------------------------------------------------------------ */
/*  Functions                                                          */
/* ------------------------------------------------------------------ */

void FASTEST_print_error(FASTEST_Err_t err);
void FASTEST_print_result(const FASTEST_TestOutput_t *out);
