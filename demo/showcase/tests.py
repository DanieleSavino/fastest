#!/usr/bin/env python3

import fastest

pool1 = fastest.pool("pool1", fastest.tests.Dinline, fastest.tests.custom_test)
pool2 = fastest.pool("pool2", fastest.tests.inline, fastest.tests.custom_test)

fastest.compare(pool1, pool2, n_repeats=5).report()
