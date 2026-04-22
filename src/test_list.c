#include "fastest/test_list.h"
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

    FASTEST_MALLOC(der_list->_buffer, init_size * sizeof(FASTEST_TestOutput), &err, cleanup);

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
}

uint64_t FASTEST_list_push(FASTEST_list_t * const list, const FASTEST_SchedTest *test) {
    size_t buff_size = list->_buff_size;
    size_t list_len = list->len;

    if(list_len >= buff_size) {
        void *tmp = realloc(list->_buffer, list->_buff_size * 2 * sizeof(FASTEST_TestOutput));
        if(!tmp) {
            return FASTEST_ERROR_MEMORY;
        }
        list->_buffer = tmp;
        list->_buff_size *= 2;
    }

    schedcpy(test, &list->_buffer[list_len]);
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

uint64_t FASTEST_list_free(FASTEST_list_t * const list) {
    if(list == NULL)
        return FASTEST_ERROR_INTERNAL;

    free(list->_buffer);
    free(list);

    return FASTEST_SUCCESS;
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

    const char *status = test->out.time_ns != 0 ? "done" : "pending";
    fprintf(stdout, "  [%s] %s\n", status, test->test_name);
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
