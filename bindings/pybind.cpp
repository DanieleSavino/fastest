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

    m.def("run_test", [](const char *name) {
        FASTEST_list_t *list;
        FASTEST_list_getInstance(&list);

        uint64_t err = FASTEST_list_exec_name(list, name);

        if(! (err & FASTEST_SUCCESS)) {
            FASTEST_PrintError(err);
        }
    });
}
