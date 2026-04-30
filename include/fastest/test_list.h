#pragma once

#include "fastest/tests.h"
#include <stddef.h>
#include <stdint.h>

#define FASTEST_DEF_INIT_SIZE 10

typedef void (*FASTEST_Func_t)(FASTEST_TestOutput_t *);

typedef enum {
    FASTEST_STATUS_PENDING,
    FASTEST_STATUS_RUNNING,
    FASTEST_STATUS_EXECUTED,
} FASTEST_SchedStatus_t;

static inline const char *FASTEST_str_status(FASTEST_SchedStatus_t status) {
    switch (status) {
        case FASTEST_STATUS_PENDING:  return "Pending";
        case FASTEST_STATUS_RUNNING:  return "Running";
        case FASTEST_STATUS_EXECUTED: return "Executed";
        default:                      return "Unknown";
    }
}

typedef struct {
    const char           *test_name;
    FASTEST_Func_t        func;
    FASTEST_Func_t        callback;
    FASTEST_TestOutput_t  out;
    FASTEST_SchedStatus_t status;
} FASTEST_SchedTest_t;

typedef struct {
    FASTEST_SchedTest_t *_buffer;
    size_t               len;
    size_t               _buff_size;
} FASTEST_List_t;

NODISCARD FASTEST_Err_t FASTEST_list_init(FASTEST_List_t **list, size_t init_size);
NODISCARD FASTEST_Err_t FASTEST_list_push(FASTEST_List_t *list, const FASTEST_SchedTest_t *test);
NODISCARD FASTEST_Err_t FASTEST_list_get(const FASTEST_List_t *list, size_t idx, FASTEST_SchedTest_t **out);
NODISCARD FASTEST_Err_t FASTEST_list_get_name(const FASTEST_List_t *list, const char *name, FASTEST_SchedTest_t **out);
NODISCARD FASTEST_Err_t FASTEST_list_free(FASTEST_List_t *list);
NODISCARD FASTEST_Err_t FASTEST_list_exec(const FASTEST_List_t *list, size_t idx);
NODISCARD FASTEST_Err_t FASTEST_list_exec_name(const FASTEST_List_t *list, const char *name);
NODISCARD FASTEST_Err_t FASTEST_list_get_instance(FASTEST_List_t **list);
NODISCARD FASTEST_Err_t FASTEST_list_pprint(const FASTEST_List_t *list);
NODISCARD FASTEST_Err_t FASTEST_sched_pprint(const FASTEST_SchedTest_t *test);
