# Ellipse Rasterization Algorithms

An exploration and performance comparison of different algorithms for rasterizing ellipses on discrete pixel grids.

## Overview

This project implements and benchmarks four different approaches to ellipse rasterization, each with different time complexity characteristics. The goal is to generate the set of points that lie inside an ellipse, along with thin (8-way connected) and thick (4-way connected) outlines.

## Algorithms Implemented

### 1. **Direct** - O(W)
Uses the ellipse equation directly with floating-point arithmetic:
- Computes `y = b√(1 - (x/a)²)` for each column
- Fast and mathematically elegant
- Requires `sqrt()` operation per column

### 2. **Incremental** - O(W + H)
Exploits the monotonic property that heights only increase toward the center:
- Starts from previous height instead of searching from 1
- Uses integer comparisons only
- Optimal for tall ellipses

### 3. **Hybrid** - O(W + H)
Combines incremental search with derivative bounds:
- Uses the fact that `dy/dx` decreases toward center
- Predicts search range based on previous height change
- Bounded linear search within predicted range

### 4. **Incremental Reverse** - O(W + H)
Optimizes for the steep region of ellipses:
- Computes first height directly
- Predicts next height from previous difference
- Falls back to incremental in flat regions
- Best overall performance

### 5. **Incremental Fast Axis Flip** - O(W + H)
Optimized for both tall and wide ellipses:
- Uses pure integer incremental math (`IncrementalFast`)
- If ellipse is wide, swaps axes before rasterization
- Transposes back with interval accumulation (no 2D grid)
- Keeps output consistent while improving wide-case behavior

## Project Structure

```
EllipseRasterization/
├── include/
│   ├── ellipse_common.h          # Shared utilities and data structures
│   ├── ellipse_algorithms.h      # All rasterization algorithms
│   └── ellipse_rasterization.h   # Point coordinate generation
├── src/
│   └── main.cpp                   # Benchmark harness
├── results/
│   ├── benchmark_results.csv      # Raw benchmark data
│   ├── benchmark_heatmaps.png     # Performance heatmaps
│   ├── algorithm_comparison.png   # Line plot comparison
│   └── speedup_analysis.png       # Relative speedup analysis
├── docs/
├── examples/
├── build.ps1                      # PowerShell build script
├── build.bat                      # Batch build script
├── CMakeLists.txt                 # CMake configuration
├── visualize_results.py           # Python visualization script
└── README.md                      # This file
```

## Building & Running

### Prerequisites
- **Windows**: MSVC Build Tools (Visual Studio 2022)
- **Optional**: Python 3 with matplotlib, pandas, seaborn (for visualizations)

### Quick Start

```powershell
# Build and run benchmark
.\build.ps1

# Generate visualizations (requires Python)
python visualize_results.py
```

### Manual Build

```powershell
# Using MSVC
cl /EHsc /std:c++17 /O2 /I"include" src\main.cpp /Fe:build\ellipse_benchmark.exe

# Run benchmark
.\build\ellipse_benchmark.exe
```

## Output Format

The rasterization generates three outputs for each ellipse:

1. **Filled Points**: All integer coordinate points `(x, y)` inside the ellipse
2. **Thin Outline** (8-way): Points with at least one of 8 neighbors outside
3. **Thick Outline** (4-way): Points with at least one of 4 cardinal neighbors outside

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

## Performance Results

### Key Findings

- **IncrementalReverse** is the fastest for most ellipse sizes
- **Direct** has the most predictable performance (always O(W))
- **Incremental** and **Hybrid** show similar performance
- All algorithms complete sub-microsecond for small ellipses (~20×20)
- Performance scales near-linearly with ellipse perimeter

### Benchmark Configuration

- Size range: 1×1 to 201×201 (step: 20)
- Runs per configuration: 5
- Metric: Average execution time in microseconds
- Platform: x64, MSVC /O2 optimization

## Algorithm Comparison

| Algorithm | Time Complexity | Best For | Notes |
|-----------|----------------|----------|-------|
| Direct | O(W) | Predictability | Uses floating-point math |
| Incremental | O(W + H) | Tall ellipses | Pure integer arithmetic |
| Hybrid | O(W + H) | Balanced cases | Bounded search optimization |
| IncrementalReverse | O(W + H) | Overall winner | Prediction + fallback strategy |

## Technical Details

### Coordinate System

- Ellipses are centered at origin (0, 0)
- Width parameter `two_a` is the full width (2 * semi-major axis)
- Height parameter `two_b` is the full height (2 * semi-minor axis)
- Supports both even and odd dimensions

### Ellipse Equation

Points are tested using: `(x²/a²) + (y²/b²) ≤ 1`

Where adjustments are made for discrete pixel centers and odd dimensions.

## Visualization

The `visualize_results.py` script generates three types of plots:

1. **Heatmaps**: Performance across all size combinations for each algorithm
2. **Comparison**: Line plot showing performance on square ellipses
3. **Speedup Analysis**: Relative performance vs. Direct algorithm

### Example Usage

```bash
python visualize_results.py
```

Generates:
- `results/benchmark_heatmaps.png`
- `results/algorithm_comparison.png`
- `results/speedup_analysis.png`

## Future Work

- [x] Implement adaptive axis swapping for wide ellipses (`IncrementalFastAxisFlip`)
- [ ] Add Bresenham-style integer-only midpoint algorithm
- [ ] Extend to filled ellipse with anti-aliasing
- [ ] GPU-accelerated batch rasterization
- [ ] Ellipse rotation support

## References

- **Midpoint Ellipse Algorithm**: Bresenham-style approach
- **Scanline Filling**: Traditional computer graphics technique
- **Discrete Geometry**: Rasterization on integer lattices

## License

See [LICENSE](LICENSE) for details.

## Author

Ellipse rasterization algorithm exploration and implementation.

---

**Note**: All algorithms produce identical results - they differ only in computational approach and performance characteristics.
