#ifndef ELLIPSE_RASTERIZATION_H
#define ELLIPSE_RASTERIZATION_H

#include "ellipse_common.h"
#include <set>
#include <algorithm>
#include <cmath>
#include <vector>
#include <utility>
#include <iostream>

namespace ellipse {

// ──────────────────────────────────────────────────────────────────────
//  Helper: compute centered x-coordinates from a heights array of size W
// ──────────────────────────────────────────────────────────────────────
inline void compute_x_coords(int W, std::vector<int>& xs)
{
    int half_w = W / 2;
    xs.clear();
    xs.reserve(W);
    if (W % 2 == 1) {
        for (int i = -half_w; i <= half_w; i++) xs.push_back(i);
    } else {
        for (int i = -half_w; i < half_w; i++) xs.push_back(i);
    }
}

// Compute the y-range [y_min, y_max] (inclusive) for a column with height H.
inline std::pair<int, int> y_range(int H)
{
    int half_h = H / 2;
    if (H % 2 == 1) {
        return {-half_h, half_h};
    } else {
        return {-half_h, half_h - 1};
    }
}

// ──────────────────────────────────────────────────────────────────────
//  Filled ellipse: all interior points from heights
// ──────────────────────────────────────────────────────────────────────
inline std::vector<Point> heights_to_filled(const std::vector<int>& heights)
{
    int W = static_cast<int>(heights.size());
    if (W == 0) return {};

    std::vector<int> xs;
    compute_x_coords(W, xs);

    std::vector<Point> points;
    for (int i = 0; i < W; i++) {
        int H = heights[i];
        if (H <= 0) continue;
        int x = xs[i];
        auto [ylo, yhi] = y_range(H);
        for (int y = ylo; y <= yhi; y++) {
            points.push_back({x, y});
        }
    }
    return points;
}

// ──────────────────────────────────────────────────────────────────────
//  Full outline (4-connected boundary)
//
//  A filled pixel (x, y) is on the full outline if at least one of its
//  4 cardinal neighbors (up, down, left, right) is NOT inside the
//  filled ellipse. This produces a thicker boundary because diagonal
//  adjacency is not considered — if two outline cells touch only
//  diagonally, they don't "connect", so the outline band must be wider
//  to remain continuous in 4-connectivity.
//
//  Efficiently computed from heights in O(perimeter) without building
//  the full O(area) point set:
//    - Top/bottom of each column are always outline.
//    - Cells in column i whose y is outside the y-range of column i-1
//      or i+1 are outline (missing left/right cardinal neighbor).
// ──────────────────────────────────────────────────────────────────────
inline std::vector<Point> heights_to_full_outline(const std::vector<int>& heights)
{
    int W = static_cast<int>(heights.size());
    if (W == 0) return {};

    std::vector<int> xs;
    compute_x_coords(W, xs);

    std::vector<Point> outline;

    for (int i = 0; i < W; i++) {
        int H = heights[i];
        if (H <= 0) continue;

        int x = xs[i];
        auto [ylo, yhi] = y_range(H);

        int H_left  = (i > 0)     ? heights[i - 1] : 0;
        int H_right = (i < W - 1) ? heights[i + 1] : 0;

        auto [ylo_L, yhi_L] = (H_left  > 0) ? y_range(H_left)  : std::pair<int,int>{0, -1};
        auto [ylo_R, yhi_R] = (H_right > 0) ? y_range(H_right) : std::pair<int,int>{0, -1};

        if (H_left == 0 && H_right == 0) {
            // Isolated column — everything is outline
            for (int y = ylo; y <= yhi; y++) {
                outline.push_back({x, y});
            }
            continue;
        }

        for (int y = ylo; y <= yhi; y++) {
            bool on_outline = false;

            if (y == yhi) on_outline = true;                              // top (y+1) outside
            if (y == ylo) on_outline = true;                              // bottom (y-1) outside
            if (H_left  == 0 || y < ylo_L || y > yhi_L) on_outline = true; // left missing
            if (H_right == 0 || y < ylo_R || y > yhi_R) on_outline = true; // right missing

            if (on_outline) {
                outline.push_back({x, y});
            }
        }
    }

    return outline;
}

// ──────────────────────────────────────────────────────────────────────
//  Thin outline (8-connected boundary)
//
//  A filled pixel (x, y) is on the thin outline if at least one of its
//  8 neighbors (including diagonals) is NOT inside the filled ellipse.
//  This produces a thinner boundary because diagonal adjacency IS
//  considered — the outline chain stays 1 pixel wide.
//
//  Checks all 4 cardinal + 4 diagonal neighbors using heights:
//    - Cardinal checks are the same as the full outline.
//    - Diagonal (x±1, y±1) is inside iff column i±1 covers y±1.
// ──────────────────────────────────────────────────────────────────────
inline std::vector<Point> heights_to_thin_outline(const std::vector<int>& heights)
{
    int W = static_cast<int>(heights.size());
    if (W == 0) return {};

    std::vector<int> xs;
    compute_x_coords(W, xs);

    std::vector<Point> outline;

    for (int i = 0; i < W; i++) {
        int H = heights[i];
        if (H <= 0) continue;

        int x = xs[i];
        auto [ylo, yhi] = y_range(H);

        int H_left  = (i > 0)     ? heights[i - 1] : 0;
        int H_right = (i < W - 1) ? heights[i + 1] : 0;

        auto [ylo_L, yhi_L] = (H_left  > 0) ? y_range(H_left)  : std::pair<int,int>{0, -1};
        auto [ylo_R, yhi_R] = (H_right > 0) ? y_range(H_right) : std::pair<int,int>{0, -1};

        if (H_left == 0 && H_right == 0) {
            for (int y = ylo; y <= yhi; y++) {
                outline.push_back({x, y});
            }
            continue;
        }

        for (int y = ylo; y <= yhi; y++) {
            bool on_outline = false;

            // Cardinal neighbors
            if (y == yhi) on_outline = true;                                // top
            if (y == ylo) on_outline = true;                                // bottom
            if (H_left  == 0 || y < ylo_L || y > yhi_L) on_outline = true; // left
            if (H_right == 0 || y < ylo_R || y > yhi_R) on_outline = true; // right

            // Diagonal neighbors (only worth checking if cardinal didn't trigger)
            if (!on_outline) {
                // top-left (x-1, y+1)
                if (H_left  == 0 || (y + 1) < ylo_L || (y + 1) > yhi_L) on_outline = true;
                // bottom-left (x-1, y-1)
                if (!on_outline && (H_left  == 0 || (y - 1) < ylo_L || (y - 1) > yhi_L)) on_outline = true;
                // top-right (x+1, y+1)
                if (!on_outline && (H_right == 0 || (y + 1) < ylo_R || (y + 1) > yhi_R)) on_outline = true;
                // bottom-right (x+1, y-1)
                if (!on_outline && (H_right == 0 || (y - 1) < ylo_R || (y - 1) > yhi_R)) on_outline = true;
            }

            if (on_outline) {
                outline.push_back({x, y});
            }
        }
    }

    return outline;
}

// ──────────────────────────────────────────────────────────────────────
//  Convenience: compute everything from heights
// ──────────────────────────────────────────────────────────────────────
inline RasterizationResult rasterize_ellipse(const std::vector<int>& heights)
{
    RasterizationResult result;
    result.filled_points = heights_to_filled(heights);
    result.thin_outline  = heights_to_thin_outline(heights);
    result.thick_outline = heights_to_full_outline(heights);
    return result;
}

// ──────────────────────────────────────────────────────────────────────
//  ASCII art visualization helpers
// ──────────────────────────────────────────────────────────────────────

// Print filled ellipse with thin outline highlighted
inline void print_thin_outline_ascii(const std::vector<int>& heights)
{
    auto filled  = heights_to_filled(heights);
    auto outline = heights_to_thin_outline(heights);
    if (filled.empty()) return;

    std::set<Point> filled_set(filled.begin(), filled.end());
    std::set<Point> outline_set(outline.begin(), outline.end());

    int min_x = filled[0].first, max_x = min_x;
    int min_y = filled[0].second, max_y = min_y;
    for (const auto& [x, y] : filled) {
        min_x = std::min(min_x, x); max_x = std::max(max_x, x);
        min_y = std::min(min_y, y); max_y = std::max(max_y, y);
    }

    for (int y = max_y; y >= min_y; y--) {
        for (int x = min_x; x <= max_x; x++) {
            if (outline_set.count({x, y}))     std::cout << "\xE2\x96\x88\xE2\x96\x88";  // ██
            else if (filled_set.count({x, y})) std::cout << "\xE2\x96\x91\xE2\x96\x91";  // ░░
            else                                std::cout << "  ";
        }
        std::cout << "\n";
    }
}

// Print filled ellipse with full (4-connected) outline highlighted
inline void print_full_outline_ascii(const std::vector<int>& heights)
{
    auto filled  = heights_to_filled(heights);
    auto outline = heights_to_full_outline(heights);
    if (filled.empty()) return;

    std::set<Point> filled_set(filled.begin(), filled.end());
    std::set<Point> outline_set(outline.begin(), outline.end());

    int min_x = filled[0].first, max_x = min_x;
    int min_y = filled[0].second, max_y = min_y;
    for (const auto& [x, y] : filled) {
        min_x = std::min(min_x, x); max_x = std::max(max_x, x);
        min_y = std::min(min_y, y); max_y = std::max(max_y, y);
    }

    for (int y = max_y; y >= min_y; y--) {
        for (int x = min_x; x <= max_x; x++) {
            if (outline_set.count({x, y}))     std::cout << "\xE2\x96\x88\xE2\x96\x88";
            else if (filled_set.count({x, y})) std::cout << "\xE2\x96\x91\xE2\x96\x91";
            else                                std::cout << "  ";
        }
        std::cout << "\n";
    }
}

// Print combined view with all three layers
inline void print_combined_ascii(const std::vector<int>& heights)
{
    auto filled      = heights_to_filled(heights);
    auto thin_out    = heights_to_thin_outline(heights);
    auto full_out    = heights_to_full_outline(heights);
    if (filled.empty()) return;

    std::set<Point> filled_set(filled.begin(), filled.end());
    std::set<Point> thin_set(thin_out.begin(), thin_out.end());
    std::set<Point> full_set(full_out.begin(), full_out.end());

    int min_x = filled[0].first, max_x = min_x;
    int min_y = filled[0].second, max_y = min_y;
    for (const auto& [x, y] : filled) {
        min_x = std::min(min_x, x); max_x = std::max(max_x, x);
        min_y = std::min(min_y, y); max_y = std::max(max_y, y);
    }

    // thin_set ⊇ full_set, so: thin-only cells get one marker,
    // cells in full_set (which are also in thin) get another,
    // pure interior gets a third.
    for (int y = max_y; y >= min_y; y--) {
        for (int x = min_x; x <= max_x; x++) {
            bool is_thin = thin_set.count({x, y}) > 0;
            bool is_full = full_set.count({x, y}) > 0;
            bool is_fill = filled_set.count({x, y}) > 0;

            if (is_full) {
                // Part of both outlines (full ⊆ thin)
                std::cout << "\xE2\x96\x88\xE2\x96\x88";  // ██
            } else if (is_thin) {
                // Thin outline only (diagonal-triggered)
                std::cout << "\xE2\x96\x93\xE2\x96\x93";  // ▓▓
            } else if (is_fill) {
                std::cout << "\xE2\x96\x91\xE2\x96\x91";  // ░░
            } else {
                std::cout << "  ";
            }
        }
        std::cout << "\n";
    }
}

} // namespace ellipse

#endif // ELLIPSE_RASTERIZATION_H
