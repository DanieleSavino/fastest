#include "fastest/tests.h"
#include <cstdint>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// Tell C++ that these functions use C-style linkage
extern "C" {
    #include "fastest/test_list.h"
    #include "fastest/logging.h"
}

namespace py = pybind11;
using namespace pybind11::literals;

std::string sanitize_name(const char* name) {
    std::string s(name);
    std::replace(s.begin(), s.end(), '/', '_');
    std::replace(s.begin(), s.end(), '-', '_');
    return s;
}

static py::dict test_to_dict(const FASTEST_SchedTest *test) {
    return py::dict(
        "test_name"_a    = test->test_name,
        "status"_a       = FASTEST_strstatus(test->status),
        "test_flags"_a   = test->out.test_flags,
        "exit_status"_a  = test->out.exit_status,
        "allocation"_a   = test->out.allocation,
        "deallocation"_a = test->out.deallocation,
        "time_ns"_a      = test->out.time_ns
    );
}

PYBIND11_MODULE(fastest, m) {

    py::module_ tests_subm = m.def_submodule("tests", "Auto-generated test constants");

    // Add tests symbols
    FASTEST_list_t *list = nullptr;
    if (FASTEST_list_getInstance(&list) == FASTEST_SUCCESS && list) {

        for (size_t i = 0; i < list->len; ++i) {
            const FASTEST_SchedTest *test = &list->_buffer[i];

            if (test && test->test_name) {
                std::string py_symbol = sanitize_name(test->test_name);
                tests_subm.attr(py_symbol.c_str()) = test->test_name;
            }
        }
    }

    m.def("get_test", [](const char *name) -> py::object {
        FASTEST_list_t *list;
        FASTEST_list_getInstance(&list);

        FASTEST_SchedTest *test;

        uint64_t err;
        FASTEST_CHECK(FASTEST_list_get_name(list, name, &test), &err, not_found);

        if(! (err & FASTEST_SUCCESS)) {
            FASTEST_PrintError(err);
        }

        return test_to_dict(test);

    not_found:
        return py::none();

    });

    m.def("get_subtests", [](const char *prefix) -> py::list {
        FASTEST_list_t *list;
        FASTEST_list_getInstance(&list);

        py::list results;

        std::string search_str = std::string(prefix) + "/";

        for (size_t i = 0; i < list->len; ++i) {
            FASTEST_SchedTest *test = &list->_buffer[i];

            if (test && test->test_name) {
                std::string_view name(test->test_name);

                if (name.substr(0, search_str.size()) == search_str) {
                    results.append(test_to_dict(test));
                }
            }
        }

        return results;
    });

    m.def("get_tests", []() -> py::list {
        FASTEST_list_t *list;
        FASTEST_list_getInstance(&list);

        py::list results;

        for (size_t i = 0; i < list->len; ++i) {
            FASTEST_SchedTest *test = &list->_buffer[i];

            if (test) {
                results.append(test_to_dict(test));
            }
        }

        return results;
    });

    m.def("run_test", [](const char *name) {
        FASTEST_list_t *list;
        FASTEST_list_getInstance(&list);

        uint64_t err = FASTEST_list_exec_name(list, name);

        if(! (err & FASTEST_SUCCESS)) {
            FASTEST_PrintError(err);
        }
    });
}
