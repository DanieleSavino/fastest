from __future__ import annotations

import importlib
import math
import os
import json
from datetime import datetime, timezone
from typing import Any

from . import flags
from .logging import color

# ── Stats ─────────────────────────────────────────────────────────────────────

class Stats:
    """Descriptive statistics over a list of nanosecond samples.

    `samples` holds one headline value (`time_ns`) per repeat, so mean / stddev /
    min / max / median are statistics ACROSS REPEATS (stddev is 0 for a single
    repeat).

    `iter_avg_samples` / `iter_stddev_samples` are optional: one value per repeat
    of the test's own inner-sample mean / population stddev (`avg_ns` /
    `stddev_ns` in the backend dict, e.g. over the timed iterations of one
    launch). They are kept per repeat and also combined into `iter_avg` /
    `iter_stddev`, the mean / population stddev of ALL inner samples of all
    repeats (law of total variance, weighted by `iter_n_samples`, the inner
    sample count of each repeat); `iter_n` is their total. All are None when the
    backend did not provide them.
    """

    def __init__(
        self,
        samples: list[int],
        exit_statuses: list[int] | None = None,
        iter_avg_samples: list[float] | None = None,
        iter_stddev_samples: list[float] | None = None,
        iter_n_samples: list[int] | None = None,
    ):
        self.samples = samples
        n = len(samples)
        self.mean   = sum(samples) / n if n else 0.0
        variance    = sum((x - self.mean) ** 2 for x in samples) / n if n > 1 else 0.0
        self.stddev = math.sqrt(variance)
        self.min    = min(samples) if samples else 0
        self.max    = max(samples) if samples else 0
        if not samples:
            self.median = 0
        else:
            srt = sorted(samples)
            self.median = srt[n // 2] if n % 2 else (srt[n // 2 - 1] + srt[n // 2]) / 2

        # Keep track of exit status (use the last run's as representative)
        self.last_exit_status = exit_statuses[-1] if exit_statuses else 0

        # Inner-sample (per-iteration) statistics, if the backend provided them.
        self.iter_avg_samples: list[float] | None = None
        self.iter_stddev_samples: list[float] | None = None
        self.iter_avg: float | None = None
        self.iter_stddev: float | None = None
        self.iter_n_samples: list[int] | None = None
        self.iter_n: int | None = None
        if (iter_avg_samples is not None and iter_stddev_samples is not None
                and iter_n_samples is not None and n
                and len(iter_avg_samples) == n and len(iter_stddev_samples) == n
                and len(iter_n_samples) == n and sum(iter_n_samples) > 0):
            self.iter_avg_samples = list(iter_avg_samples)
            self.iter_stddev_samples = list(iter_stddev_samples)
            self.iter_n_samples = list(iter_n_samples)
            total = sum(iter_n_samples)
            self.iter_n = total
            self.iter_avg = sum(w * a for w, a in zip(iter_n_samples, iter_avg_samples)) / total
            self.iter_stddev = math.sqrt(sum(
                w * (sd ** 2 + (a - self.iter_avg) ** 2)
                for w, a, sd in zip(iter_n_samples, iter_avg_samples, iter_stddev_samples)
            ) / total)

    def to_dict(self) -> dict[str, Any]:
        return {
            "samples_ns": self.samples,
            "mean_ns":    self.mean,
            "stddev_ns":  self.stddev,
            "min_ns":     self.min,
            "max_ns":     self.max,
            "median_ns":  self.median,
            "iter_avg_ns":          self.iter_avg,
            "iter_stddev_ns":       self.iter_stddev,
            "iter_avg_samples_ns":  self.iter_avg_samples,
            "iter_stddev_samples_ns": self.iter_stddev_samples,
            "iter_n":               self.iter_n,
            "iter_n_samples":       self.iter_n_samples,
        }


# ── Pool ──────────────────────────────────────────────────────────────────────

class Pool:
    """A named collection of test names to run together."""

    def __init__(self, name: str, tests: list[str]):
        if not tests:
            raise ValueError("Pool must contain at least one test name.")
        self.name  = name
        self.tests = list(tests)

    def __repr__(self) -> str:
        return f"Pool({self.name!r}, {self.tests})"


# ── CompareResult ─────────────────────────────────────────────────────────────

class CompareResult:
    def __init__(self, pools: list[Pool], n_repeats: int, data: dict[str, dict[str, Stats]]):
        self.pools     = pools
        self.n_repeats = n_repeats
        self.data      = data
        self.timestamp = datetime.now(timezone.utc).isoformat()

        # Pre‑compute cross‑comparisons table (used by report / save)
        self.comparisons = self._build_comparisons()

    def _build_comparisons(self) -> list[dict[str, Any]]:
        comparisons: list[dict[str, Any]] = []
        if len(self.pools) < 2:
            return comparisons

        min_len = min(len(p.tests) for p in self.pools)
        for i in range(min_len):
            entry: dict[str, Any] = {"index": i, "pairs": []}
            baseline_mean = self.data[self.pools[0].name][self.pools[0].tests[i]].mean
            for pool in self.pools:
                test  = pool.tests[i]
                stats = self.data[pool.name][test]
                ratio = stats.mean / baseline_mean if baseline_mean else None
                entry["pairs"].append({
                    "pool":  pool.name,
                    "test":  test,
                    "mean_ns": stats.mean,
                    "ratio_vs_first": ratio,
                })
            comparisons.append(entry)
        return comparisons

    # ── Serialisation ─────────────────────────────────────────────────────────

    def to_dict(self) -> dict[str, Any]:
        results: dict[str, Any] = {}
        for pool in self.pools:
            results[pool.name] = {
                test: self.data[pool.name][test].to_dict()
                for test in pool.tests
            }
        return {
            "meta": {
                "n_repeats": self.n_repeats,
                "timestamp": self.timestamp,
                "pools":     [p.name for p in self.pools],
            },
            "results":     results,
            "comparisons": self.comparisons,
        }

    def save(self, path: str) -> None:
        with open(path, "w") as f:
            json.dump(self.to_dict(), f, indent=2)

    def save_csv(self, path: str) -> None:
        import csv
        with open(path, "w", newline="") as f:
            writer = csv.writer(f)
            # Header
            writer.writerow([
                "pool", "test",
                "mean_ns", "stddev_ns", "min_ns", "max_ns", "median_ns",
                "samples_ns",
                "iter_avg_ns", "iter_stddev_ns",
                "iter_avg_samples_ns", "iter_stddev_samples_ns",
                "iter_n",
            ])
            for pool in self.pools:
                for test in pool.tests:
                    s = self.data[pool.name][test]
                    writer.writerow([
                        pool.name, test,
                        s.mean, s.stddev, s.min, s.max, s.median,
                        json.dumps(s.samples),  # serialize list as JSON string
                        "" if s.iter_avg is None else s.iter_avg,
                        "" if s.iter_stddev is None else s.iter_stddev,
                        "" if s.iter_avg_samples is None else json.dumps(s.iter_avg_samples),
                        "" if s.iter_stddev_samples is None else json.dumps(s.iter_stddev_samples),
                        "" if s.iter_n is None else s.iter_n,
                    ])

    # ── Report ────────────────────────────────────────────────────────────────

    def report(self) -> None:
        _report(self)


# ── Runner ────────────────────────────────────────────────────────────────────

class Runner:
    def __init__(self, backend=None):
        self._backend = None
        self.verbose  = False

        if backend is not None:
            self.set_backend(backend)
        elif os.environ.get("FASTEST_PROJECT"):
            self._auto_detect_backend()

    def _auto_detect_backend(self) -> None:
        project = os.environ["FASTEST_PROJECT"]
        mod = importlib.import_module(project)
        self.set_backend(mod)

    def set_backend(self, module) -> None:
        required = ["get_tests", "get_test", "run_test", "tests"]
        missing  = [a for a in required if not hasattr(module, a)]
        if missing:
            raise TypeError(f"Module missing required fastest interface: {missing}")
        self._backend = module

    @property
    def backend(self):
        if self._backend is None:
            raise RuntimeError("No backend set. Pass one to Runner() or set FASTEST_PROJECT.")
        return self._backend

    @property
    def is_ready(self) -> bool:
        return self._backend is not None

    # ── Backend passthrough ───────────────────────────────────────────────────

    def get_tests(self) -> list[dict]:
        return self.backend.get_tests()

    def get_test(self, name: str) -> dict:
        return self.backend.get_test(name)

    def run_test(self, name: str) -> None:
        self.backend.run_test(name)

    def run_log(self, name: str) -> dict:
        self.run_test(name)
        result = self.get_test(name)
        print(flags.strtest(result))
        return result

    def run_all(self) -> None:
        for t in self.get_tests():
            if self.verbose:
                print(f"  running {t['test_name']}")
            self.run_test(t["test_name"])

    def run_log_all(self, tests=None) -> None:
        if(tests is None):
            tests = self.get_tests()

        for t in tests:
            self.run_log(t["test_name"])

    # ── Pool constructors ─────────────────────────────────────────────────────

    def pool(self, name: str, *test_names: str) -> Pool:
        """Create a pool from an explicit list of test names."""
        return Pool(name, list(test_names))

    def pool_from_prefix(self, prefix: str) -> Pool:
        """Create a pool from all tests whose names start with `prefix/`."""
        subtests = self.backend.get_subtests(prefix)
        if not subtests:
            raise ValueError(f"No tests found with prefix '{prefix}/'")
        names = [t["test_name"] for t in subtests]
        return Pool(prefix, names)

    # ── Compare ───────────────────────────────────────────────────────────────

    def compare(self, *pools: Pool, n_repeats: int = 5) -> CompareResult:
        """Run every test in every pool n_repeats times and collect timing."""
        if not pools:
            raise ValueError("Provide at least one pool.")

        data: dict[str, dict[str, Stats]] = {}

        for pool in pools:
            data[pool.name] = {}
            for test_name in pool.tests:
                samples: list[int] = []
                statuses: list[int] = []
                avgs: list[float | None] = []
                stddevs: list[float | None] = []
                ns: list[int | None] = []
                for _ in range(n_repeats):
                    self.run_test(test_name)
                    result = self.get_test(test_name)
                    samples.append(result["time_ns"])
                    statuses.append(result.get("exit_status", 0))
                    # absent on backends built before avg_ns/stddev_ns existed
                    avgs.append(result.get("avg_ns"))
                    stddevs.append(result.get("stddev_ns"))
                    ns.append(result.get("n_iters"))
                    if self.verbose:
                        print(flags.strtest(result))
                have_iter = all(v is not None for v in avgs + stddevs + ns)
                data[pool.name][test_name] = Stats(
                    samples, statuses,
                    avgs if have_iter else None,
                    stddevs if have_iter else None,
                    ns if have_iter else None,
                )

        return CompareResult(list(pools), n_repeats, data)


# ── Report renderer ───────────────────────────────────────────────────────────

_NS_THRESHOLDS = [
    (1_000_000_000, "s",  1_000_000_000),
    (1_000_000,     "ms", 1_000_000),
    (1_000,         "µs", 1_000),
    (1,             "ns", 1),
]

def _fmt_ns(ns: float) -> str:
    for threshold, unit, div in _NS_THRESHOLDS:
        if ns >= threshold:
            return f"{ns / div:>8.2f} {unit}"
    return f"{ns:>8.2f} ns"


def _fmt_opt_ns(ns: float | None) -> str:
    return f"{'-':>11}" if ns is None else _fmt_ns(ns)


def _bar(value: float, max_value: float, width: int = 20) -> str:
    filled = int(round(value / max_value * width)) if max_value else 0
    return "█" * filled + "░" * (width - filled)


def _report(result: CompareResult) -> None:
    print()
    print("─" * 70)
    print(f"  fastest compare  │  {result.n_repeats} repetitions  │  {result.timestamp[:19]}")
    print("─" * 70)

    for pool in result.pools:
        pool_data = result.data[pool.name]
        all_means = [pool_data[t].mean for t in pool.tests]
        max_mean  = max(all_means) if all_means else 1.0

        print(f"\n  pool: {color(pool.name, 'bold')}")
        print(f"  {'test':<35} {'':2} {'mean':>12}  {'stddev':>10}  {'min':>10}  {'max':>10}  "
              f"{'iter avg':>11}  {'iter sd':>11}  status")
        print(f"  {'─'*35} {'─'*2} {'─'*12}  {'─'*10}  {'─'*10}  {'─'*10}  "
              f"{'─'*11}  {'─'*11}  {'─'*20}")

        for test_name in pool.tests:
            s    = pool_data[test_name]
            bar  = _bar(s.mean, max_mean)
            name = test_name if len(test_name) <= 35 else "…" + test_name[-34:]

            status_code = s.last_exit_status
            sym  = flags.symbol(status_code)
            desc = flags.strexit(status_code, 0)

            print(
                f"  {name:<35} {sym:<4} {_fmt_ns(s.mean)}  "
                f"{_fmt_ns(s.stddev)}  {_fmt_ns(s.min)}  {_fmt_ns(s.max)}  "
                f"{_fmt_opt_ns(s.iter_avg)}  {_fmt_opt_ns(s.iter_stddev)}  {desc}"
            )
            # Print the bar below (it stays uncoloured, or you can colour it too)
            print(f"  {'':37} {color(bar, 'blue')}")

    # Cross‑comparison section with colour hints
    if result.comparisons:
        print(f"\n  {'─'*70}")
        print("  paired comparison (ratio vs first pool)")
        print(f"  {'─'*70}")
        for entry in result.comparisons:
            print(f"\n  index {entry['index']}")
            for pair in entry["pairs"]:
                ratio_str = (
                    f"{pair['ratio_vs_first']:.3f}x"
                    if pair["ratio_vs_first"] is not None
                    else "  base"
                )
                # Dim the first pool’s entry (the baseline)
                if pair["pool"] == result.pools[0].name:
                    line = f"    {color(pair['pool'] + ' (base)', 'dim'):<25} {pair['test']:<30} {_fmt_ns(pair['mean_ns'])}  {ratio_str}"
                else:
                    line = f"    {pair['pool']:<25} {pair['test']:<30} {_fmt_ns(pair['mean_ns'])}  {ratio_str}"
                print(line)

    print()
    print("─" * 70)
    print()
