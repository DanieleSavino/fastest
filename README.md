# fastest

> **Note:** Toy project — built for personal learning, no stability guarantees and no intent for external use. That said, PRs are welcome.

A macro-based C testing framework built around a global test scheduler. Tests register themselves before `main()` via `__attribute__((constructor))` and can be run from C or Python. Nanosecond-precision timing and memory tracking included; zero external dependencies for the core library.

## Table of Contents

- [Project Structure](#project-structure)
- [Building](#building)
- [Integration](#integration)
- [Testing Modes](#testing-modes)
- [Flags Reference](#flags-reference)
- [Python Bindings](#python-bindings)
- [Examples](#examples)

---

## Project Structure

```
fastest/
├── include/fastest/
│   ├── tests.h           # Core types, flags, status codes, helper macros
│   ├── test_list.h       # Test scheduler (list, push, exec, get_instance)
│   ├── logging.h         # ANSI output macros
│   ├── quick_tests.h     # FASTEST_QUICKTEST / FASTEST_QUICKTEST_DELEGATE
│   └── custom_tests.h    # FASTEST_CUSTOMTEST / INLINE / DINLINE macros
├── src/
│   ├── logging.c         # FASTEST_print_result, FASTEST_print_error
│   └── test_list.c       # Scheduler implementation
├── bindings/
│   └── pybind.cpp        # Python module (pybind11)
├── python/
│   ├── setup.py          # Builds and installs the Python extension
│   ├── pyproject.toml
│   └── requirements.txt  # setuptools, wheel, pybind11, packaging
├── demo/
│   ├── shared_Makefile   # Shared Makefile for demo projects
│   ├── shared_build_wheel.sh
│   └── showcase/         # Example demo project
├── build/
│   └── libfastest.a      # Static library (generated)
└── CMakeLists.txt
```

---

## Building

### Prerequisites

- GCC / Clang (C11)
- G++ (C++17, for Python bindings)
- CMake ≥ 3.15
- pybind11 ≥ 2.11 (Python bindings only)

### Static Library

```bash
cmake -B build
cmake --build build
# Produces build/libfastest.a
```

---

## Integration

### C-only

Copy the library and headers into your project, then compile:

```bash
cp build/libfastest.a /your/project/lib/
cp -r include/fastest /your/project/include/

gcc -std=c11 -o tests tests.c -Iinclude -Llib -lfastest
```

Add `-DDEBUG` to enable `DEBUG_PRINTF` output.

### Demo project layout

The shared Makefile expects this layout inside your demo directory:

```
your-demo/
├── src/              # your .c source files
├── vendor/
│   └── fastest/      # symlink or copy of the fastest repo root
├── Makefile          # symlink to fastest/demo/shared_Makefile
└── build_wheel.sh    # symlink to fastest/demo/shared_build_wheel.sh
```

The symlink approach used in the demos:

```bash
ln -s /path/to/fastest/demo/shared_Makefile Makefile
ln -s /path/to/fastest/demo/shared_build_wheel.sh build_wheel.sh
mkdir -p vendor
ln -s /path/to/fastest vendor/fastest
```

The Makefile compiles all `.c` files under `src/` as PIC objects and
archives them into `build/release/lib<demo-name>.a`. `FASTEST_HOME`
defaults to `vendor/fastest`.

```bash
make              # release build → build/release/lib<name>.a
make BUILD=debug  # debug build with -g -O0 -DDEBUG
make clean
```

---

## Python Bindings

fastest exposes the test scheduler to Python via pybind11. The shared
`setup.py` in `fastest/python/` handles building and installing the
extension. The module name and linked libraries are configured via
environment variables.

### Building and installing

From your demo directory:

```bash
./build_wheel.sh
```

The script sets `FASTEST_HOME`, `FASTEST_USER_LIB` (your compiled
`.a`), and `FASTEST_MODULE_NAME` (defaults to the directory name), then
runs `pip install -e .` from `fastest/python/`. This produces an
editable install importable from anywhere in the active environment.

The installed module also generates a `.pyi` stub alongside the `.so`
for IDE autocompletion.

### Python API

```python
import showcase  # or whatever FASTEST_MODULE_NAME is set to

# Get all registered tests
showcase.get_tests()
# → [{"test_name": ..., "status": ..., "test_flags": ...,
#     "exit_status": ..., "allocation": ..., "deallocation": ...,
#     "time_ns": ...}, ...]

# Get a single test by name
showcase.get_test("my test")

# Get all tests whose names start with "suite/"
showcase.get_subtests("suite")

# Run a test by name (executes it, updates scheduler state in place)
showcase.run_test("my test")

# Test name constants — spaces/slashes/dashes normalised to underscores
showcase.tests.my_test      # == "my test"
showcase.tests.Dinline      # == "Dinline"
showcase.tests.custom_test  # == "custom test"
```

### Stub (`.pyi`)

Generated automatically after build:

```python
from typing import Any

def get_tests() -> list[dict[str, Any]]: ...
def get_test(name: str) -> dict[str, Any]: ...
def get_subtests(prefix: str) -> list[dict[str, Any]]: ...
def run_test(name: str) -> None: ...

class tests:
    Dinline: str
    custom_test: str
    inline: str
```

### Example

```python
import showcase as t

target = t.tests.custom_test

print(t.get_tests())          # inspect all tests before running
t.run_test(target)            # execute
print(t.get_test(target))     # inspect result (status, time_ns, exit_status updated)
```

---

## Testing Modes

fastest provides four ways to define tests.

### 1. QUICKTEST — Expression-based

Tests a boolean expression inline. Timing is measured around the expression. Does **not** register in the scheduler — result is printed immediately.

```c
FASTEST_QUICKTEST(name, expr, flags)
```

```c
FASTEST_QUICKTEST("add 2+3==5", add(2, 3) == 5,
    FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS);
```

### 2. QUICKTEST\_DELEGATE — Function-based quick test

Like QUICKTEST but calls a function with arguments and compares its return value against an expected value.

```c
FASTEST_QUICKTEST_DELEGATE(name, func, expected, flags, arg1, arg2, ...)
```

```c
FASTEST_QUICKTEST_DELEGATE("add", add, 5,
    FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS,
    2, 3);
```

### 3. CUSTOMTEST — Scheduler-registered function

Registers a named test function in the global scheduler via `__attribute__((constructor))`. The function receives `FASTEST_TestOutput_t *out` and sets flags and status manually.

```c
FASTEST_CUSTOMTEST(name, preload_flags, test_func, callback)
```

```c
void my_test(FASTEST_TestOutput_t *out) {
    out->test_flags  |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
}

FASTEST_CUSTOMTEST("my test", FASTEST_TIME_NS | FASTEST_FAIL_ERROR, my_test, NULL);
```

`preload_flags` are merged into `out->test_flags` before the test runs.
`callback` is an optional `void (*)(FASTEST_TestOutput_t *)` called after execution; pass `NULL` to skip.

### 4. CUSTOMTEST\_INLINE — Inline body, external callback

Test body is written inline; callback is a separate named function.

```c
FASTEST_CUSTOMTEST_INLINE(name, preload_flags, callback, { body })
```

```c
void after(FASTEST_TestOutput_t *out) {
    DEBUG_PRINTF("%s done", out->test_name);
}

FASTEST_CUSTOMTEST_INLINE("inline add", FASTEST_TIME_NS | FASTEST_FAIL_ERROR,
    after,
    {
        out->test_flags  |= FASTEST_ASSERT_EQ;
        out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
        out->exit_status |= FASTEST_DEFAULT_LOG;
    }
)
```

### 5. CUSTOMTEST\_DINLINE — Fully inline

Both test body and callback body are written inline. Most compact form.

```c
FASTEST_CUSTOMTEST_DINLINE(name, preload_flags, { callback_body }, { test_body })
```

```c
FASTEST_CUSTOMTEST_DINLINE("dinline add",
    FASTEST_TIME_NS | FASTEST_FAIL_ERROR | FASTEST_DEFAULT_LOG,
    {
        DEBUG_PRINTF("%s done, assert failed: %d",
            out->test_name,
            (out->exit_status & FASTEST_ERROR_ASSERT) > 0);
    },
    {
        out->test_flags  |= FASTEST_ASSERT_EQ;
        out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
        out->exit_status |= FASTEST_DEFAULT_LOG;
    }
)
```

### Running Scheduled Tests from C

```c
#include "fastest/test_list.h"

int main(void) {
    FASTEST_List_t *list;
    FASTEST_list_get_instance(&list);

    FASTEST_list_exec_name(list, "my test");  // run by name
    FASTEST_list_exec(list, 0);               // run by index
    return 0;
}
```

---

## Flags Reference

Flags are `uint64_t` bitmasks combined with `|`.

### Assertion

| Flag | Assertion |
|------|-----------|
| `FASTEST_ASSERT_EQ` | `==` |
| `FASTEST_ASSERT_NEQ` | `!=` |
| `FASTEST_ASSERT_GT` | `>` |
| `FASTEST_ASSERT_GE` | `>=` |
| `FASTEST_ASSERT_LT` | `<` |
| `FASTEST_ASSERT_LE` | `<=` |

### Failure reporting

| Flag | Output style |
|------|-------------|
| `FASTEST_FAIL_ERROR` | Red `[FASTEST ERROR]` |
| `FASTEST_FAIL_WARNING` | Yellow `[FASTEST WARNING]` |
| `FASTEST_FAIL_LOG` | Blue `[FASTEST LOG]` |

### Timing

| Flag | Unit |
|------|------|
| `FASTEST_TIME_NS` | nanoseconds |
| `FASTEST_TIME_US` | microseconds |
| `FASTEST_TIME_MS` | milliseconds |
| `FASTEST_TIME_S` | seconds |

### Misc

| Flag | Effect |
|------|--------|
| `FASTEST_MEM_TRACK` | Track allocation/deallocation, report leaks |
| `FASTEST_DEFAULT_LOG` | Enable `FASTEST_print_result` output |

### Exit status codes

Set on `out->exit_status` inside test functions.

| Code | Meaning |
|------|---------|
| `FASTEST_SUCCESS` | Test passed |
| `FASTEST_ERROR_ASSERT` | Assertion failed |
| `FASTEST_ERROR_MEMORY` | Memory leak detected |
| `FASTEST_ERROR_TIMEOUT` | Test exceeded time limit |
| `FASTEST_ERROR_EXCEPTION` | Signal or fatal error |
| `FASTEST_ERROR_UNEXPECTED` | Unexpected result |
| `FASTEST_ERROR_RESOURCE` | I/O or resource failure |
| `FASTEST_ERROR_MPI` | MPI failure |
| `FASTEST_ERROR_OMP` | OpenMP failure |
| `FASTEST_ERROR_CUDA` | CUDA failure |
| `FASTEST_ERROR_INTERNAL` | Framework misuse |
| `FASTEST_ERROR_COLLISION` | Duplicate test name |
| `FASTEST_ERROR_NOT_FOUND` | Test not in scheduler |

---

## Examples

### Minimal

```c
#include "fastest/quick_tests.h"
#include "fastest/tests.h"

int add(int a, int b) { return a + b; }

int main(void) {
    FASTEST_QUICKTEST("add 2+3==5", add(2, 3) == 5,
        FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS);
    return 0;
}
```

### Mixed modes

```c
#include "fastest/quick_tests.h"
#include "fastest/custom_tests.h"
#include "fastest/test_list.h"
#include "fastest/tests.h"

int multiply(int a, int b) { return a * b; }

FASTEST_CUSTOMTEST_INLINE("multiply/basic",
    FASTEST_TIME_NS | FASTEST_FAIL_ERROR,
    NULL,
    {
        out->test_flags  |= FASTEST_ASSERT_EQ;
        out->exit_status |= multiply(3, 4) == 12 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
        out->exit_status |= FASTEST_DEFAULT_LOG;
    }
)

int main(void) {
    FASTEST_QUICKTEST("3*4==12", multiply(3, 4) == 12,
        FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_US);

    FASTEST_List_t *list;
    FASTEST_list_get_instance(&list);
    FASTEST_list_exec_name(list, "multiply/basic");
    return 0;
}
```

### Memory tracking

```c
#include "fastest/custom_tests.h"
#include "fastest/tests.h"
#include <stdlib.h>

void mem_test(FASTEST_TestOutput_t *out) {
    out->test_flags |= FASTEST_MEM_TRACK | FASTEST_FAIL_ERROR;

    void *ptr = malloc(64);
    out->allocation += 64;
    free(ptr);
    out->deallocation += 64;

    out->exit_status |= FASTEST_SUCCESS | FASTEST_DEFAULT_LOG;
}

FASTEST_CUSTOMTEST("mem/alloc-free", 0, mem_test, NULL);
```

---

## Output

```
[FASTEST SUCCESS] multiply/basic
[FASTEST LOG]     multiply/basic: [time: 312ns]

[FASTEST ERROR]   add returns 5: ASSERT_EQ failed
```

---

## License

MIT License

Copyright (c) 2025 Daniele

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
