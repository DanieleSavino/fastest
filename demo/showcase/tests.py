#!/usr/bin/env python3

import fastest

pool1 = fastest.pool("pool1", *([fastest.tests.Dinline, fastest.tests.custom_test] * 10))
pool2 = fastest.pool("pool2", *([fastest.tests.inline, fastest.tests.custom_test] * 10))

cmp = fastest.compare(pool1, pool2, n_repeats=5)
cmp.report()

fastest.default_runner.run_log_all()
