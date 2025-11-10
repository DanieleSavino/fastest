#include "quick_tests.h"
#include "tests.h"

int add(int a, int b) {
    return a + b;
}

int main(int argc, char *argv[])
{
    FASTEST_TestOutput out = FASTEST_QUICKTEST("Addition", add, 6, FASTEST_ASSERT_EQ | FASTEST_FAIL_ERROR | FASTEST_TIME_NS, 2, 3);

    return 0;
}
