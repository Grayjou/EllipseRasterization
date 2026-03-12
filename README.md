# Ellipse Rasterization Algorithms

A systematic investigation and empirical performance comparison of discrete ellipse rasterization algorithms operating on integer pixel grids.

---

## Overview

This project implements and benchmarks five distinct approaches to ellipse rasterization, each exhibiting different time complexity characteristics. The objective is to generate the complete set of integer-coordinate points contained within an ellipse, as well as its thin (8-connectivity) and thick (4-connectivity) boundary outlines.

---

## Summary

Among the five algorithms evaluated, `IncrementalFastAxisFlip` (0.8314 μs) and `IncrementalFast` (0.8672 μs) consistently outperform the `Direct` baseline (1.0284 μs) by approximately 20% across a comprehensive 200×200 grid evaluation comprising 40,000 sample configurations. A sixth candidate, `IncrementalBinary`, was prototyped but ultimately excluded: because rasterized heights tend to lie near their respective upper bounds, the incremental linear scan required fewer iterations than the binary search in virtually all practical cases. Only at sizes on the order of 2000×2000 did the binary search variant approach competitive performance.

The `Direct` algorithm is notably competitive relative to the simplicity of its implementation. The incremental family of algorithms, while faster in the best case, demands a more involved implementation, as it requires exploiting both the first and second discrete derivatives of the ellipse height function.

---

## Benchmark Results

All benchmarks were conducted on ellipses ranging from 1×1 to 200×200, evaluating every integer combination within this range (40,000 unique configurations total), with five timed runs per configuration. Reported times are mean execution times in microseconds (μs) with standard deviation, along with the 1st and 5th percentiles.

| Algorithm | Mean (μs) ± Std Dev | 1st Percentile | 5th Percentile |
|---|---|---|---|
| Direct | 1.0284 ± 0.3881 | 0.0230 | 0.4522 |
| Hybrid | 1.2425 ± 0.4616 | 0.0220 | 0.4533 |
| Incremental | 1.0809 ± 0.3644 | 0.0224 | 0.4505 |
| IncrementalFast | 0.8672 ± 0.2824 | 0.0211 | 0.4344 |
| IncrementalFastAxisFlip | 0.8314 ± 0.2689 | 0.0211 | 0.4262 |
| IncrementalReverse | 1.0353 ± 0.3721 | 0.0226 | 0.4616 |
| IncrementalReverseFast | 0.9288 ± 0.3159 | 0.0224 | 0.4459 |

### Benchmark Configuration

- **Size range**: 1×1 to 200×200 (exhaustive sampling)
- **Total configurations**: 40,000
- **Runs per configuration**: 5
- **Metric**: Mean execution time (μs)
- **Platform**: x64, MSVC with `/O2` optimization

---

## Algorithms Implemented

### 1. Direct — O(W)

Evaluates the ellipse equation directly using floating-point arithmetic:

- Computes $y = b\sqrt{1 - (x/a)^2}$ for each column $x$
- Mathematically straightforward and highly predictable in runtime
- Requires one `sqrt()` operation per column

---

### 2. Incremental — O(W + H)

Exploits the monotonic property that rasterized heights increase toward the ellipse center:

- Each column's search begins from the preceding column's height rather than from a fixed lower bound
- Operates exclusively on integer comparisons
- Particularly well-suited to tall ellipses

---

### 3. Hybrid — O(W + H) *(currently produces heights with ±1 error; under repair)*

Combines incremental search with analytically derived derivative bounds:

- Exploits the fact that $dy/dx$ is monotonically decreasing toward the center
- Narrows the search interval using predictions based on the previous height differential
- Performs a bounded linear search within the predicted range

---

### 4. Incremental Reverse — O(W + H)

Optimizes traversal for the steep region of the ellipse:

- Computes the first column height directly
- Predicts subsequent heights from the previous finite difference
- Reverts to standard incremental search in the flat region
- Achieves strong overall performance across diverse aspect ratios

---

### 5. Incremental Fast Axis Flip — O(W + H)

Extends `IncrementalFast` to handle both tall and wide ellipses efficiently:

- Applies pure integer incremental arithmetic
- For wide ellipses, transposes axes prior to rasterization
- Accumulates the transposed output via interval accumulation, avoiding a 2D intermediate grid
- Restores output consistency after transposition, yielding improved performance in the wide-ellipse regime

---

## Theoretical Analysis of Derivative Bounds

Let $q(x) = b\sqrt{1 - x^2/a^2}$ denote the continuous ellipse height function, and define the following:

$$h_r(x) = \lfloor q(x) + 0.49 \rfloor \quad \text{(rasterized height)}$$

$$d_r(x) = h_r(x) - h_r(x - 1) \quad \text{(discrete finite difference)}$$

### Upper Bound

The linear function

$$U(x) = \frac{h_r(1 - a) + 1}{1 - a} \cdot x$$

is a provable upper bound on $d_r(x)$ for $x \in [1-a,\ 0]$.

At small ellipse sizes, however, finite differences collapse to either zero or one across the entire domain, rendering this bound trivially loose and introducing unnecessary computational overhead.

### Lower Bound

The ellipse height function is steep over $[-a,\ -a/4]$ and nearly flat over $[-a/4,\ 0]$. Accordingly, a piecewise lower bound is defined as follows:

$$L(x) = \begin{cases} -\dfrac{h_r(1-a)}{1 + a/4}\left(x + \dfrac{3a}{4}\right), & x \in \left[1 - a,\ -\dfrac{3a}{4}\right] \\[10pt] \dfrac{2b}{3a\left(1 - \tfrac{3a}{4}\right)} \cdot x, & x \in \left[-\dfrac{3a}{4},\ 0\right] \end{cases}$$

In the flat interval, the upper bound tightens further to $\frac{h_r(1-a)+1}{2(1-a)} \cdot x$.

### Practical Considerations

These bounds offer a meaningful reduction in search iterations only when the aspect ratio $b/a$ is large. For $b/a \leq 6$, the lower bound is identically zero throughout the flat interval. Empirically, the naïve incremental approach performs between 3 and 6 iterations per column even at sizes as large as 20×120 — negligible relative to the overhead incurred by evaluating the bounds and performing the associated comparisons.

A meaningful benefit would require the naïve approach to incur on the order of 15 or more iterations per column, a regime that arises only in extremely elongated ellipses with large semi-major axes.

#### Recommended Validation Methodology

The most informative evaluation would consist of:
1. A heatmap of per-column iteration counts for the naïve approach across representative ellipse sizes
2. Overlay of the upper and lower bounds to visualize the achievable reduction

Given that both the bounds and the excess iterations scale approximately linearly, this reduction is only expected to be significant in high-aspect-ratio configurations well outside the typical operating range.

---

## Possible Improvements

- Tighter derivative bounds for high-aspect-ratio ellipses, as detailed above
- A Bresenham-style midpoint algorithm operating entirely in integer arithmetic
- Anti-aliased filled ellipse generation
- GPU-accelerated batch rasterization for large-scale workloads
- Support for rotated ellipses

---

## Project Structure

```
EllipseRasterization/
├── include/
│   ├── ellipse_common.h          # Shared utilities and data structures
│   ├── ellipse_algorithms.h      # All rasterization algorithm implementations
│   └── ellipse_rasterization.h   # Integer coordinate generation interface
├── src/
│   └── main.cpp                  # Benchmark harness
├── results/
│   ├── benchmark_results.csv     # Raw benchmark data
│   ├── benchmark_heatmaps.png    # Per-algorithm performance heatmaps
│   ├── algorithm_comparison.png  # Comparative line plots
│   └── speedup_analysis.png      # Relative speedup over Direct baseline
├── docs/
├── examples/
├── build.ps1                     # PowerShell build script
├── build.bat                     # Batch build script
├── CMakeLists.txt                # CMake configuration
├── visualize_results.py          # Python visualization script
└── README.md                     # This document
```

---

## Building & Running

### Prerequisites

- **Compiler**: MSVC Build Tools (Visual Studio 2022), targeting x64
- **Optional**: Python 3 with `matplotlib`, `pandas`, and `seaborn` for result visualization

### Quick Start

```powershell
# Build and execute benchmark
.\build.ps1

# Generate visualizations (requires Python)
python visualize_results.py
```

### Manual Build

```powershell
# Compile with MSVC
cl /EHsc /std:c++17 /O2 /I"include" src\main.cpp /Fe:build\ellipse_benchmark.exe

# Execute
.\build\ellipse_benchmark.exe
```

---

## Output Format

Each rasterized ellipse produces three distinct point sets:

1. **Filled region**: All integer-coordinate points $(x, y)$ satisfying the ellipse inequality
2. **Thin outline** (8-connectivity): Filled points with at least one 8-neighbor outside the ellipse
3. **Thick outline** (4-connectivity): Filled points with at least one cardinal 4-neighbor outside the ellipse

### Example Output

```
Ellipse: 20x12
Total filled points: 192
Thin outline points (8-way): 60
Thick outline points (4-way): 44

Visualization (█=thin outline, ▓=thick outline, ░=interior):

            █ █ █ █ █ █ █ █
      █ █ █ █ ░ ░ ░ ░ ░ ░ █ █ █ █
    █ █ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ █ █
  █ █ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ ░ █ █
...
```

---

## Performance Summary

| Algorithm | Time Complexity | Characteristic Use Case | Notes |
|---|---|---|---|
| Direct | O(W) | Predictable, general-purpose | Floating-point arithmetic |
| Incremental | O(W + H) | Tall ellipses | Pure integer arithmetic |
| Hybrid | O(W + H) | Balanced aspect ratios | Bounded search; currently under repair |
| IncrementalReverse | O(W + H) | General use | Prediction with incremental fallback |
| IncrementalFastAxisFlip | O(W + H) | Wide and tall ellipses | Axis transposition for wide-case optimization |

### Key Observations

- `IncrementalFastAxisFlip` and `IncrementalFast` achieve the lowest mean execution times across the comprehensive 40,000-configuration test suite
- `Direct` exhibits the most uniform performance profile, as its O(W) complexity is independent of ellipse height
- All algorithms complete in sub-microsecond time for small ellipses (approximately 20×20)
- Execution time scales near-linearly with ellipse perimeter across the tested configurations

---

## Technical Details

### Coordinate System

- All ellipses are centered at the origin $(0, 0)$
- `two_a` denotes the full width ($2a$, where $a$ is the semi-major axis)
- `two_b` denotes the full height ($2b$, where $b$ is the semi-minor axis)
- Both even and odd integer dimensions are supported

### Ellipse Inequality

A point $(x, y)$ is considered interior to the ellipse if:

$$\frac{x^2}{a^2} + \frac{y^2}{b^2} \leq 1$$

Appropriate adjustments are applied for discrete pixel centers and odd-dimensioned ellipses.

---

## Visualization

The `visualize_results.py` script produces three categories of figures:

1. **Heatmaps**: Per-algorithm execution time across all tested size combinations
2. **Comparative line plots**: Execution time on square ellipses across all algorithms
3. **Speedup analysis**: Relative performance normalized to the `Direct` baseline

```bash
python visualize_results.py
```

Output files:
- `results/benchmark_heatmaps.png`
- `results/algorithm_comparison.png`
- `results/speedup_analysis.png`

---

## Future Work

- [x] Adaptive axis transposition for wide ellipses (`IncrementalFastAxisFlip`)
- [ ] Bresenham-style integer-only midpoint ellipse algorithm
- [ ] Anti-aliased filled ellipse rasterization
- [ ] GPU-accelerated batch rasterization
- [ ] Support for arbitrarily rotated ellipses

---

## References

- Bresenham, J. E. — Midpoint ellipse algorithm and related scan-conversion techniques
- Foley, van Dam, et al. — *Computer Graphics: Principles and Practice* — Scanline filling methods
- Klette, R. & Rosenfeld, A. — *Digital Geometry* — Rasterization on integer lattices

---

## License

See [LICENSE](LICENSE) for details.

---

> **Note**: With the exception of `Hybrid` (currently under repair), all implemented algorithms produce identical output. They differ exclusively in their computational strategy and resulting performance characteristics.