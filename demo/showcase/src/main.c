#include "fastest/custom_tests.h"
#include "fastest/quick_tests.h"
#include "fastest/tests.h"
#include <stdio.h>

// Simple function to test
int add(int a, int b) { return a + b; }

// Callback function for inline mode - executed after test body completes
void inline_callback(FASTEST_TestOutput *out) {
  (void)(out);
  DEBUG_PRINTF("%s executed mem err: %d", out->test_name,
               (out->exit_status & FASTEST_ERROR_ASSERT) > 0L);
}

/*
 * MODE 4: INLINE MODE (FASTEST_CUSTOMTEST_INLINE)
 * - Combines test body and callback in a single macro
 * - Test body runs inline, then callback is invoked
 * - Uses __attribute__((constructor)) to run automatically before main()
 * - Useful for tests that need post-execution processing with a separate
 * callback function
 */
FASTEST_CUSTOMTEST_INLINE("inline", FASTEST_TIME_NS | FASTEST_FAIL_ERROR,
    inline_callback,
    {
        out->test_flags |= FASTEST_ASSERT_EQ;
        out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
        out->exit_status |= FASTEST_DEFAULT_LOG;
    }
)

/*
 * MODE 5: DOUBLE INLINE MODE (FASTEST_CUSTOMTEST_DINLINE)
 * - Both test body AND callback body are defined inline
 * - Most compact form - everything in one macro invocation
 * - Uses __attribute__((constructor)) to run automatically before main()
 * - Ideal for self-contained tests where callback logic is simple and specific
 * to this test
 */
FASTEST_CUSTOMTEST_DINLINE("Dinline", FASTEST_TIME_NS | FASTEST_FAIL_ERROR | FASTEST_DEFAULT_LOG,
    {
        // Callback body - runs after test and timing
        DEBUG_PRINTF("%s executed mem err: %d", out->test_name, (out->exit_status & FASTEST_ERROR_ASSERT) > 0L);
    }, {
        // Test body - runs with timing
        out->test_flags |= FASTEST_ASSERT_EQ;
        out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
        out->exit_status |= FASTEST_DEFAULT_LOG;
    }
)

/*
 * Test function for custom mode
 * MODE 3: CUSTOM TEST MODE (FASTEST_CUSTOMTEST)
 * - Uses a separate test function that you define
 * - Provides more control over test logic and assertions
 * - Optional callback parameter for post-test processing (NULL here)
 * - Good for complex tests that need multiple assertions or setup/teardown
 */
void test(FASTEST_TestOutput *out) {
  out->test_flags |= FASTEST_ASSERT_EQ;
  out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
  out->exit_status |= FASTEST_DEFAULT_LOG;
}

/*
* MODE 2: CUSTOM TEST MODE (FASTEST_CUSTOMTEST)
* - Uses a separate test function that you define
* - Provides more control over test logic and assertions
* - Optional callback parameter for post-test processing (NULL here)
* - Good for complex tests that need multiple assertions or setup/teardown
*/
FASTEST_CUSTOMTEST("custom test", FASTEST_TIME_NS | FASTEST_FAIL_ERROR, test, NULL);

int main(void)
{
    /*
     * MODE 1: QUICK TEST MODE (FASTEST_QUICKTEST)
     * - Simplest form - tests a function directly with arguments
     * - Automatically compares return value against expected result
     * - Syntax: FASTEST_QUICKTEST(name, function, expected_value, flags, arg1, arg2, ...)
     * - Best for simple unit tests of pure functions
     */
    FASTEST_QUICKTEST("Addition", add(2, 3) == 6, (FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS));



  return 0;
}

/*
 * FASTEST FRAMEWORK - 5 TESTING MODES SUMMARY:
 *
 * 1. QUICKTEST DELEGATE: Direct function call with arguments
 * 2. QUICKTEST (expression): Boolean expression evaluated inline
 * 3. CUSTOMTEST: Function-based testing with optional callback
 * 4. INLINE: Inline test body with separate callback function
 * 5. DOUBLE INLINE: Fully inline test body and callback
 */
