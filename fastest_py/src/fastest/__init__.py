from .runner import Runner, Pool, Stats, CompareResult

default_runner = Runner()

# ── Optional plotting support ─────────────────────────────────────────────────
try:
    from .plotting import PlotMode, Plotter
except ImportError:
    PlotMode = None
    plotter = None


def __getattr__(name: str):
    """
    Delegate unknown module attributes to `default_runner`.
    Special-cases `tests` to return the backend's test constants.
    """
    if name == "tests":
        backend = default_runner.backend  # raises RuntimeError if no backend
        return backend.tests
    try:
        return getattr(default_runner, name)
    except AttributeError:
        raise AttributeError(f"module {__name__!r} has no attribute {name!r}")


def __dir__() -> list[str]:
    """Enhance tab-completion with runner's public names."""
    standard = list(globals().keys())
    if default_runner.is_ready:
        runner_attrs = [attr for attr in dir(default_runner) if not attr.startswith('_')]
        standard.extend(runner_attrs)
        standard.append("tests")
    return sorted(set(standard))


__all__ = ["Runner", "Pool", "Stats", "CompareResult", "default_runner",
           "PlotMode", "Plotter"]
