# FASTEST Testing Framework

A lightweight, macro-based C testing framework designed for simplicity, flexibility, and performance. FASTEST provides multiple testing modes to suit different use cases, from quick function tests to complex custom test scenarios.

## Features

- **4 Testing Modes**: Quick tests, custom tests, inline tests, and double-inline tests
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
- [License](#license)

## Project Structure

```
fastest/
├── include/
│   └── fastest/
│       ├── tests.h           # Core data structures and flags
│       ├── logging.h         # Output formatting and colors
│       ├── quick_tests.h     # FASTEST_QUICKTEST macro
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
   git clone <repository-url>
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
    FASTEST_QUICKTEST("Addition Test", add, 5, 
                      FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS, 
                      2, 3);
    return 0;
}
```

## Testing Modes

FASTEST provides four distinct testing modes to match your workflow:

### 1. QUICKTEST - Direct Function Testing

**Best for**: Simple unit tests of pure functions

**Syntax**:
```c
FASTEST_QUICKTEST(name, function, expected_value, flags, arg1, arg2, ...)
```

**Example**:
```c
FASTEST_QUICKTEST("Addition", add, 5, 
                  FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_MS, 
                  2, 3);
```

**Characteristics**:
- Tests function directly with provided arguments
- Automatically compares return value against expected result
- Most concise syntax
- Runs when called in `main()`

---

### 2. CUSTOMTEST - Function-Based Testing

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
- Test logic in separate function
- Optional callback for post-test processing
- Good code organization
- Manual invocation in `main()`

---

### 3. INLINE - Inline Test Body with Callback

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
- Separate callback function
- Auto-runs before `main()` using `__attribute__((constructor))`
- Good when callback is shared across tests

---

### 4. DINLINE - Double Inline (Test + Callback)

**Best for**: Self-contained tests with simple, test-specific callbacks

**Syntax**:
```c
FASTEST_CUSTOMTEST_DINLINE(name, { callback_body }, { test_body })
```

**Example**:
```c
FASTEST_CUSTOMTEST_DINLINE("Double Inline Test", {
    // Callback body
    DEBUG_PRINTF("%s finished", out->test_name);
}, {
    // Test body
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})
```

**Characteristics**:
- Most compact form
- Everything defined in one place
- Auto-runs before `main()` using `__attribute__((constructor))`
- Ideal for self-contained tests

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

**Combining Flags**: Use bitwise OR (`|`) to combine multiple flags:
```c
FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_MS
```

## Examples

### Complete Test Suite Example

```c
#include "fastest/quick_tests.h"
#include "fastest/custom_tests.h"
#include "fastest/tests.h"

// Functions to test
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

// Callback for post-test processing
void test_callback(FASTEST_TestOutput *out) {
    DEBUG_PRINTF("Test '%s' completed in %luns", out->test_name, out->time_ns);
}

// Custom test function
void division_test(FASTEST_TestOutput *out) {
    out->test_flags |= FASTEST_ASSERT_EQ | FASTEST_TIME_US;
    int result = 10 / 2;
    out->exit_status |= result == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
}

// Inline test (runs before main)
FASTEST_CUSTOMTEST_INLINE("Subtraction Test", test_callback, {
    out->test_flags |= FASTEST_ASSERT_EQ | FASTEST_TIME_NS;
    out->exit_status |= (10 - 3 == 7) ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})

// Double inline test (runs before main)
FASTEST_CUSTOMTEST_DINLINE("Modulo Test", {
    DEBUG_PRINTF("Test completed");
}, {
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= (10 % 3 == 1) ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})

int main(void) {
    // Quick tests (simple function calls)
    FASTEST_QUICKTEST("Add Positive", add, 8, 
                      FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS, 
                      5, 3);
    
    FASTEST_QUICKTEST("Multiply", multiply, 20, 
                      FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_US, 
                      4, 5);
    
    FASTEST_QUICKTEST("Greater Than", add, 5, 
                      FASTEST_ASSERT_GT | FASTEST_FAIL_WARNING, 
                      2, 3);
    
    // Custom test with callback
    FASTEST_CUSTOMTEST("Division", division_test, test_callback);
    
    return 0;
}
```

### Memory Tracking Example

> ⚠️ **Note**: The memory profiler is currently under development. Manual tracking is shown below as a placeholder for the upcoming automatic profiling feature.

```c
#include "fastest/custom_tests.h"
#include "fastest/tests.h"
#include <stdlib.h>

void memory_test(FASTEST_TestOutput *out) {
    out->test_flags |= FASTEST_MEM_TRACK;
    
    // Manual memory tracking (automatic profiling coming soon)
    void *ptr = malloc(100);
    out->allocation += 100;
    
    free(ptr);
    out->deallocation += 100;
    
    out->exit_status |= FASTEST_SUCCESS | FASTEST_DEFAULT_LOG;
}

int main(void) {
    FASTEST_CUSTOMTEST("Memory Leak Test", memory_test, NULL);
    return 0;
}
```

## CI/CD Integration

FASTEST includes GitHub Actions workflow for automated building and releasing.

### Workflow Features

- **Automatic builds** on push to `main` or `develop` branches
- **Pull request validation**
- **Automated releases** when tags are pushed
- **Release artifacts** packaged as ZIP files

### Triggering a Release

```bash
git tag v1.0.0
git push origin v1.0.0
```

This automatically:
1. Builds the library
2. Packages headers and library files
3. Creates a GitHub Release
4. Uploads `fastest-v1.0.0.zip` as a release asset

### Manual Workflow Trigger

The workflow can also be triggered manually from the GitHub Actions tab.

## Output Examples

**Successful Test**:
```
[SUCCESS] Addition Test
[LOG] Addition Test: [time: 123ns]
```

**Failed Test**:
```
[ERROR] Division Test: ASSERT_EQ failed
```

**Memory Leak Detection**:
```
[ERROR] Memory Test: Memory leaks found: [alloc=100, dealloc=50]
```

## Best Practices

1. **Choose the right testing mode**:
   - Use QUICKTEST for simple function tests
   - Use CUSTOMTEST for complex test logic
   - Use INLINE/DINLINE for tests that should run automatically

2. **Always enable logging**:
   ```c
   out->exit_status |= FASTEST_DEFAULT_LOG;
   ```

3. **Combine timing with tests** to identify performance issues

4. **Use appropriate failure modes**:
   - `FASTEST_FAIL_ERROR` for critical failures
   - `FASTEST_FAIL_WARNING` for non-critical issues
   - `FASTEST_FAIL_LOG` for informational purposes

5. **Enable DEBUG mode** during development for verbose output

## Contributing

Contributions are welcome! Please ensure:
- Code follows existing style conventions
- All tests pass
- New features include documentation

## Roadmap

### Upcoming Features

- **Memory Profiler**: Automatic memory allocation/deallocation tracking and leak detection (Coming Soon)
- Additional assertion types
- JSON test result export
- Test suite organization and filtering

## License

MIT License

Copyright (c) 2025 [Your Name/Organization]

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

## Support

For issues, questions, or contributions, please [open an issue](link-to-issues) on GitHub.
