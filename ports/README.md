# Ellipse Rasterization Algorithm Ports

This directory contains ports of the fastest ellipse rasterization algorithms to various programming languages, including outline extraction.

## Algorithms Included

### IncrementalFast - O(W + H)
Pure integer arithmetic implementation that avoids costly floating-point operations and square root calculations. Uses incremental computation to efficiently determine which pixels fall inside the ellipse boundary.

**Key Features:**
- Integer-only operations (no `sqrt()` calls)
- Incremental delta updates
- Optimal for both tall and square ellipses
- Consistently fast performance across all sizes

### IncrementalFastAxisFlip - O(W + H)
Experimental variant intended for wide ellipses. Currently implemented as a wrapper to `IncrementalFast` due to complexities in handling axis transposition with mixed even/odd dimensions.

**Note:** The axis-flip optimization is marked for future enhancement due to discrete geometry centering challenges.

### Outline Extraction from Heights

Two outline types can be extracted directly from the heights array in O(perimeter) time, without building the full O(area) point set:

- **Thin outline (8-connected):** A filled pixel is on the outline if at least one of its 8 neighbors (including diagonals) is outside. Produces a 1-pixel-wide boundary chain.
- **Full outline (4-connected):** A filled pixel is on the outline if at least one of its 4 cardinal neighbors (N/S/E/W) is outside. Produces a thicker boundary because diagonal gaps are not bridged.

## Available Ports

### Python (`ellipse_fast.py`)

**Requirements:** Python 3.6+

**Usage:**
```python
from ellipse_fast import (
    generate_ellipse_heights_incremental_fast,
    heights_to_filled,
    heights_to_thin_outline,
    heights_to_full_outline,
)

# Generate heights for a 20x15 ellipse
half_heights, full_heights = generate_ellipse_heights_incremental_fast(20, 15)

print(f"Generated {len(full_heights)} columns")
print(f"Total points: {sum(full_heights)}")

# Extract outlines directly from heights (no full point-set needed)
thin = heights_to_thin_outline(full_heights)
full = heights_to_full_outline(full_heights)
print(f"Thin outline (8-connected): {len(thin)} points")
print(f"Full outline (4-connected): {len(full)} points")
```

**Run example:**
```bash
python ellipse_fast.py
```

### C# (`EllipseFast.cs`)

**Requirements:** .NET 5.0+ or .NET Framework 4.7+

**Usage:**
```csharp
using EllipseRasterization;

// Generate heights for a 20x15 ellipse
var result = EllipseFast.GenerateEllipseHeightsIncrementalFast(20, 15);

Console.WriteLine($"Generated {result.FullHeights.Count} columns");
Console.WriteLine($"Total points: {result.FullHeights.Sum()}");

// Extract outlines directly from heights
var thin = EllipseFast.HeightsToThinOutline(result.FullHeights);
var full = EllipseFast.HeightsToFullOutline(result.FullHeights);
Console.WriteLine($"Thin outline (8-connected): {thin.Count} points");
Console.WriteLine($"Full outline (4-connected): {full.Count} points");
```

**Compile and run:**
```bash
# Using dotnet CLI
csc EllipseFast.cs
EllipseFast.exe

# Or with dotnet (if you have a .csproj)
dotnet run
```

## Algorithm Details

### Input Parameters
- `two_a` / `twoA`: Full width of the ellipse (2 × semi-major axis)
- `two_b` / `twoB`: Full height of the ellipse (2 × semi-minor axis)

### Output
Returns a tuple/object containing:
1. **Half-heights**: Heights computed for the right half of the ellipse
2. **Full-heights**: Complete array of column heights for the full width

The full-heights array can be used to:
- Generate filled pixel coordinates by iterating through each column and its corresponding height.
- Extract **thin** or **full** outlines directly, without ever building the full point set.

### Outline Extraction
The outline functions (`heights_to_thin_outline` / `heights_to_full_outline`) work directly on the heights array:
- **Thin (8-connected):** Larger point set, but forms a visually thin 1-pixel-wide chain. Every outline pixel has ≤ 2 outline-neighbors in 8-connectivity.
- **Full (4-connected):** Smaller point set (subset of thin), but the boundary band is visually thicker. Every outline pixel has ≤ 2 outline-neighbors in 4-connectivity.

### Coordinate System
- Ellipse is centered at origin (0, 0)
- Supports both even and odd dimensions
- Uses normalized coordinates with proper half-pixel adjustments

### Performance Characteristics
- **Time Complexity:** O(W + H) where W = width, H = height
- **Space Complexity:** O(W) for storing column heights
- **Operations:** Pure integer arithmetic, bit shifts, incremental updates

## Integration Examples

### Python: Generate Pixel Coordinates and Outlines
```python
from ellipse_fast import (
    generate_ellipse_heights_incremental_fast,
    heights_to_filled,
    heights_to_thin_outline,
    heights_to_full_outline,
)

# Generate heights
_, full_heights = generate_ellipse_heights_incremental_fast(20, 15)

# Get filled points
points = heights_to_filled(full_heights)
print(f"Total filled points: {len(points)}")

# Get outlines (no full point-set needed!)
thin = heights_to_thin_outline(full_heights)
full = heights_to_full_outline(full_heights)
print(f"Thin outline: {len(thin)} points")
print(f"Full outline: {len(full)} points")
```

### C#: Generate Pixel Coordinates and Outlines
```csharp
using EllipseRasterization;

// Generate heights
var result = EllipseFast.GenerateEllipseHeightsIncrementalFast(20, 15);

// Get filled points
var points = EllipseFast.HeightsToFilled(result.FullHeights);
Console.WriteLine($"Total filled points: {points.Count}");

// Get outlines directly from heights
var thin = EllipseFast.HeightsToThinOutline(result.FullHeights);
var full = EllipseFast.HeightsToFullOutline(result.FullHeights);
Console.WriteLine($"Thin outline: {thin.Count} points");
Console.WriteLine($"Full outline: {full.Count} points");
```

## Testing

All ports include built-in test cases that verify:
- Correct output for various ellipse sizes
- Consistency between algorithms
- Total point counts

Run the test code in each file to validate the implementation.

## Performance Notes

### Python
- Pure Python implementation (no NumPy required)
- Suitable for moderate-sized ellipses (< 1000×1000)
- For larger ellipses or batch processing, consider Cython or NumPy vectorization

### C#
- Optimized for .NET runtime
- Suitable for large ellipses and real-time applications
- Can be further optimized with `Span<T>` and unsafe code for critical paths

## References

See the main C++ implementation in `include/ellipse_algorithms.h` for detailed algorithm documentation and additional variants.

## License

Same as the main EllipseRasterization project. See [LICENSE](../LICENSE) for details.
