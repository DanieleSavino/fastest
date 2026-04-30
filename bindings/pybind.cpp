#include "fastest/tests.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <algorithm>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

extern "C" {
#include "fastest/test_list.h"
#include "fastest/logging.h"
}

namespace py = pybind11;
using namespace pybind11::literals;

/* ------------------------------------------------------------------ */
/*  Error handling bridge                                             */
/* ------------------------------------------------------------------ */

static inline void throw_if_error(FASTEST_Err_t err, const char *where) {
    if (err == FASTEST_OK)
        return;

    std::string msg = std::string(where) + ": " + FASTEST_strerr(err);

    switch (err) {
        case FASTEST_ERR_OOM:
            throw std::bad_alloc();
        case FASTEST_ERR_NULLPTR:
            throw py::value_error(msg);
        case FASTEST_ERR_NOT_FOUND:
            throw py::key_error(msg);
        case FASTEST_ERR_OUT_OF_BOUNDS:
            throw py::index_error(msg);
        default:
            throw std::runtime_error(msg);
    }
}

/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */

static std::string sanitize_name(const char *name) {
    std::string s(name);
    std::replace(s.begin(), s.end(), '/', '_');
    std::replace(s.begin(), s.end(), '-', '_');
    std::replace(s.begin(), s.end(), ' ', '_');
    return s;
}

static py::dict test_to_dict(const FASTEST_SchedTest_t *test) {
    return py::dict(
        "test_name"_a    = test->test_name,
        "status"_a       = FASTEST_str_status(test->status),
        "test_flags"_a   = test->out.test_flags,
        "exit_status"_a  = test->out.exit_status,
        "allocation"_a   = test->out.allocation,
        "deallocation"_a = test->out.deallocation,
        "time_ns"_a      = test->out.time_ns
    );
}

/* ------------------------------------------------------------------ */
/*  Module                                                            */
/* ------------------------------------------------------------------ */

PYBIND11_MODULE(fastest, m) {

    py::module_ tests_subm =
        m.def_submodule("tests", "Auto-generated test constants");

    /* -------------------------------------------------------------- */
    /* Export test names as constants                                 */
    /* -------------------------------------------------------------- */
    {
        FASTEST_List_t *list = nullptr;
        FASTEST_Err_t err = FASTEST_list_get_instance(&list);
        throw_if_error(err, "FASTEST_list_get_instance");

        for (size_t i = 0; i < list->len; ++i) {
            const FASTEST_SchedTest_t *test = &list->_buffer[i];

            if (!test || !test->test_name)
                continue;

            std::string py_symbol = sanitize_name(test->test_name);
            tests_subm.attr(py_symbol.c_str()) = test->test_name;
        }
    }

    /* -------------------------------------------------------------- */
    /* Get single test                                                */
    /* -------------------------------------------------------------- */
    m.def("get_test", [](const char *name) -> py::object {

        FASTEST_List_t *list = nullptr;
        FASTEST_SchedTest_t *test = nullptr;

        FASTEST_Err_t err;

        err = FASTEST_list_get_instance(&list);
        throw_if_error(err, "FASTEST_list_get_instance");

        err = FASTEST_list_get_name(list, name, &test);
        throw_if_error(err, "FASTEST_list_get_name");

        return test_to_dict(test);
    });

    /* -------------------------------------------------------------- */
    /* Get subtests                                                   */
    /* -------------------------------------------------------------- */
    m.def("get_subtests", [](const char *prefix) -> py::list {

        FASTEST_List_t *list = nullptr;

        FASTEST_Err_t err = FASTEST_list_get_instance(&list);
        throw_if_error(err, "FASTEST_list_get_instance");

        py::list results;

        std::string search(prefix);
        search += "/";

        for (size_t i = 0; i < list->len; ++i) {
            auto *test = &list->_buffer[i];

            if (!test || !test->test_name)
                continue;

            std::string_view name(test->test_name);

            if (name.substr(0, search.size()) == search)
                results.append(test_to_dict(test));
        }

        return results;
    });

    /* -------------------------------------------------------------- */
    /* Get all tests                                                  */
    /* -------------------------------------------------------------- */
    m.def("get_tests", []() -> py::list {

        FASTEST_List_t *list = nullptr;

        FASTEST_Err_t err = FASTEST_list_get_instance(&list);
        throw_if_error(err, "FASTEST_list_get_instance");

        py::list results;

        for (size_t i = 0; i < list->len; ++i) {
            auto *test = &list->_buffer[i];

            if (test)
                results.append(test_to_dict(test));
        }

        return results;
    });

    /* -------------------------------------------------------------- */
    /* Run test                                                       */
    /* -------------------------------------------------------------- */
    m.def("run_test", [](const char *name) {

        FASTEST_List_t *list = nullptr;

        FASTEST_Err_t err = FASTEST_list_get_instance(&list);
        throw_if_error(err, "FASTEST_list_get_instance");

        err = FASTEST_list_exec_name(list, name);
        throw_if_error(err, "FASTEST_list_exec_name");
    });
}
