#include "fastest/test_list.h"
#include "fastest/logging.h"
#include "fastest/tests.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

uint64_t FASTEST_list_init(FASTEST_list_t **list, size_t init_size) {
    uint64_t err = FASTEST_SUCCESS;

    FASTEST_list_t *der_list = NULL;
    FASTEST_MALLOC(der_list, sizeof(FASTEST_list_t), &err, cleanup);

    FASTEST_MALLOC(der_list->_buffer, init_size * sizeof(FASTEST_SchedTest), &err, cleanup);

    der_list->_buff_size = init_size;
    der_list->len = 0;
    *list = der_list;

    return FASTEST_SUCCESS;

    cleanup:
        free(der_list);
        return err;
}

void schedcpy(const FASTEST_SchedTest *test, FASTEST_SchedTest *out) {
    out->test_name = test->test_name;
    out->func = test->func;
    out->out = test->out;
    out->callback = test->callback;
    out->status = test->status;
}

static inline int check_key(FASTEST_list_t * const list, const FASTEST_SchedTest *test) {
    FASTEST_SchedTest *cmp_test;

    for(size_t i = 0; i < list->len; i++) {
        // WARN: Ignoring the out of bounds error, should not happen.
        FASTEST_list_get(list, i, &cmp_test);
        if(strcmp(cmp_test->test_name, test->test_name) == 0) {
            ERROR_PRINTF("Test: %s defined twice\n", test->test_name);
            return 1;
        }
    }

    return 0;
}

uint64_t FASTEST_list_push(FASTEST_list_t * const list, const FASTEST_SchedTest *test) {
    size_t buff_size = list->_buff_size;
    size_t list_len = list->len;

    if(check_key(list, test)) {
        return FASTEST_ERROR_COLLISION;
    }

    if(list_len >= buff_size) {
        void *tmp = realloc(list->_buffer, list->_buff_size * 2 * sizeof(FASTEST_SchedTest));
        if(!tmp) {
            return FASTEST_ERROR_MEMORY;
        }
        list->_buffer = tmp;
        list->_buff_size *= 2;
    }

    schedcpy(test, &list->_buffer[list_len]);
    list->_buffer[list_len].status = FASTEST_STATUS_PENDING;
    list->len++;

    return FASTEST_SUCCESS;
}

uint64_t FASTEST_list_get(const FASTEST_list_t *const list, size_t idx, FASTEST_SchedTest **out) {
    FASTEST_SchedTest *buffer = list->_buffer;
    size_t list_len = list->len;

    if(idx >= list_len) {
        return FASTEST_ERROR_INTERNAL;
    }

    *out = &(buffer[idx]);
    return FASTEST_SUCCESS;
}

uint64_t FASTEST_list_get_name(const FASTEST_list_t *const list, const char *name, FASTEST_SchedTest **out) {
    FASTEST_SchedTest *test;

    for(size_t i = 0; i < list->len; i++) {
        FASTEST_list_get(list, i, &test);

        if(strcmp(test->test_name, name) == 0) {
            *out = test;
            return FASTEST_SUCCESS;
        }
    }

    return FASTEST_ERROR_NOT_FOUND;
}

uint64_t FASTEST_list_free(FASTEST_list_t * const list) {
    if(list == NULL)
        return FASTEST_ERROR_INTERNAL;

    free(list->_buffer);
    free(list);

    return FASTEST_SUCCESS;
}

uint64_t FASTEST_list_exec(const FASTEST_list_t *list, size_t idx) {
    if(list == NULL || idx >= list->len) {
        return FASTEST_ERROR_INTERNAL;
    }

    uint64_t err = FASTEST_SUCCESS;

    FASTEST_SchedTest *test;
    FASTEST_CHECK(FASTEST_list_get(list, idx, &test), &err, cleanup);

    test->out.test_name = test->test_name;
    test->status = FASTEST_STATUS_RUNNING;

    struct timespec start, end;
    timespec_get(&start, TIME_UTC);
    test->func(&test->out);
    timespec_get(&end, TIME_UTC);

    test->status = FASTEST_STATUS_EXECUTED;

    if (FASTEST_Get_time_mode(test->out.test_flags))
        test->out.time_ns = (uint64_t)(end.tv_sec - start.tv_sec) * 1000000000ULL +
                            (end.tv_nsec - start.tv_nsec);

    if(test->out.test_flags & FASTEST_DEFAULT_LOG)
        FASTEST_Print_result(&test->out);

    if(test->callback != NULL)
        test->callback(&test->out);

cleanup:
    return err;
}

uint64_t FASTEST_list_exec_name(const FASTEST_list_t *list, const char* name) {
    FASTEST_SchedTest *test;
    size_t idx = SIZE_MAX;

    for(size_t i = 0; i < list->len; i++) {
        FASTEST_list_get(list, i, &test);

        if(strcmp(test->test_name, name) == 0) {
            idx = i;
        }
    }

    if(idx == SIZE_MAX) {
        return FASTEST_ERROR_NOT_FOUND;
    }

    return FASTEST_list_exec(list, idx);
}

uint64_t FASTEST_list_getInstance(FASTEST_list_t **list) {
    if(list == NULL) {
        return FASTEST_ERROR_INTERNAL;
    }

    uint64_t err = FASTEST_SUCCESS;

    static FASTEST_list_t *s_list;
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

uint64_t FASTEST_sched_pprint(const FASTEST_SchedTest *test) {
    if (test == NULL)
        return FASTEST_ERROR_INTERNAL;

    fprintf(stdout, "  [%s] %s\n", FASTEST_strstatus(test->status), test->test_name);
    return FASTEST_SUCCESS;
}

uint64_t FASTEST_list_pprint(const FASTEST_list_t * const list) {
    if (!list) {
        fprintf(stderr, "FASTEST_list_t: (null)\n");
        return FASTEST_ERROR_INTERNAL;
    }
    printf("FASTEST_list_t {\n");
    printf("  len       = %zu\n", list->len);
    printf("  buff_size = %zu\n", list->_buff_size);
    printf("  buffer    = %p\n", (void *)list->_buffer);
    printf("}\n");
    for (size_t i = 0; i < list->len; i++) {
        printf("[%zu] ", i);
        FASTEST_sched_pprint(&list->_buffer[i]);
    }
    return FASTEST_SUCCESS;
}
