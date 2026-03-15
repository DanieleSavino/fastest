# FASTEST Testing Framework

> ⚠️ **This README was generated with AI assistance. Some information may be inaccurate or incomplete. Refer to the source code and headers as the authoritative reference.**

A lightweight, macro-based C testing framework designed for simplicity and low overhead. Originally developed as a personal tool for the "Programmazione di sistemi embedded e multicore" (PEM) course at [La Sapienza University of Rome](https://www.uniroma1.it/).

FASTEST is intended for personal use — it is not a production-grade or widely adopted testing framework. That said, contributions and improvements are welcome if you find it useful.

## Features

- **5 Testing Modes**: Quick tests, quick delegate tests, custom tests, inline tests, and double-inline tests
- **Flexible Assertions**: Support for EQ, NEQ, GT, GE, LT, LE comparisons
- **Built-in Timing**: Measure test execution time in seconds, milliseconds, microseconds, or nanoseconds
- **Memory Tracking**: Track allocations and deallocations (Memory profiler coming soon)
- **Colored Output**: ANSI-colored terminal output for clear test results
- **Zero Dependencies**: Pure C implementation with minimal external requirements
- **Static Library**: Easy integration into existing projects

## Table of Contents

- [Project Structure](#project-structure)
- [Building the Library](#building-the-library)
- [Using FASTEST](#using-fastest)
- [Testing Modes](#testing-modes)
- [Flags Reference](#flags-reference)
- [Examples](#examples)
- [CI/CD Integration](#cicd-integration)
- [Contributing](#contributing)
- [License](#license)

## Project Structure

```
fastest/
├── include/
│   └── fastest/
│       ├── tests.h           # Core data structures and flags
│       ├── logging.h         # Output formatting and colors
│       ├── quick_tests.h     # FASTEST_QUICKTEST macros
│       └── custom_tests.h    # FASTEST_CUSTOMTEST macros
├── src/
│   └── logging.c            # Implementation of logging functions
├── build/
│   └── fastest.a            # Generated static library
├── obj/                     # Object files (generated)
├── Makefile
└── README.md
```

## Building the Library

### Prerequisites

- GCC compiler
- GNU Make
- Standard C library (C11 or later recommended)

### Build Instructions

1. **Clone the repository**:
   ```bash
   git clone https://github.com/DanieleSavino/fastest
   cd fastest
   ```

2. **Build the static library**:
   ```bash
   make
   ```
   This creates `build/fastest.a`

3. **Clean build artifacts**:
   ```bash
   make clean
   ```

4. **Debug build info**:
   ```bash
   make print
   ```

## Using FASTEST

### Integration Steps

1. **Copy the library and headers to your project**:
   ```bash
   cp build/fastest.a /path/to/your/project/lib/
   cp -r include/fastest /path/to/your/project/include/
   ```

2. **Compile your test file**:
   ```bash
   gcc -o test_program test.c -Iinclude -Llib -lfastest
   ```

3. **For debug output, compile with `-DDEBUG`**:
   ```bash
   gcc -DDEBUG -o test_program test.c -Iinclude -Llib -lfastest
   ```

### Basic Test File

```c
#include "fastest/quick_tests.h"
#include "fastest/custom_tests.h"
#include "fastest/tests.h"

int add(int a, int b) {
    return a + b;
}

int main(void) {
    FASTEST_QUICKTEST("Addition Test", add(2, 3) == 5,
                      FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS);
    return 0;
}
```

## Testing Modes

FASTEST provides five distinct testing modes to match your workflow:

### 1. QUICKTEST - Raw Expression Testing

**Best for**: Quick inline expression tests

**Syntax**:
```c
FASTEST_QUICKTEST(name, expression, flags)
```

**Example**:
```c
FASTEST_QUICKTEST("Addition", add(2, 3) == 5,
                  FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_MS);
```

**Characteristics**:
- Tests any boolean expression directly
- Most concise syntax for inline tests
- Runs when called in `main()`

---

### 2. QUICKTEST_DELEGATE - Direct Function Testing

**Best for**: Simple unit tests of pure functions with arguments

**Syntax**:
```c
FASTEST_QUICKTEST_DELEGATE(name, function, expected_value, flags, arg1, arg2, ...)
```

**Example**:
```c
FASTEST_QUICKTEST_DELEGATE("Addition", add, 5,
                           FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_MS,
                           2, 3);
```

**Characteristics**:
- Tests a function directly with provided arguments
- Automatically compares return value against expected result
- Runs when called in `main()`

---

### 3. CUSTOMTEST - Function-Based Testing

**Best for**: Complex tests requiring multiple assertions or setup/teardown

**Syntax**:
```c
FASTEST_CUSTOMTEST(name, test_function, callback_function)
```

**Example**:
```c
void my_test(FASTEST_TestOutput *out) {
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
}

int main(void) {
    FASTEST_CUSTOMTEST("Custom Test", my_test, NULL);
    return 0;
}
```

**Characteristics**:
- Test logic lives in a separate function
- Optional callback for post-test processing
- Manual invocation in `main()`

---

### 4. INLINE - Inline Test Body with Callback

**Best for**: Tests with reusable callback logic

**Syntax**:
```c
FASTEST_CUSTOMTEST_INLINE(name, callback_function, { test_body })
```

**Example**:
```c
void my_callback(FASTEST_TestOutput *out) {
    DEBUG_PRINTF("%s completed", out->test_name);
}

FASTEST_CUSTOMTEST_INLINE("Inline Test", my_callback, {
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})
```

**Characteristics**:
- Test body written inline
- Separate reusable callback
- Auto-runs before `main()` via `__attribute__((constructor))`

---

### 5. DINLINE - Double Inline (Test + Callback)

**Best for**: Fully self-contained tests

**Syntax**:
```c
FASTEST_CUSTOMTEST_DINLINE(name, { callback_body }, { test_body })
```

**Example**:
```c
FASTEST_CUSTOMTEST_DINLINE("Double Inline Test", {
    DEBUG_PRINTF("%s finished", out->test_name);
}, {
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})
```

**Characteristics**:
- Everything in one place
- Auto-runs before `main()` via `__attribute__((constructor))`

## Flags Reference

### Assertion Flags

| Flag | Description |
|------|-------------|
| `FASTEST_ASSERT_EQ` | Assert equal (==) |
| `FASTEST_ASSERT_NEQ` | Assert not equal (!=) |
| `FASTEST_ASSERT_GT` | Assert greater than (>) |
| `FASTEST_ASSERT_GE` | Assert greater or equal (>=) |
| `FASTEST_ASSERT_LT` | Assert less than (<) |
| `FASTEST_ASSERT_LE` | Assert less or equal (<=) |

### Failure Mode Flags

| Flag | Description |
|------|-------------|
| `FASTEST_FAIL_ERROR` | Report failures as errors (red) |
| `FASTEST_FAIL_WARNING` | Report failures as warnings (yellow) |
| `FASTEST_FAIL_LOG` | Report failures as logs (blue) |

### Timing Flags

| Flag | Description |
|------|-------------|
| `FASTEST_TIME_S` | Report time in seconds |
| `FASTEST_TIME_MS` | Report time in milliseconds |
| `FASTEST_TIME_US` | Report time in microseconds |
| `FASTEST_TIME_NS` | Report time in nanoseconds |

### Memory Tracking Flag

| Flag | Description |
|------|-------------|
| `FASTEST_MEM_TRACK` | Enable memory tracking (⚠️ Profiler in development) |

### Status Codes

| Status | Description |
|--------|-------------|
| `FASTEST_SUCCESS` | Test passed |
| `FASTEST_ERROR_ASSERT` | Assertion failed |
| `FASTEST_ERROR_MEMORY` | Memory leak detected |
| `FASTEST_ERROR_TIMEOUT` | Test exceeded time limit |
| `FASTEST_ERROR_EXCEPTION` | Exception/signal occurred |
| `FASTEST_ERROR_UNEXPECTED` | Unexpected result |

Combine flags with bitwise OR:
```c
FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_MS
```

## Output Examples

**Successful test**:
```
[SUCCESS] Addition Test
[LOG] Addition Test: [time: 123ns]
```

**Failed test**:
```
[ERROR] Division Test: ASSERT_EQ failed
```

**Memory leak**:
```
[ERROR] Memory Test: Memory leaks found: [alloc=100, dealloc=50]
```

## CI/CD Integration

FASTEST includes a GitHub Actions workflow for automated building and releasing.

### Triggering a Release

```bash
git tag v1.0.0
git push origin v1.0.0
```

This automatically builds the library, packages headers and the `.a` file, creates a GitHub Release, and uploads the zip as a release asset.

## Contributing

Contributions are welcome. FASTEST was built for personal use during a university course, so the bar for changes is pragmatic rather than strict — if it's useful and doesn't break existing behavior, it's probably fine.

A few guidelines:
- Keep it dependency-free and C11 compatible
- Don't break the existing macro API
- If adding a new testing mode or flag, add a usage example

Open an issue or PR on [GitHub](https://github.com/DanieleSavino/fastest).

## Roadmap

- **Memory Profiler**: Automatic leak detection (in development)
- Additional assertion types
- JSON test result export

## License

MIT License

Copyright (c) 2025 Daniele Savino

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
