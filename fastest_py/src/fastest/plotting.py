# fastest_py/src/fastest/plotting.py

from __future__ import annotations

from enum import Enum
from typing import Optional

from .runner import CompareResult

try:
    import matplotlib.pyplot as plt
    import matplotlib.ticker as ticker
    from matplotlib.figure import Figure
    from matplotlib.axes import Axes
except ImportError as exc:
    raise ImportError(
        "Package 'matplotlib' is required for plotting. "
        "Install it with `pip install matplotlib`."
    ) from exc


# ── Enums for type-safe configuration ─────────────────────────────────────────

class PlotMode(Enum):
    """Statistic to display on the y-axis."""
    MEAN   = "mean"
    MEDIAN = "median"
    MIN    = "min"
    MAX    = "max"
    STDDEV = "stddev"


class PlotTransform(Enum):
    """How to render the y-axis values.

    ABSOLUTE  – raw nanosecond values (default, existing behaviour).
    DIFF      – signed percent difference relative to the first pool.
                Pool 0 is the baseline and is drawn as a flat 0 % reference
                line; positive values mean slower than baseline.
    """
    ABSOLUTE = "absolute"
    DIFF     = "diff"


class LegendLocation(Enum):
    """Valid legend placements."""
    BEST         = "best"
    UPPER_RIGHT  = "upper right"
    UPPER_LEFT   = "upper left"
    LOWER_LEFT   = "lower left"
    LOWER_RIGHT  = "lower right"
    CENTER_LEFT  = "center left"
    CENTER_RIGHT = "center right"
    LOWER_CENTER = "lower center"
    UPPER_CENTER = "upper center"
    CENTER       = "center"


class LineStyle(Enum):
    """Line styles for plotted curves and grids."""
    SOLID    = "-"
    DASHED   = "--"
    DASH_DOT = "-."
    DOTTED   = ":"


class MarkerStyle(Enum):
    """Common marker shapes."""
    POINT           = "."
    PIXEL           = ","
    CIRCLE          = "o"
    TRIANGLE_DOWN   = "v"
    TRIANGLE_UP     = "^"
    TRIANGLE_LEFT   = "<"
    TRIANGLE_RIGHT  = ">"
    SQUARE          = "s"
    PENTAGON        = "p"
    STAR            = "*"
    HEXAGON1        = "h"
    HEXAGON2        = "H"
    PLUS            = "+"
    X               = "x"
    DIAMOND         = "D"
    THIN_DIAMOND    = "d"
    VERTICAL_LINE   = "|"
    HORIZONTAL_LINE = "_"


# ── Plotter with builder pattern ──────────────────────────────────────────────

class Plotter:
    """
    Builder-style plot customiser for `CompareResult` objects.

    Every aesthetic choice is made through typed enums – no raw strings.
    Axis labels must be set explicitly via set_x_label / set_y_label; there
    are no auto-generated fallbacks.

    Example::

        from fastest.plotting import Plotter, PlotMode, PlotTransform, LegendLocation, LineStyle

        (Plotter()
         .set_title("BINE vs RING – AllReduce scaling")
         .set_x_label("Message size")
         .set_y_label("Latency (ns)")           # or "Δ vs RING (%)" for DIFF
         .set_bg_color("#313131")
         .set_pool_colors("#ff5f5f", "#1f77b4")
         .set_legend(LegendLocation.UPPER_LEFT, fontsize=9)
         .set_grid(True, style=LineStyle.DASHED, color="#888888")
         .set_marker(MarkerStyle.DIAMOND, size=6)
         .plot(result, "output.png", PlotMode.MEDIAN, PlotTransform.DIFF))
    """

    def __init__(self) -> None:
        # Figure defaults
        self._fig_size: tuple[float, float] = (10.0, 6.0)
        self._fig_dpi: int = 150
        self._bg_color: str = "#ffffff"
        self._title: Optional[str] = None
        self._title_color: str = "#333333"
        self._title_size: int = 16

        # Axis labels — no defaults, must be set by caller
        self._x_label: Optional[str] = None
        self._y_label: Optional[str] = None
        self._label_color: str = "#333333"
        self._label_size: int = 12
        self._tick_color: str = "#333333"
        self._tick_size: int = 10

        # Grid
        self._show_grid: bool = True
        self._grid_color: str = "#cccccc"
        self._grid_style: LineStyle = LineStyle.DOTTED
        self._grid_alpha: float = 0.7

        # Legend
        self._legend_loc: LegendLocation = LegendLocation.BEST
        self._legend_fontsize: int = 10

        # Lines & markers (data curves)
        self._line_width: float = 2.0
        self._line_style: LineStyle = LineStyle.SOLID
        self._marker_size: int = 8
        self._marker_style: MarkerStyle = MarkerStyle.CIRCLE

        # Pool colours
        self._pool_colors: dict[int, str] = {}
        self._default_colors: list[str] = [
            "#1f77b4", "#ff7f0e", "#2ca02c", "#d62728",
            "#9467bd", "#8c564b", "#e377c2", "#7f7f7f",
            "#bcbd22", "#17becf",
        ]

        # Info box (metadata above the title)
        self._show_info: bool = True
        self._info_fontsize: int = 9
        self._info_color: str = "#666666"

    # ── Builder setters ─────────────────────────────────────────────────────

    def set_fig_size(self, width: float, height: float) -> Plotter:
        self._fig_size = (width, height)
        return self

    def set_dpi(self, dpi: int) -> Plotter:
        self._fig_dpi = dpi
        return self

    def set_bg_color(self, color: str) -> Plotter:
        """Figure / axes face color (hex or named colour)."""
        self._bg_color = color
        return self

    def set_title(self, title: str) -> Plotter:
        self._title = title
        return self

    def set_title_color(self, color: str) -> Plotter:
        self._title_color = color
        return self

    def set_title_size(self, size: int) -> Plotter:
        self._title_size = size
        return self

    def set_x_label(self, label: str) -> Plotter:
        self._x_label = label
        return self

    def set_y_label(self, label: str) -> Plotter:
        self._y_label = label
        return self

    def set_label_color(self, color: str) -> Plotter:
        self._label_color = color
        return self

    def set_label_size(self, size: int) -> Plotter:
        self._label_size = size
        return self

    def set_tick_color(self, color: str) -> Plotter:
        self._tick_color = color
        return self

    def set_tick_size(self, size: int) -> Plotter:
        self._tick_size = size
        return self

    def set_grid(self, visible: bool = True, *,
                 color: Optional[str] = None,
                 style: Optional[LineStyle] = None,
                 alpha: Optional[float] = None) -> Plotter:
        """Configure the background grid."""
        self._show_grid = visible
        if visible:
            if color is not None:
                self._grid_color = color
            if style is not None:
                self._grid_style = style
            if alpha is not None:
                self._grid_alpha = alpha
        return self

    def set_legend(self, loc: LegendLocation = LegendLocation.BEST,
                   fontsize: int = 10) -> Plotter:
        self._legend_loc = loc
        self._legend_fontsize = fontsize
        return self

    def set_line_width(self, width: float) -> Plotter:
        self._line_width = width
        return self

    def set_line_style(self, style: LineStyle) -> Plotter:
        self._line_style = style
        return self

    def set_marker(self, style: MarkerStyle = MarkerStyle.CIRCLE,
                   size: int = 8) -> Plotter:
        self._marker_style = style
        self._marker_size = size
        return self

    def set_pool_color(self, pool_index: int, color: str) -> Plotter:
        """Colour of the line for the *pool_index*-th pool (0-based)."""
        self._pool_colors[pool_index] = color
        return self

    def set_pool_colors(self, *colors: str) -> Plotter:
        """Set pool colours in order (one string per pool)."""
        for i, c in enumerate(colors):
            self._pool_colors[i] = c
        return self

    def show_info(self, show: bool = True) -> Plotter:
        self._show_info = show
        return self

    def set_info_style(self, fontsize: int = 9, color: str = "#666666") -> Plotter:
        self._info_fontsize = fontsize
        self._info_color = color
        return self

    # ── Internal helpers ────────────────────────────────────────────────────

    def _pool_color(self, idx: int) -> str:
        return self._pool_colors.get(
            idx, self._default_colors[idx % len(self._default_colors)]
        )

    def _raw_values(self, result: CompareResult,
                    mode: PlotMode) -> list[list[float]]:
        """Return per-pool lists of the chosen statistic."""
        return [
            [getattr(result.data[pool.name][t], mode.value) for t in pool.tests]
            for pool in result.pools
        ]

    def _diff_values(self, raw: list[list[float]]) -> list[list[float]]:
        """Convert raw values to signed % difference vs pool 0."""
        baseline = raw[0]
        out: list[list[float]] = []
        for idx, vals in enumerate(raw):
            if idx == 0:
                out.append([0.0] * len(vals))
            else:
                out.append([
                    ((v - b) / b * 100.0 if b != 0.0 else 0.0)
                    for v, b in zip(vals, baseline)
                ])
        return out

    # ── Main rendering ──────────────────────────────────────────────────────

    def plot(self, result: CompareResult, filepath: str,
             mode: PlotMode = PlotMode.MEAN,
             transform: PlotTransform = PlotTransform.ABSOLUTE) -> None:
        """Render the comparison graph and save it to *filepath*.

        Parameters
        ----------
        result:
            Output of a fastest compare run.
        filepath:
            Destination image path (format inferred from extension).
        mode:
            Which statistic to pull from each test result.
        transform:
            ABSOLUTE for raw ns values; DIFF for signed % vs pool 0.
        """
        fig, ax = plt.subplots(figsize=self._fig_size)
        fig.patch.set_facecolor(self._bg_color)
        ax.set_facecolor(self._bg_color)

        # --- Compute y-values ---
        raw = self._raw_values(result, mode)
        y_data = self._diff_values(raw) if transform is PlotTransform.DIFF else raw

        # --- Plot each pool ---
        for idx, (pool, y_vals) in enumerate(zip(result.pools, y_data)):
            x_vals = range(1, len(pool.tests) + 1)
            ax.plot(x_vals, y_vals,
                    marker=self._marker_style.value,
                    markersize=self._marker_size,
                    linewidth=self._line_width,
                    linestyle=self._line_style.value,
                    color=self._pool_color(idx),
                    label=pool.name)

        # --- Zero reference line in diff mode ---
        if transform is PlotTransform.DIFF:
            ax.axhline(0,
                       color=self._grid_color,
                       linewidth=1.0,
                       linestyle=LineStyle.DASHED.value,
                       alpha=0.9)

        # --- Labels ---
        if self._x_label is not None:
            ax.set_xlabel(self._x_label, color=self._label_color,
                          fontsize=self._label_size)
        if self._y_label is not None:
            ax.set_ylabel(self._y_label, color=self._label_color,
                          fontsize=self._label_size)

        # --- Ticks ---
        ax.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
        ax.tick_params(axis='both', colors=self._tick_color,
                       labelsize=self._tick_size)

        # --- Grid ---
        if self._show_grid:
            ax.grid(True,
                    linestyle=self._grid_style.value,
                    color=self._grid_color,
                    alpha=self._grid_alpha)

        # --- Legend ---
        ax.legend(loc=self._legend_loc.value, fontsize=self._legend_fontsize)

        # --- Title & info ---
        title = self._title or f"Test Comparison – {mode.name}"
        ax.set_title(title, color=self._title_color, fontsize=self._title_size)

        if self._show_info:
            n_pools = len(result.pools)
            n_tests = len(result.pools[0].tests) if result.pools else 0
            info_text = (f"pools: {n_pools}  |  "
                         f"tests per pool: {n_tests}  |  "
                         f"repetitions: {result.n_repeats}")
            ax.text(0.5, 1.02, info_text,
                    transform=ax.transAxes, ha='center', va='bottom',
                    fontsize=self._info_fontsize, color=self._info_color)

        fig.tight_layout()
        fig.savefig(filepath, dpi=self._fig_dpi)
        plt.close(fig)
