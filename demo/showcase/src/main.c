#include "fastest/quick_tests.h"
#include "fastest/custom_tests.h"
#include "fastest/tests.h"
#include <stdio.h>

// Simple function to test
int add(int a, int b) {
    return a + b;
}

// Callback function for inline mode - executed after test body completes
void inline_callback(FASTEST_TestOutput *out) {
    (void)(out);
    DEBUG_PRINTF("%s executed mem err: %d", out->test_name, (out->exit_status & FASTEST_ERROR_ASSERT) > 0L);
}

/*
 * MODE 3: INLINE MODE (FASTEST_CUSTOMTEST_INLINE)
 * - Combines test body and callback in a single macro
 * - Test body runs inline, then callback is invoked
 * - Uses __attribute__((constructor)) to run automatically before main()
 * - Useful for tests that need post-execution processing with a separate callback function
 */
FASTEST_CUSTOMTEST_INLINE("inline", inline_callback, {
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})

/*
 * MODE 4: DOUBLE INLINE MODE (FASTEST_CUSTOMTEST_DINLINE)
 * - Both test body AND callback body are defined inline
 * - Most compact form - everything in one macro invocation
 * - Uses __attribute__((constructor)) to run automatically before main()
 * - Ideal for self-contained tests where callback logic is simple and specific to this test
 */
FASTEST_CUSTOMTEST_DINLINE("Dinline", {
    // Callback body - runs after test and timing
    DEBUG_PRINTF("%s executed mem err: %d", out->test_name, (out->exit_status & FASTEST_ERROR_ASSERT) > 0L);
}, {
    // Test body - runs with timing
    out->test_flags |= FASTEST_ASSERT_EQ;
    out->exit_status |= add(2, 3) == 5 ? FASTEST_SUCCESS : FASTEST_ERROR_ASSERT;
    out->exit_status |= FASTEST_DEFAULT_LOG;
})

/*
 * Test function for custom mode
 * Traditional function-based test - receives test output structure as parameter
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
FASTEST_CUSTOMTEST("custom", test, NULL);

int main(void)
{
    /*
     * MODE 1: QUICK TEST MODE (FASTEST_QUICKTEST)
     * - Simplest form - tests a function directly with arguments
     * - Automatically compares return value against expected result
     * - Syntax: FASTEST_QUICKTEST(name, function, expected_value, flags, arg1, arg2, ...)
     * - Best for simple unit tests of pure functions
     */
    FASTEST_QUICKTEST_DELEGATE("Addition", add, 6, (FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS), 2, 3);

    FASTEST_QUICKTEST("Addition", add(2, 3) == 6, (FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS));

    FASTEST_list_t *list;
    FASTEST_list_getInstance(&list);
    FASTEST_list_exec(list, 0);
    FASTEST_list_exec(list, 1);
    FASTEST_list_exec(list, 2);
    // FASTEST_list_pprint(list);


    return 0;
}

/*
 * FASTEST FRAMEWORK - 4 TESTING MODES SUMMARY:
 * 
 * 1. QUICKTEST: Direct function testing with arguments
 *    - Simplest, most concise
 *    - Limited to testing single function calls
 * 
 * 2. CUSTOMTEST: Function-based testing with optional callback
 *    - Traditional approach, good code organization
 *    - Test logic in separate function
 *    - Runs in main() - manual invocation
 * 
 * 3. INLINE: Inline test body with separate callback function
 *    - Test logic inline, callback separate
 *    - Auto-runs before main()
 *    - Good when callback is reused across tests
 * 
 * 4. DINLINE: Fully inline test body and callback
 *    - Everything in one place
 *    - Auto-runs before main()
 *    - Most compact for self-contained tests
 */
