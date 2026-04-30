#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  Compiler portability                                               */
/* ------------------------------------------------------------------ */

#if defined(__GNUC__) || defined(__clang__)
#   define NODISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER)
#   define NODISCARD _Check_return_
#else
#   define NODISCARD
#endif

#define IGNORE (void)

/* ------------------------------------------------------------------ */
/*  Test flags (bitmap, combinable with |)                            */
/* ------------------------------------------------------------------ */

/* Assertion mode */
#define FASTEST_ASSERT_EQ       (1 << 0)
#define FASTEST_ASSERT_NEQ      (1 << 1)
#define FASTEST_ASSERT_GT       (1 << 2)
#define FASTEST_ASSERT_GE       (1 << 3)
#define FASTEST_ASSERT_LT       (1 << 4)
#define FASTEST_ASSERT_LE       (1 << 5)

/* Failure reporting mode */
#define FASTEST_FAIL_ERROR      (1 << 6)
#define FASTEST_FAIL_WARNING    (1 << 7)
#define FASTEST_FAIL_LOG        (1 << 8)

/* Timing resolution */
#define FASTEST_TIME_S          (1 << 9)
#define FASTEST_TIME_MS         (1 << 10)
#define FASTEST_TIME_US         (1 << 11)
#define FASTEST_TIME_NS         (1 << 12)

/* Memory tracking */
#define FASTEST_MEM_TRACK       (1 << 13)

/* Logging */
#define FASTEST_DEFAULT_LOG     (1 << 21)


/* ------------------------------------------------------------------ */
/*  Test exit status (bitmap — multiple can be set simultaneously)    */
/* ------------------------------------------------------------------ */

#define FASTEST_SUCCESS             (1 << 0)
#define FASTEST_SKIPPED             (1 << 1)
#define FASTEST_INCOMPLETE          (1 << 2)

#define FASTEST_ERROR_ASSERT        (1 << 8)
#define FASTEST_ERROR_UNEXPECTED    (1 << 9)
#define FASTEST_ERROR_EXCEPTION     (1 << 10)
#define FASTEST_ERROR_MEMORY        (1 << 11)
#define FASTEST_ERROR_TIMEOUT       (1 << 12)
#define FASTEST_ERROR_RESOURCE      (1 << 13)
#define FASTEST_ERROR_MPI           (1 << 14)
#define FASTEST_ERROR_OMP           (1 << 15)
#define FASTEST_ERROR_CUDA          (1 << 16)
#define FASTEST_ERROR_INTERNAL      (1 << 17)
#define FASTEST_ERROR_UNKNOWN       (1 << 18)
#define FASTEST_ERROR_COLLISION     (1 << 19)
#define FASTEST_ERROR_NOT_FOUND     (1 << 20)

static inline const char *FASTEST_strexit(uint64_t exit_status,
                                                uint64_t flags) {
    if (exit_status & FASTEST_SUCCESS)
        return "SUCCESS";

    if (exit_status & FASTEST_ERROR_ASSERT) {
        if (flags & FASTEST_ASSERT_EQ)
            return "ASSERT_EQ failed";
        if (flags & FASTEST_ASSERT_NEQ)
            return "ASSERT_NEQ failed";
        if (flags & FASTEST_ASSERT_GT)
            return "ASSERT_GT failed";
        if (flags & FASTEST_ASSERT_GE)
            return "ASSERT_GE failed";
        if (flags & FASTEST_ASSERT_LT)
            return "ASSERT_LT failed";
        if (flags & FASTEST_ASSERT_LE)
            return "ASSERT_LE failed";
        return "ASSERT failed";
    }

    if (exit_status & FASTEST_ERROR_UNEXPECTED)
        return "Unexpected result";
    if (exit_status & FASTEST_ERROR_EXCEPTION)
        return "Exception occurred";
    if (exit_status & FASTEST_ERROR_MEMORY)
        return "Memory error";
    if (exit_status & FASTEST_ERROR_TIMEOUT)
        return "Timeout";
    if (exit_status & FASTEST_ERROR_RESOURCE)
        return "Resource error";
    if (exit_status & FASTEST_ERROR_MPI)
        return "MPI error";
    if (exit_status & FASTEST_ERROR_OMP)
        return "OpenMP error";
    if (exit_status & FASTEST_ERROR_CUDA)
        return "CUDA error";
    if (exit_status & FASTEST_ERROR_INTERNAL)
        return "Internal framework error";
    if (exit_status & FASTEST_ERROR_UNKNOWN)
        return "Unknown error";
    if(exit_status & FASTEST_ERROR_COLLISION)
        return "Collision in test names";

    return "Unknown status";
}

/* ------------------------------------------------------------------ */
/*  Infrastructure error codes (single value, not a bitmap)            */
/* ------------------------------------------------------------------ */

typedef enum {
    FASTEST_OK = 0,
    FASTEST_ERR_OOM,
    FASTEST_ERR_NULLPTR,
    FASTEST_ERR_NOT_FOUND,
    FASTEST_ERR_DUPLICATE,
    FASTEST_ERR_OUT_OF_BOUNDS,
    FASTEST_ERR_UNKNOWN,
} FASTEST_Err_t;

static inline const char* FASTEST_strerr(FASTEST_Err_t err) {
    switch (err) {
        case FASTEST_OK:
            return "OK";
        case FASTEST_ERR_OOM:
            return "Out of memory";
        case FASTEST_ERR_NULLPTR:
            return "Null pointer";
        case FASTEST_ERR_NOT_FOUND:
            return "Not found";
        case FASTEST_ERR_DUPLICATE:
            return "Duplicate entry";
        case FASTEST_ERR_OUT_OF_BOUNDS:
            return "Out of bounds";
        case FASTEST_ERR_UNKNOWN:
            return "Unknown error";
        default:
            return "Invalid error code";
    }
}

/* ------------------------------------------------------------------ */
/*  Test output                                                        */
/* ------------------------------------------------------------------ */

typedef struct {
    const char *test_name;
    uint64_t    test_flags;
    uint64_t    exit_status;
    uint64_t    allocation;
    uint64_t    deallocation;
    uint64_t    time_ns;
} FASTEST_TestOutput_t;

/* ------------------------------------------------------------------ */
/*  Flag helpers                                                       */
/* ------------------------------------------------------------------ */

static inline uint64_t FASTEST_get_assert_mode(uint64_t flags) {
    if (flags & FASTEST_ASSERT_EQ)  return FASTEST_ASSERT_EQ;
    if (flags & FASTEST_ASSERT_NEQ) return FASTEST_ASSERT_NEQ;
    if (flags & FASTEST_ASSERT_GT)  return FASTEST_ASSERT_GT;
    if (flags & FASTEST_ASSERT_GE)  return FASTEST_ASSERT_GE;
    if (flags & FASTEST_ASSERT_LT)  return FASTEST_ASSERT_LT;
    if (flags & FASTEST_ASSERT_LE)  return FASTEST_ASSERT_LE;
    return 0;
}

static inline uint64_t FASTEST_get_time_mode(uint64_t flags) {
    if (flags & FASTEST_TIME_NS) return FASTEST_TIME_NS;
    if (flags & FASTEST_TIME_MS) return FASTEST_TIME_MS;
    if (flags & FASTEST_TIME_US) return FASTEST_TIME_US;
    if (flags & FASTEST_TIME_S)  return FASTEST_TIME_S;
    return 0;
}

NODISCARD static inline FASTEST_Err_t FASTEST_test_cpy(const FASTEST_TestOutput_t *src,
                                              FASTEST_TestOutput_t       *dst) {
    if (src == NULL || dst == NULL)
        return FASTEST_ERR_NULLPTR;

    dst->test_name    = src->test_name;
    dst->test_flags   = src->test_flags;
    dst->exit_status  = src->exit_status;
    dst->allocation   = src->allocation;
    dst->deallocation = src->deallocation;
    dst->time_ns      = src->time_ns;

    return FASTEST_OK;
}

/* ------------------------------------------------------------------ */
/*  Utility macros                                                     */
/* ------------------------------------------------------------------ */

#define FASTEST_MALLOC(ptr, size, err_ref, label)   \
do {                                                \
    (ptr) = malloc(size);                           \
    if (!(ptr)) {                                   \
        *(err_ref) = FASTEST_ERR_OOM;             \
        goto label;                                 \
    }                                               \
} while (0)

#define FASTEST_CHECK(call, err_ref, label)                                         \
do {                                                                                \
    *(err_ref) = (call);                                                            \
    if (*(err_ref) != FASTEST_OK) {                                                 \
        fprintf(stderr,                                                             \
                "[FASTEST INTERNAL ERROR] %s failed. In file: %s on line: %d\n",   \
                #call, __FILE__, __LINE__);                                         \
        goto label;                                                                 \
    }                                                                               \
} while (0)


NODISCARD static inline int has_null_ptr(size_t count, const void *ptrs[]) {
    for (size_t i = 0; i < count; ++i) {
        if (ptrs[i] == NULL) {
            return 1;
        }
    }
    return 0;
}

#define FASTEST_CHECK_NULLS(...)                                      \
    do {                                                              \
        const void *ptrs[] = { __VA_ARGS__ };                               \
        if (has_null_ptr(sizeof(ptrs) / sizeof(ptrs[0]), ptrs)) {     \
            return FASTEST_ERR_NULLPTR;                               \
        }                                                             \
    } while (0)
