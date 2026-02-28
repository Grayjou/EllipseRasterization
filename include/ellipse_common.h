#ifndef ELLIPSE_COMMON_H
#define ELLIPSE_COMMON_H

#include <vector>
#include <cmath>
#include <utility>
#include <set>

using Point = std::pair<int, int>;
using HeightsResult = std::pair<std::vector<int>, std::vector<int>>;

inline bool is_point_in_ellipse(int norm_x, int norm_y, int two_a, int two_b)
{
    double a = two_a / 2.0;
    double b = two_b / 2.0;
    double reposition_factor_x = (two_a % 2 == 1) ? 0.5 : 0.0;
    double reposition_factor_y = (two_b % 2 == 1) ? 0.5 : 0.0;
    double x = norm_x - 0.5 - reposition_factor_x;
    double y = norm_y - 0.5 - reposition_factor_y;
    return (x * x / (a * a)) + (y * y / (b * b)) <= 1.0;
}

inline HeightsResult build_full_heights(const std::vector<int>& half_heights, int two_a, int two_b)
{
    std::vector<int> head;
    if (two_a % 2 == 0) {
        head = half_heights;
    } else {
        if (!half_heights.empty()) {
            head.assign(half_heights.begin(), half_heights.end() - 1);
        }
    }
    
    std::vector<int> tail(half_heights.rbegin(), half_heights.rend());
    std::vector<int> full_half_heights = head;
    full_half_heights.insert(full_half_heights.end(), tail.begin(), tail.end());
    
    std::vector<int> full_heights;
    if (two_b % 2 == 0) {
        for (int h : full_half_heights) {
            full_heights.push_back(h * 2);
        }
    } else {
        for (int h : full_half_heights) {
            full_heights.push_back(h * 2 - 1);
        }
    }
    
    return {full_half_heights, full_heights};
}

// Structure to hold rasterization results
struct RasterizationResult {
    std::vector<Point> filled_points;
    std::vector<Point> thin_outline;  // 8-way connectivity
    std::vector<Point> thick_outline; // 4-way connectivity
};

#endif // ELLIPSE_COMMON_H
