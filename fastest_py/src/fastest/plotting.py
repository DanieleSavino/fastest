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


class LegendLocation(Enum):
    """Valid legend placements."""
    BEST          = "best"
    UPPER_RIGHT   = "upper right"
    UPPER_LEFT    = "upper left"
    LOWER_LEFT    = "lower left"
    LOWER_RIGHT   = "lower right"
    CENTER_LEFT   = "center left"
    CENTER_RIGHT  = "center right"
    LOWER_CENTER  = "lower center"
    UPPER_CENTER  = "upper center"
    CENTER        = "center"


class LineStyle(Enum):
    """Line styles for plotted curves and grids."""
    SOLID         = "-"
    DASHED        = "--"
    DASH_DOT      = "-."
    DOTTED        = ":"


class MarkerStyle(Enum):
    """Common marker shapes."""
    POINT             = "."
    PIXEL             = ","
    CIRCLE            = "o"
    TRIANGLE_DOWN     = "v"
    TRIANGLE_UP       = "^"
    TRIANGLE_LEFT     = "<"
    TRIANGLE_RIGHT    = ">"
    SQUARE            = "s"
    PENTAGON          = "p"
    STAR              = "*"
    HEXAGON1          = "h"
    HEXAGON2          = "H"
    PLUS              = "+"
    X                 = "x"
    DIAMOND           = "D"
    THIN_DIAMOND      = "d"
    VERTICAL_LINE     = "|"
    HORIZONTAL_LINE   = "_"


# ── Plotter with builder pattern ──────────────────────────────────────────────

class Plotter:
    """
    Builder‑style plot customiser for `CompareResult` objects.

    Every aesthetic choice is made through typed enums – no raw strings.

    Example::

        from fastest.plotting import Plotter, PlotMode, LegendLocation, LineStyle

        (Plotter()
         .set_title("Comparison of Sorting Algorithms")
         .set_bg_color("#313131")
         .set_pool_color(0, "#ff5f5f")
         .set_legend(LegendLocation.UPPER_LEFT, fontsize=9)
         .set_grid(True, style=LineStyle.DASHED, color="#888888")
         .set_marker(MarkerStyle.DIAMOND, size=6)
         .plot(result, "output.png", PlotMode.MEAN))
    """

    def __init__(self) -> None:
        # Figure defaults
        self._fig_size: tuple[float, float] = (10.0, 6.0)
        self._fig_dpi: int = 150
        self._bg_color: str = "#ffffff"
        self._title: Optional[str] = None
        self._title_color: str = "#333333"
        self._title_size: int = 16

        # Axis defaults
        self._x_label: str = "Test index within pool"
        self._y_label: Optional[str] = None   # set by mode if None
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
        self._pool_colors: dict[int, str] = {}   # index -> hex color
        self._default_colors: list[str] = [
            "#1f77b4", "#ff7f0e", "#2ca02c", "#d62728",
            "#9467bd", "#8c564b", "#e377c2", "#7f7f7f",
            "#bcbd22", "#17becf"
        ]

        # Info box (metadata above the title)
        self._show_info: bool = True
        self._info_fontsize: int = 9
        self._info_color: str = "#666666"

    # ── Builder setters (return self for chaining) ──────────────────────────

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
        if not visible:
            self._show_grid = False
        else:
            self._show_grid = True
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

    # ── Main rendering ──────────────────────────────────────────────────────

    def plot(self, result: CompareResult, filepath: str,
             mode: PlotMode = PlotMode.MEAN) -> None:
        """Render the comparison graph and save it to *filepath*."""
        fig, ax = plt.subplots(figsize=self._fig_size)
        fig.patch.set_facecolor(self._bg_color)
        ax.set_facecolor(self._bg_color)

        # --- Plot each pool ---
        for idx, pool in enumerate(result.pools):
            pool_data = result.data[pool.name]
            test_names = pool.tests
            y_vals = [getattr(pool_data[t], mode.value) for t in test_names]
            x_vals = range(1, len(test_names) + 1)

            color = self._pool_colors.get(
                idx,
                self._default_colors[idx % len(self._default_colors)]
            )

            ax.plot(x_vals, y_vals,
                    marker=self._marker_style.value,
                    markersize=self._marker_size,
                    linewidth=self._line_width,
                    linestyle=self._line_style.value,
                    color=color,
                    label=pool.name)

        # --- Labels ---
        ax.set_xlabel(self._x_label, color=self._label_color,
                      fontsize=self._label_size)
        ylabel = self._y_label or f"time ({mode.value}) [ns]"
        ax.set_ylabel(ylabel, color=self._label_color,
                      fontsize=self._label_size)

        # --- Ticks ---
        ax.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
        ax.tick_params(axis='both', colors=self._tick_color,
                       labelsize=self._tick_size)

        # --- Grid ---
        if self._show_grid:
            ax.grid(True, linestyle=self._grid_style.value,
                    color=self._grid_color, alpha=self._grid_alpha)

        # --- Legend ---
        ax.legend(loc=self._legend_loc.value,
                  fontsize=self._legend_fontsize)

        # --- Title & info ---
        title = self._title or f"Test Comparison – {mode.name}"
        ax.set_title(title, color=self._title_color,
                     fontsize=self._title_size)

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
