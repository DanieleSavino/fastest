#include "quick_tests.h"
#include "tests.h"

int add(int a, int b) {
    return a + b;
}

int main(int argc, char *argv[])
{
    FASTEST_TestOutput out = FASTEST_QUICKTEST("Addition", add, 5, FASTEST_FAIL_WARNING | FASTEST_TIME_NS, 2, 3);

    return 0;
}
