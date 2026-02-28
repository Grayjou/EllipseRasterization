# Quick Start Guide

## Building the Project

### Windows (PowerShell)

```powershell
# Build and run benchmark
.\build.ps1
```

The script will:
1. Detect MSVC installation
2. Compile with `/O2` optimization
3. Run benchmarks automatically
4. Save results to `results/benchmark_results.csv`

### Alternative: Using CMake

```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\Release\ellipse_benchmark.exe
```

## Visualizing Results

### Install Python dependencies:

```powershell
pip install -r requirements.txt
```

### Generate plots:

```powershell
python visualize_results.py
```

This creates three visualization files in `results/`:
- `benchmark_heatmaps.png` - Performance heatmaps for each algorithm
- `algorithm_comparison.png` - Line plot comparing algorithms
- `speedup_analysis.png` - Relative speedup analysis

## Using the Algorithms

### Basic Usage

```cpp
#include "ellipse_algorithms.h"
#include "ellipse_rasterization.h"

// Generate ellipse points
int width = 50, height = 30;
auto [half_heights, full_heights] = ellipse::generate_ellipse_heights_incremental_reverse(width, height);

// Convert to coordinates
auto raster = ellipse::rasterize_ellipse(full_heights);

// Access results
std::cout << "Filled points: " << raster.filled_points.size() << "\n";
std::cout << "Outline points: " << raster.thin_outline.size() << "\n";
```

### Available Algorithms

```cpp
using namespace ellipse;

// 1. Direct (fastest for very small ellipses)
auto result1 = generate_ellipse_heights_direct(width, height);

// 2. Incremental (good for tall ellipses)
auto result2 = generate_ellipse_heights_incremental(width, height);

// 3. Hybrid (balanced performance)
auto result3 = generate_ellipse_heights_hybrid(width, height);

// 4. Incremental Reverse (best overall)
auto result4 = generate_ellipse_heights_incremental_reverse(width, height);

// 5. Incremental Fast Axis Flip (strong for wide ellipses)
auto result5 = generate_ellipse_heights_incremental_fast_axis_flip(width, height);
```

### Output Structure

```cpp
struct RasterizationResult {
    std::vector<Point> filled_points;   // All interior points
    std::vector<Point> thin_outline;    // 8-way connected outline
    std::vector<Point> thick_outline;   // 4-way connected outline
};

// Point is std::pair<int, int> representing (x, y)
```

## Running the Example

```powershell
# Compile example
cl /EHsc /std:c++17 /O2 /I"include" examples\simple_example.cpp /Fe:examples\example.exe

# Run
.\examples\example.exe
```

## Performance Tips

1. Use `IncrementalReverse` for general-purpose rasterization
2. Use `Direct` if you need predictable O(W) performance
3. All algorithms produce identical results
4. Compiled with `/O2` or `-O3` for best performance
5. Use release build for benchmarking

## Troubleshooting

### Compiler not found
- Install Visual Studio 2022 with "Desktop development with C++" workload
- Or install MinGW/Clang and update build scripts

### Python packages missing
```powershell
pip install pandas matplotlib seaborn numpy
```

### Build fails
- Ensure MSVC is properly installed
- Try running from "Developer Command Prompt for VS 2022"
- Check that all source files are in correct directories

## Next Steps

- Explore different ellipse sizes in the benchmark
- Modify `src/main.cpp` to customize benchmark parameters
- Implement your own algorithm variant
- Integrate into your graphics pipeline
