#pragma once

#include "fastest/tests.h"
#include <stddef.h>
#include <stdint.h>

#define FASTEST_DEF_INIT_SIZE 10

typedef void (*FASTEST_Func)(FASTEST_TestOutput *);

typedef struct {
    const char *test_name;
    FASTEST_Func func;
    FASTEST_Func callback;
    FASTEST_TestOutput out;
} FASTEST_SchedTest;

typedef struct {
    FASTEST_SchedTest *_buffer;
    size_t len;

    size_t _buff_size;
} FASTEST_list_t;

uint64_t FASTEST_list_init(FASTEST_list_t **list, size_t init_size);
uint64_t FASTEST_list_push(FASTEST_list_t * const list, const FASTEST_SchedTest *test);
uint64_t FASTEST_list_get(const FASTEST_list_t *const list, size_t idx, FASTEST_SchedTest **out);
uint64_t FASTEST_list_pop(FASTEST_list_t * const list, FASTEST_SchedTest **out);
uint64_t FASTEST_list_free(FASTEST_list_t * const list);

uint64_t FASTEST_list_getInstance(FASTEST_list_t **list);

uint64_t FASTEST_list_pprint(const FASTEST_list_t * const list);
uint64_t FASTEST_sched_pprint(const FASTEST_SchedTest * const test);
