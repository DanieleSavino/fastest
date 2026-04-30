"""
fastest.flags
─────────────
Python mirror of fastest/tests.h bitmask constants, plus
strerror-style helpers for exit_status and test_flags.
"""

from __future__ import annotations

from .logging import color

# ── Test flags ────────────────────────────────────────────────────────────────

ASSERT_EQ  = 1 << 0
ASSERT_NEQ = 1 << 1
ASSERT_GT  = 1 << 2
ASSERT_GE  = 1 << 3
ASSERT_LT  = 1 << 4
ASSERT_LE  = 1 << 5

FAIL_ERROR   = 1 << 6
FAIL_WARNING = 1 << 7
FAIL_LOG     = 1 << 8

TIME_S  = 1 << 9
TIME_MS = 1 << 10
TIME_US = 1 << 11
TIME_NS = 1 << 12

MEM_TRACK    = 1 << 13
DEFAULT_LOG  = 1 << 21

# ── Exit status ───────────────────────────────────────────────────────────────

SUCCESS    = 1 << 0
SKIPPED    = 1 << 1
INCOMPLETE = 1 << 2

ERROR_ASSERT      = 1 << 8
ERROR_UNEXPECTED  = 1 << 9
ERROR_EXCEPTION   = 1 << 10
ERROR_MEMORY      = 1 << 11
ERROR_TIMEOUT     = 1 << 12
ERROR_RESOURCE    = 1 << 13
ERROR_MPI         = 1 << 14
ERROR_OMP         = 1 << 15
ERROR_CUDA        = 1 << 16
ERROR_INTERNAL    = 1 << 17
ERROR_UNKNOWN     = 1 << 18
ERROR_COLLISION   = 1 << 19
ERROR_NOT_FOUND   = 1 << 20

# ── Helpers ───────────────────────────────────────────────────────────────────

def strassert(flags: int) -> str:
    if flags & ASSERT_EQ:  return "ASSERT_EQ"
    if flags & ASSERT_NEQ: return "ASSERT_NEQ"
    if flags & ASSERT_GT:  return "ASSERT_GT"
    if flags & ASSERT_GE:  return "ASSERT_GE"
    if flags & ASSERT_LT:  return "ASSERT_LT"
    if flags & ASSERT_LE:  return "ASSERT_LE"
    return ""

def strtime(flags: int) -> str:
    if flags & TIME_NS: return "ns"
    if flags & TIME_US: return "µs"
    if flags & TIME_MS: return "ms"
    if flags & TIME_S:  return "s"
    return "ns"

def strexit(exit_status: int, flags: int = 0) -> str:
    if exit_status & SUCCESS:
        return color("SUCCESS", "green")
    if exit_status & SKIPPED:
        return color("SKIPPED", "yellow")
    if exit_status & INCOMPLETE:
        return color("INCOMPLETE", "blue")
    if exit_status & ERROR_ASSERT:
        detail = strassert(flags)
        return color(f"{detail} failed" if detail else "ASSERT failed", "red")
    _errors = [
        (ERROR_UNEXPECTED, "Unexpected result"),
        (ERROR_EXCEPTION,  "Exception occurred"),
        (ERROR_MEMORY,     "Memory error"),
        (ERROR_TIMEOUT,    "Timeout"),
        (ERROR_RESOURCE,   "Resource error"),
        (ERROR_MPI,        "MPI error"),
        (ERROR_OMP,        "OpenMP error"),
        (ERROR_CUDA,       "CUDA error"),
        (ERROR_INTERNAL,   "Internal framework error"),
        (ERROR_COLLISION,  "Collision in test names"),
        (ERROR_NOT_FOUND,  "Test not found"),
        (ERROR_UNKNOWN,    "Unknown error"),
    ]
    for bit, msg in _errors:
        if exit_status & bit:
            return color(msg, "red")
    return color("Unknown status", "red")

def symbol(exit_status: int) -> str:
    if exit_status & SUCCESS:    return color("✓", "green")
    if exit_status & SKIPPED:    return color("~", "blue")
    if exit_status & INCOMPLETE: return color("?", "yellow")
    return color("✗", "red")

def passed(exit_status: int) -> bool:
    return bool(exit_status & SUCCESS)

def strleak(allocation: int, deallocation: int) -> str | None:
    diff = allocation - deallocation
    if diff > 0:
        return f"leak: {diff} bytes (alloc {allocation}, free {deallocation})"
    if diff < 0:
        return f"over-free: {-diff} bytes (alloc {allocation}, free {deallocation})"
    return None

def strtest(test: dict) -> str:
    exit_status = test.get("exit_status", 0)
    flags_val   = test.get("test_flags",  0)
    name        = test.get("test_name",   "<unknown>")
    time_ns     = test.get("time_ns",     0)
    alloc       = test.get("allocation",   0)
    dealloc     = test.get("deallocation", 0)

    sym    = symbol(exit_status)
    status = strexit(exit_status, flags_val)
    time   = f"{time_ns} ns" if time_ns else ""
    leak   = strleak(alloc, dealloc)

    parts = [f"{sym} {name:<40} {status}"]
    if time:
        parts.append(time)
    if leak:
        parts.append(f"{color('⚠', 'yellow')} {leak}")
    return "  ".join(parts)
