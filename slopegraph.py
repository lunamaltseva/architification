#!/usr/bin/env python3
"""
Linux Distribution Market Share — Slopegraph (Satire Edition)

Tufte-inspired slopegraph: two time points, slopes as the primary visual,
direct labelling, no legend, minimal decoration.
"""

import matplotlib.pyplot as plt

# ── Data ──────────────────────────────────────────────────────────────────────

YEAR_LEFT  = "2023"
YEAR_RIGHT = "2026"

# (name, share_left, share_right, brand_color)
DISTROS = [
    ("Ubuntu",  32, 31, "#E95420"),   # Ubuntu orange
    ("Debian",  16, 15, "#A81D33"),   # Debian red
    ("Mint",    14, 13, "#4CAF50"),   # Mint green
    ("Fedora",   9,  8, "#3C6EB4"),   # Fedora blue
    ("Arch",     8,  0, "#1793D1"),   # Arch blue — into the void
    ("Gentoo",   1,  9, "#54487A"),   # Gentoo purple — the great ascent
]

# ── Layout constants ──────────────────────────────────────────────────────────

X_LEFT,  X_RIGHT  = 0.0, 1.0
LABEL_PAD         = 0.06    # horizontal gap: line endpoint → label
MIN_LABEL_GAP     = 2.2     # minimum vertical distance between adjacent labels
LINE_WIDTH_NORMAL = 1.5
LINE_WIDTH_HERO   = 2.5
ALPHA_NORMAL      = 0.72
ALPHA_HERO        = 1.00
FONT_SIZE         = 9.5
HIGHLIGHT         = {"Arch", "Gentoo"}

# ── Label collision resolver ──────────────────────────────────────────────────

def resolve_overlaps(values: list[float], min_gap: float) -> list[float]:
    """
    Iteratively push label positions apart until no adjacent pair
    is closer than min_gap. Preserves sort order.
    """
    pos = sorted(values)
    for _ in range(300):
        moved = False
        for i in range(len(pos) - 1):
            gap = pos[i + 1] - pos[i]
            if gap < min_gap:
                shift = (min_gap - gap) / 2
                pos[i]     -= shift
                pos[i + 1] += shift
                moved = True
        if not moved:
            break
    return pos


def build_nudge_map(distros, col: int) -> dict[str, float]:
    """
    Return {name: nudged_y} for whichever column (1=left, 2=right)
    is being labelled.
    """
    ordered = sorted(distros, key=lambda d: d[col])
    raw     = [d[col] for d in ordered]
    nudged  = resolve_overlaps(raw, MIN_LABEL_GAP)
    return {d[0]: y for d, y in zip(ordered, nudged)}

# ── Main drawing function ─────────────────────────────────────────────────────

def draw_slopegraph():
    fig, ax = plt.subplots(figsize=(9, 4.5))
    fig.patch.set_facecolor("white")
    ax.set_facecolor("white")

    left_nudge  = build_nudge_map(DISTROS, 1)
    right_nudge = build_nudge_map(DISTROS, 2)

    all_vals = [v for *_, l, r, __ in DISTROS for v in (l, r)]
    y_min    = min(all_vals) - 3
    y_max    = max(all_vals) + 4

    for name, left, right, color in DISTROS:
        hero  = name in HIGHLIGHT
        lw    = LINE_WIDTH_HERO   if hero else LINE_WIDTH_NORMAL
        alpha = ALPHA_HERO        if hero else ALPHA_NORMAL
        fw    = "bold"            if hero else "normal"
        draw_color = color if hero else "#bbbbbb"

        # Slope line
        ax.plot(
            [X_LEFT, X_RIGHT], [left, right],
            color=draw_color, linewidth=lw, alpha=alpha,
            solid_capstyle="round", zorder=3,
        )

        # Endpoint dots
        ax.scatter(
            [X_LEFT, X_RIGHT], [left, right],
            s=30, color=draw_color, alpha=alpha,
            edgecolors="none", zorder=4,
        )

        # Left label: "Name  XX%"
        ax.text(
            X_LEFT - LABEL_PAD, left_nudge[name],
            f"{name}  {left}%",
            ha="right", va="center",
            fontsize=FONT_SIZE, color=draw_color, fontweight=fw,
        )

        # Right label: "XX%  Name"
        ax.text(
            X_RIGHT + LABEL_PAD, right_nudge[name],
            f"{right}%  {name}",
            ha="left", va="center",
            fontsize=FONT_SIZE, color=draw_color, fontweight=fw,
        )

    # ── Axes: Tufte style — remove all non-essential ink ─────────────────────

    for spine in ax.spines.values():
        spine.set_visible(False)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_xlim(-0.70, 1.70)
    ax.set_ylim(y_min, y_max)

    # Vertical poles
    ax.axvline(X_LEFT,  color="#cccccc", linewidth=0.8, zorder=1)
    ax.axvline(X_RIGHT, color="#cccccc", linewidth=0.8, zorder=1)

    # Year headers
    for x, year in ((X_LEFT, YEAR_LEFT), (X_RIGHT, YEAR_RIGHT)):
        ax.text(
            x, y_max, year,
            ha="center", va="bottom",
            fontsize=13, fontweight="bold", color="#111111",
        )

    # ── Output ────────────────────────────────────────────────────────────────

    out = "linux_marketshare_slopegraph.png"
    fig.tight_layout()
    fig.savefig(out, dpi=150, bbox_inches="tight", facecolor="white")
    print(f"Saved → {out}")
    plt.show()


if __name__ == "__main__":
    draw_slopegraph()
