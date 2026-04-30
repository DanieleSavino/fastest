#!/usr/bin/env python3

import fastest
from fastest.plotting import PlotMode, LegendLocation, LineStyle, MarkerStyle

p1 = fastest.pool_from_prefix("t1")
p2 = fastest.pool_from_prefix("t2")

cmp = fastest.compare(p1, p2, n_repeats=5)
cmp.report()

(fastest.Plotter()
    .set_title("Sleep Scaling Comparison")
    .set_x_label("Array index × step (1ms)")
    .set_y_label("Execution time")
    .set_bg_color("#1a1a2e")
    .set_title_color("#e0e0e0")
    .set_label_color("#cccccc")
    .set_tick_color("#aaaaaa")
    .set_pool_color(0, "#00d2ff")   # t1 – cyan
    .set_pool_color(1, "#ff6b6b")   # t2 – coral
    .set_line_width(2.5)
    .set_marker(MarkerStyle.DIAMOND, size=10)
    .set_legend(LegendLocation.UPPER_LEFT, fontsize=11)
    .set_grid(True, color="#333355", style=LineStyle.DOTTED, alpha=0.5)
    .show_info(False)
    .set_dpi(200)
    .plot(cmp, "scaling.png", PlotMode.MEDIAN))
