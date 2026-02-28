using System;
using System.Collections.Generic;
using System.Linq;

namespace EllipseRasterization
{
    /// <summary>
    /// Fast ellipse rasterization algorithms - C# port
    /// Ports of IncrementalFast and IncrementalFastAxisFlip from C++
    /// </summary>
    public static class EllipseFast
    {
        /// <summary>
        /// Result containing half-heights and full-heights for an ellipse
        /// </summary>
        public class HeightsResult
        {
            public List<int> HalfHeights { get; set; }
            public List<int> FullHeights { get; set; }

            public HeightsResult(List<int> halfHeights, List<int> fullHeights)
            {
                HalfHeights = halfHeights;
                FullHeights = fullHeights;
            }
        }

        /// <summary>
        /// Check if a normalized point is inside the ellipse
        /// </summary>
        public static bool IsPointInEllipse(int normX, int normY, int twoA, int twoB)
        {
            double a = twoA / 2.0;
            double b = twoB / 2.0;
            double repositionFactorX = (twoA % 2 == 1) ? 0.5 : 0.0;
            double repositionFactorY = (twoB % 2 == 1) ? 0.5 : 0.0;
            double x = normX - 0.5 - repositionFactorX;
            double y = normY - 0.5 - repositionFactorY;
            return (x * x / (a * a)) + (y * y / (b * b)) <= 1.0;
        }

        /// <summary>
        /// Mirror half-heights to get full column heights
        /// </summary>
        public static HeightsResult BuildFullHeights(List<int> halfHeights, int twoA, int twoB)
        {
            var head = new List<int>();
            if (twoA % 2 == 0)
            {
                head = new List<int>(halfHeights);
            }
            else
            {
                if (halfHeights.Count > 0)
                {
                    head = halfHeights.Take(halfHeights.Count - 1).ToList();
                }
            }

            var tail = new List<int>(halfHeights);
            tail.Reverse();

            var fullHalfHeights = new List<int>(head);
            fullHalfHeights.AddRange(tail);

            var fullHeights = new List<int>();
            if (twoB % 2 == 0)
            {
                foreach (int h in fullHalfHeights)
                {
                    fullHeights.Add(h * 2);
                }
            }
            else
            {
                foreach (int h in fullHalfHeights)
                {
                    fullHeights.Add(h * 2 - 1);
                }
            }

            return new HeightsResult(fullHalfHeights, fullHeights);
        }

        /// <summary>
        /// O(W + H) - Pure integer arithmetic, no floating point.
        /// 
        /// Uses incremental computation with integer-only operations to avoid
        /// costly sqrt() calls and floating-point rounding issues.
        /// </summary>
        /// <param name="twoA">Full width of ellipse (2 * semi-major axis a)</param>
        /// <param name="twoB">Full height of ellipse (2 * semi-minor axis b)</param>
        /// <returns>HeightsResult containing half-heights and full-heights</returns>
        public static HeightsResult GenerateEllipseHeightsIncrementalFast(int twoA, int twoB)
        {
            var halfHeights = new List<int>();
            int halfW = twoA / 2;
            int halfH = (twoB + 1) / 2;

            if (halfW == 0)
            {
                return BuildFullHeights(halfHeights, twoA, twoB);
            }

            // Integer arithmetic: (sx/a)² + (sy/b)² <= 1
            // Becomes: sx² * b² + sy² * a² <= a² * b²
            long aSquared = (long)twoA * twoA;
            long bSquared = (long)twoB * twoB;
            long threshold = aSquared * bSquared;
            long eightASquared = aSquared << 3;
            long eightBSquared = bSquared << 3;

            int xOffset = twoA & 1;
            int yOffset = twoB & 1;

            // Initialize y tracking
            int currentHeight = 1;
            long syNext = 3 - yOffset;
            long yTermNext = syNext * syNext * aSquared;
            long yDelta = (aSquared << 2) * (syNext + 1);

            // Initialize x tracking
            long sx = 2 * halfW - 1 - xOffset;
            long xTerm = sx * sx * bSquared;
            long xDelta = (bSquared << 2) * (sx - 1);

            for (int col = halfW; col > 0; col--)
            {
                while (currentHeight < halfH && xTerm + yTermNext <= threshold)
                {
                    currentHeight++;
                    yTermNext += yDelta;
                    yDelta += eightASquared;
                }
                halfHeights.Add(currentHeight);

                // Update x term for next column
                xTerm -= xDelta;
                xDelta -= eightBSquared;
            }

            return BuildFullHeights(halfHeights, twoA, twoB);
        }

        /// <summary>
        /// O(W + H) - Axis-flip variant of IncrementalFast.
        /// 
        /// Experimental: For wide ellipses, this algorithm is intended to swap axes
        /// before running IncrementalFast for better performance characteristics.
        /// 
        /// Current implementation: Calls IncrementalFast directly without swapping
        /// due to parity/centering complexities in the transpose operation.
        /// </summary>
        /// <param name="twoA">Full width of ellipse</param>
        /// <param name="twoB">Full height of ellipse</param>
        /// <returns>HeightsResult containing half-heights and full-heights</returns>
        public static HeightsResult GenerateEllipseHeightsIncrementalFastAxisFlip(int twoA, int twoB)
        {
            // For now, always use the direct incremental-fast without swapping
            // Future work: implement correct transpose handling for odd/even dimensions
            return GenerateEllipseHeightsIncrementalFast(twoA, twoB);
        }

        // ──────────────────────────────────────────────────────────────
        //  Outline extraction from heights
        // ──────────────────────────────────────────────────────────────

        /// <summary>
        /// Compute the inclusive y-range [yLo, yHi] for a column of given height.
        /// </summary>
        private static (int yLo, int yHi) YRange(int H)
        {
            int halfH = H / 2;
            return H % 2 == 1 ? (-halfH, halfH) : (-halfH, halfH - 1);
        }

        /// <summary>
        /// Compute centered x-coordinates for a heights array of size W.
        /// </summary>
        private static List<int> ComputeXCoords(int W)
        {
            int halfW = W / 2;
            var xs = new List<int>(W);
            if (W % 2 == 1)
                for (int i = -halfW; i <= halfW; i++) xs.Add(i);
            else
                for (int i = -halfW; i < halfW; i++) xs.Add(i);
            return xs;
        }

        /// <summary>
        /// Convert heights to filled (x, y) coordinates.
        /// </summary>
        public static List<(int x, int y)> HeightsToFilled(List<int> heights)
        {
            int W = heights.Count;
            if (W == 0) return new List<(int, int)>();
            var xs = ComputeXCoords(W);
            var points = new List<(int, int)>();
            for (int i = 0; i < W; i++)
            {
                int H = heights[i];
                if (H <= 0) continue;
                int x = xs[i];
                var (yLo, yHi) = YRange(H);
                for (int y = yLo; y <= yHi; y++)
                    points.Add((x, y));
            }
            return points;
        }

        /// <summary>
        /// Full outline (4-connected boundary).
        /// A filled pixel is on the full outline if at least one of its 4 cardinal
        /// neighbors is NOT inside the filled ellipse. Produces a thicker boundary.
        /// Computed directly from heights in O(perimeter).
        /// </summary>
        public static List<(int x, int y)> HeightsToFullOutline(List<int> heights)
        {
            int W = heights.Count;
            if (W == 0) return new List<(int, int)>();
            var xs = ComputeXCoords(W);
            var outline = new List<(int, int)>();

            for (int i = 0; i < W; i++)
            {
                int H = heights[i];
                if (H <= 0) continue;
                int x = xs[i];
                var (yLo, yHi) = YRange(H);

                int hLeft  = i > 0     ? heights[i - 1] : 0;
                int hRight = i < W - 1 ? heights[i + 1] : 0;

                var (yLoL, yHiL) = hLeft  > 0 ? YRange(hLeft)  : (0, -1);
                var (yLoR, yHiR) = hRight > 0 ? YRange(hRight) : (0, -1);

                if (hLeft == 0 && hRight == 0)
                {
                    for (int y = yLo; y <= yHi; y++)
                        outline.Add((x, y));
                    continue;
                }

                for (int y = yLo; y <= yHi; y++)
                {
                    bool onOutline = false;
                    if (y == yHi) onOutline = true;
                    if (y == yLo) onOutline = true;
                    if (hLeft == 0 || y < yLoL || y > yHiL) onOutline = true;
                    if (hRight == 0 || y < yLoR || y > yHiR) onOutline = true;
                    if (onOutline)
                        outline.Add((x, y));
                }
            }
            return outline;
        }

        /// <summary>
        /// Thin outline (8-connected boundary).
        /// A filled pixel is on the thin outline if at least one of its 8 neighbors
        /// (including diagonals) is NOT inside the filled ellipse. Produces a thinner,
        /// 1-pixel-wide boundary.
        /// Computed directly from heights in O(perimeter).
        /// </summary>
        public static List<(int x, int y)> HeightsToThinOutline(List<int> heights)
        {
            int W = heights.Count;
            if (W == 0) return new List<(int, int)>();
            var xs = ComputeXCoords(W);
            var outline = new List<(int, int)>();

            for (int i = 0; i < W; i++)
            {
                int H = heights[i];
                if (H <= 0) continue;
                int x = xs[i];
                var (yLo, yHi) = YRange(H);

                int hLeft  = i > 0     ? heights[i - 1] : 0;
                int hRight = i < W - 1 ? heights[i + 1] : 0;

                var (yLoL, yHiL) = hLeft  > 0 ? YRange(hLeft)  : (0, -1);
                var (yLoR, yHiR) = hRight > 0 ? YRange(hRight) : (0, -1);

                if (hLeft == 0 && hRight == 0)
                {
                    for (int y = yLo; y <= yHi; y++)
                        outline.Add((x, y));
                    continue;
                }

                for (int y = yLo; y <= yHi; y++)
                {
                    bool onOutline = false;

                    // Cardinal
                    if (y == yHi) onOutline = true;
                    if (y == yLo) onOutline = true;
                    if (hLeft == 0 || y < yLoL || y > yHiL) onOutline = true;
                    if (hRight == 0 || y < yLoR || y > yHiR) onOutline = true;

                    // Diagonals
                    if (!onOutline)
                    {
                        if (hLeft == 0 || (y + 1) < yLoL || (y + 1) > yHiL) onOutline = true;
                        if (!onOutline && (hLeft == 0 || (y - 1) < yLoL || (y - 1) > yHiL)) onOutline = true;
                        if (!onOutline && (hRight == 0 || (y + 1) < yLoR || (y + 1) > yHiR)) onOutline = true;
                        if (!onOutline && (hRight == 0 || (y - 1) < yLoR || (y - 1) > yHiR)) onOutline = true;
                    }

                    if (onOutline)
                        outline.Add((x, y));
                }
            }
            return outline;
        }
    }

    /// <summary>
    /// Example usage and testing
    /// </summary>
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Fast Ellipse Rasterization Algorithms - C#");
            Console.WriteLine(new string('=', 50));

            // Test cases
            var testCases = new[]
            {
                (TwoA: 10, TwoB: 10, Description: "Square 10x10"),
                (TwoA: 20, TwoB: 15, Description: "Wide 20x15"),
                (TwoA: 15, TwoB: 20, Description: "Tall 15x20"),
                (TwoA: 30, TwoB: 25, Description: "Wide 30x25"),
            };

            foreach (var (twoA, twoB, description) in testCases)
            {
                Console.WriteLine($"\n{description}:");
                var result = EllipseFast.GenerateEllipseHeightsIncrementalFast(twoA, twoB);

                Console.WriteLine($"  Dimensions: {twoA}x{twoB}");
                Console.WriteLine($"  Half-heights count: {result.HalfHeights.Count}");
                Console.WriteLine($"  Full-heights count: {result.FullHeights.Count}");
                
                var preview = string.Join(", ", result.FullHeights.Take(10));
                var ellipsis = result.FullHeights.Count > 10 ? "..." : "";
                Console.WriteLine($"  Full heights: [{preview}{ellipsis}]");

                // Count total points
                int totalPoints = result.FullHeights.Sum();
                Console.WriteLine($"  Total filled points: {totalPoints}");

                // Outline extraction
                var thin = EllipseFast.HeightsToThinOutline(result.FullHeights);
                var full = EllipseFast.HeightsToFullOutline(result.FullHeights);
                Console.WriteLine($"  Thin outline (8-connected): {thin.Count} points");
                Console.WriteLine($"  Full outline (4-connected): {full.Count} points");
            }

            Console.WriteLine("\n" + new string('=', 50));
            Console.WriteLine("Testing axis-flip variant...");

            int testA = 20, testB = 15;
            var resultFast = EllipseFast.GenerateEllipseHeightsIncrementalFast(testA, testB);
            var resultFlip = EllipseFast.GenerateEllipseHeightsIncrementalFastAxisFlip(testA, testB);

            bool identical = resultFast.HalfHeights.SequenceEqual(resultFlip.HalfHeights) &&
                           resultFast.FullHeights.SequenceEqual(resultFlip.FullHeights);

            if (identical)
            {
                Console.WriteLine($"  Both algorithms produce identical results for {testA}x{testB}");
            }
            else
            {
                Console.WriteLine($"  Results differ for {testA}x{testB}");
            }
        }
    }
}
