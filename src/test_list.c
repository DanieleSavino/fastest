#include "fastest/test_list.h"
#include "fastest/logging.h"
#include "fastest/tests.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

FASTEST_Err_t FASTEST_list_init(FASTEST_List_t **list, size_t init_size) {
    FASTEST_CHECK_NULLS(list);

    FASTEST_Err_t err = FASTEST_OK;

    FASTEST_List_t *der_list = NULL;
    FASTEST_MALLOC(der_list, sizeof(FASTEST_List_t), &err, cleanup);

    FASTEST_MALLOC(der_list->_buffer, init_size * sizeof(FASTEST_SchedTest_t), &err, cleanup);

    der_list->_buff_size = init_size;
    der_list->len = 0;
    *list = der_list;

    return FASTEST_OK;

    cleanup:
        free(der_list);
        return err;
}

void schedcpy(const FASTEST_SchedTest_t *test, FASTEST_SchedTest_t *out) {
    out->test_name = test->test_name;
    out->func = test->func;
    out->out = test->out;
    out->callback = test->callback;
    out->status = test->status;
}

static inline int check_key(FASTEST_List_t * const list, const FASTEST_SchedTest_t *test) {
    FASTEST_CHECK_NULLS(list, test);

    FASTEST_SchedTest_t *cmp_test;

    for(size_t i = 0; i < list->len; i++) {

        // INFO: Ignoring the out of bounds error, should not happen.
        IGNORE FASTEST_list_get(list, i, &cmp_test);

        if(strcmp(cmp_test->test_name, test->test_name) == 0) {
            ERROR_PRINTF("Test: %s defined twice\n", test->test_name);
            return 1;
        }
    }

    return 0;
}

FASTEST_Err_t FASTEST_list_push(FASTEST_List_t * const list, const FASTEST_SchedTest_t *test) {
    FASTEST_CHECK_NULLS(list, test);

    size_t buff_size = list->_buff_size;
    size_t list_len = list->len;

    if(check_key(list, test)) {
        return FASTEST_ERR_DUPLICATE;
    }

    if(list_len >= buff_size) {
        void *tmp = realloc(list->_buffer, list->_buff_size * 2 * sizeof(FASTEST_SchedTest_t));
        if(!tmp) {
            return FASTEST_ERR_OOM;
        }
        list->_buffer = tmp;
        list->_buff_size *= 2;
    }

    schedcpy(test, &list->_buffer[list_len]);
    list->_buffer[list_len].status = FASTEST_STATUS_PENDING;
    list->len++;

    return FASTEST_OK;
}

FASTEST_Err_t FASTEST_list_get(const FASTEST_List_t *const list, size_t idx, FASTEST_SchedTest_t **out) {
    FASTEST_CHECK_NULLS(list, out);

    FASTEST_SchedTest_t *buffer = list->_buffer;
    size_t list_len = list->len;

    if(idx >= list_len) {
        return FASTEST_ERR_OUT_OF_BOUNDS;
    }

    *out = &(buffer[idx]);
    return FASTEST_OK;
}

FASTEST_Err_t FASTEST_list_get_name(const FASTEST_List_t *const list, const char *name, FASTEST_SchedTest_t **out) {
    FASTEST_CHECK_NULLS(list, name, out);

    FASTEST_SchedTest_t *test;

    for(size_t i = 0; i < list->len; i++) {
        // INFO: Ignoring the out of bounds error, should not happen.
        IGNORE FASTEST_list_get(list, i, &test);

        if(strcmp(test->test_name, name) == 0) {
            *out = test;
            return FASTEST_OK;
        }
    }

    return FASTEST_ERR_NOT_FOUND;
}

FASTEST_Err_t FASTEST_list_free(FASTEST_List_t * const list) {
    FASTEST_CHECK_NULLS(list);

    free(list->_buffer);
    free(list);

    return FASTEST_OK;
}

FASTEST_Err_t FASTEST_list_exec(const FASTEST_List_t *list, size_t idx) {
    FASTEST_CHECK_NULLS(list);
    if(idx >= list->len) {
        return FASTEST_ERR_OUT_OF_BOUNDS;
    }

    FASTEST_Err_t err = FASTEST_OK;

    FASTEST_SchedTest_t *test;
    FASTEST_CHECK(FASTEST_list_get(list, idx, &test), &err, cleanup);

    test->out.test_name = test->test_name;
    test->status = FASTEST_STATUS_RUNNING;

    struct timespec start, end;
    timespec_get(&start, TIME_UTC);
    test->func(&test->out);
    timespec_get(&end, TIME_UTC);

    test->status = FASTEST_STATUS_EXECUTED;

    if (FASTEST_get_time_mode(test->out.test_flags))
        test->out.time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +
                            (end.tv_nsec - start.tv_nsec);

    FASTEST_print_result(&test->out);

    if(test->callback != NULL)
        test->callback(&test->out);

cleanup:
    return err;
}

FASTEST_Err_t FASTEST_list_exec_name(const FASTEST_List_t *list, const char* name) {
    FASTEST_CHECK_NULLS(list, name);

    FASTEST_SchedTest_t *test;
    size_t idx = SIZE_MAX;

    for(size_t i = 0; i < list->len; i++) {

        // INFO: Ignoring the out of bounds error, should not happen.
        IGNORE FASTEST_list_get(list, i, &test);

        if(strcmp(test->test_name, name) == 0) {
            idx = i;
        }
    }

    if(idx == SIZE_MAX) {
        return FASTEST_ERR_NOT_FOUND;
    }

    return FASTEST_list_exec(list, idx);
}

FASTEST_Err_t FASTEST_list_get_instance(FASTEST_List_t **list) {
    FASTEST_CHECK_NULLS(list);

    FASTEST_Err_t err = FASTEST_OK;

    static FASTEST_List_t *s_list;
    if(s_list == NULL) {
        FASTEST_CHECK(
            FASTEST_list_init(&s_list, FASTEST_DEF_INIT_SIZE),
            &err,
            cleanup
        );
    }

    *list = s_list;

cleanup:
    return err;
}

FASTEST_Err_t FASTEST_sched_pprint(const FASTEST_SchedTest_t *test) {
    FASTEST_CHECK_NULLS(test);

    fprintf(stdout, "  [%s] %s\n", FASTEST_str_status(test->status), test->test_name);
    return FASTEST_OK;
}

FASTEST_Err_t FASTEST_list_pprint(const FASTEST_List_t * const list) {
    FASTEST_CHECK_NULLS(list);

    printf("FASTEST_List_t {\n");
    printf("  len       = %zu\n", list->len);
    printf("  buff_size = %zu\n", list->_buff_size);
    printf("  buffer    = %p\n", (void *)list->_buffer);
    printf("}\n");
    for (size_t i = 0; i < list->len; i++) {
        printf("[%zu] ", i);

        // INFO: already checked non null list.
        IGNORE FASTEST_sched_pprint(&list->_buffer[i]);
    }
    return FASTEST_OK;
}
