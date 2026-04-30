from __future__ import annotations

import importlib
import math
import os
import json
from datetime import datetime, timezone
from typing import Any

from . import flags
#
# -- ANSI colour helpers --------------------------------------------------------
_ANSI = {
    "reset":   "\033[0m",
    "green":   "\033[32m",
    "red":     "\033[31m",
    "yellow":  "\033[33m",
    "blue":    "\033[34m",
    "magenta": "\033[35m",
    "cyan":    "\033[36m",
    "dim":     "\033[2m",
    "bold":    "\033[1m",
}

def _c(text: str, colour: str) -> str:
    """Wrap text in ANSI colour codes (no-op if colour unknown)."""
    code = _ANSI.get(colour, "")
    return f"{code}{text}{_ANSI['reset']}" if code else text

# ── Stats ─────────────────────────────────────────────────────────────────────

class Stats:
    """Descriptive statistics over a list of nanosecond samples."""

    def __init__(self, samples: list[int], exit_statuses: list[int] | None = None):
        self.samples = samples
        n = len(samples)
        self.mean   = sum(samples) / n if n else 0.0
        variance    = sum((x - self.mean) ** 2 for x in samples) / n if n > 1 else 0.0
        self.stddev = math.sqrt(variance)
        self.min    = min(samples) if samples else 0
        self.max    = max(samples) if samples else 0
        self.median = sorted(samples)[n // 2] if samples else 0

        # Keep track of exit status (use the last run's as representative)
        self.last_exit_status = exit_statuses[-1] if exit_statuses else 0

    def to_dict(self) -> dict[str, Any]:
        return {
            "samples_ns": self.samples,
            "mean_ns":    self.mean,
            "stddev_ns":  self.stddev,
            "min_ns":     self.min,
            "max_ns":     self.max,
            "median_ns":  self.median,
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

    def run_all(self) -> None:
        for t in self.get_tests():
            if self.verbose:
                print(f"  running {t['test_name']}")
            self.run_test(t["test_name"])

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
                for _ in range(n_repeats):
                    self.run_test(test_name)
                    result = self.get_test(test_name)
                    samples.append(result["time_ns"])
                    statuses.append(result.get("exit_status", 0))
                    if self.verbose:
                        print(flags.strtest(result))
                data[pool.name][test_name] = Stats(samples, statuses)

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

        print(f"\n  pool: {_c(pool.name, 'bold')}")
        print(f"  {'test':<35} {'':2} {'mean':>12}  {'stddev':>10}  {'min':>10}  {'max':>10}  status")
        print(f"  {'─'*35} {'─'*2} {'─'*12}  {'─'*10}  {'─'*10}  {'─'*10}  {'─'*20}")

        for test_name in pool.tests:
            s    = pool_data[test_name]
            bar  = _bar(s.mean, max_mean)
            name = test_name if len(test_name) <= 35 else "…" + test_name[-34:]

            status_code = s.last_exit_status
            sym  = flags.symbol(status_code)
            desc = flags.strexit(status_code, 0)

            # Colour the symbol based on result
            if flags.passed(status_code):
                col_sym = _c(sym, "green")
            elif status_code & flags.SKIPPED:
                col_sym = _c(sym, "dim")
            elif status_code & flags.INCOMPLETE:
                col_sym = _c(sym, "yellow")
            else:
                col_sym = _c(sym, "red")

            # Colour the failure description red
            col_desc = _c(desc, "green" if flags.passed(status_code) else "red")

            print(
                f"  {name:<35} {col_sym:<4} {_fmt_ns(s.mean)}  "
                f"{_fmt_ns(s.stddev)}  {_fmt_ns(s.min)}  {_fmt_ns(s.max)}  {col_desc}"
            )
            # Print the bar below (it stays uncoloured, or you can colour it too)
            print(f"  {'':37} {_c(bar, 'blue')}")

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
                    line = f"    {_c(pair['pool'] + ' (base)', 'dim'):<25} {pair['test']:<30} {_fmt_ns(pair['mean_ns'])}  {ratio_str}"
                else:
                    line = f"    {pair['pool']:<25} {pair['test']:<30} {_fmt_ns(pair['mean_ns'])}  {ratio_str}"
                print(line)

    print()
    print("─" * 70)
    print()
