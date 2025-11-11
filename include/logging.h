#pragma once

// ==========================
// ANSI Color Macros (Minimal)
// ==========================

#include <stdio.h>

#include "tests.h"
#define FASTEST_RESET "\033[0m"

#define FASTEST_BOLD "\033[1m"
#define FASTEST_DIM "\033[2m"
#define FASTEST_ITALIC "\033[3m"
#define FASTEST_UNDERLINE "\033[4m"

#define FASTEST_GREEN "\033[92m"
#define FASTEST_YELLOW "\033[93m"
#define FASTEST_RED "\033[91m"
#define FASTEST_MAGENTA "\033[35m"
#define FASTEST_BLUE "\033[94m"

#define SUCCESS_PRINTF(fstring, ...)                                           \
    fprintf(stdout,                                                            \
            FASTEST_BOLD FASTEST_GREEN "[SUCCESS] " FASTEST_RESET fstring      \
                                       "\n",                                   \
            ##__VA_ARGS__)

#define WARNING_PRINTF(fstring, ...)                                           \
    fprintf(stderr,                                                            \
            FASTEST_BOLD FASTEST_YELLOW "[WARNING] " FASTEST_RESET fstring     \
                                        "\n",                                  \
            ##__VA_ARGS__)

#define LOG_PRINTF(fstring, ...)                                               \
    fprintf(stderr,                                                            \
            FASTEST_BOLD FASTEST_BLUE "[LOG] " FASTEST_RESET fstring "\n",     \
            ##__VA_ARGS__)

#define ERROR_PRINTF(fstring, ...)                                             \
    fprintf(stderr,                                                            \
            FASTEST_BOLD FASTEST_RED "[ERROR] " FASTEST_RESET fstring "\n",    \
            ##__VA_ARGS__)

#ifdef DEBUG

#define DEBUG_PRINTF(fstring, ...)                                             \
    fprintf(stderr,                                                            \
            FASTEST_BOLD FASTEST_MAGENTA                        \
            "[DEBUG] " FASTEST_RESET fstring,                                  \
            ##__VA_ARGS__)

#else

#define DEBUG_PRINTF(fstring, ...) ;

#endif // DEBUG

void FASTEST_Print_result(const FASTEST_TestOutput *out);
