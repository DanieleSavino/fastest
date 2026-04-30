#!/usr/bin/env python3

import showcase as t

target = t.tests.custom_test

print(t.get_tests())

t.run_test(target)

print(t.get_test(target))
