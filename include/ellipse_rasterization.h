#ifndef ELLIPSE_RASTERIZATION_H
#define ELLIPSE_RASTERIZATION_H

#include "ellipse_common.h"
#include <set>
#include <algorithm>
#include <cmath>

namespace ellipse {

// Convert heights to point coordinates with outlines
inline RasterizationResult rasterize_ellipse(const std::vector<int>& heights)
{
    RasterizationResult result;
    
    int W = heights.size();
    if (W == 0) return result;

    // Generate x-coordinates centered at 0
    int half_w = W / 2;
    std::vector<int> xs;
    if (W % 2 == 1) {
        for (int i = -half_w; i <= half_w; i++) xs.push_back(i);
    } else {
        for (int i = -half_w; i < half_w; i++) xs.push_back(i);
    }

    // Build filled set
    std::set<Point> inside_set;
    for (int i = 0; i < W; i++) {
        int H = heights[i];
        if (H <= 0) continue;
        
        int x = xs[i];
        int half_h = H / 2;
        
        if (H % 2 == 1) {
            for (int y = -half_h; y <= half_h; y++) {
                inside_set.insert({x, y});
            }
        } else {
            for (int y = -half_h; y < half_h; y++) {
                inside_set.insert({x, y});
            }
        }
    }

    // Copy filled points to result
    result.filled_points.assign(inside_set.begin(), inside_set.end());

    // Thin outline: 8-way connectivity (point has at least one 8-neighbor outside)
    const int neighbors8[] = {-1,-1, -1,0, -1,1, 0,-1, 0,1, 1,-1, 1,0, 1,1};
    std::set<Point> thin_outline_set;
    for (const auto& pt : inside_set) {
        int x = pt.first;
        int y = pt.second;
        for (int i = 0; i < 16; i += 2) {
            int nx = x + neighbors8[i];
            int ny = y + neighbors8[i + 1];
            if (inside_set.find({nx, ny}) == inside_set.end()) {
                thin_outline_set.insert(pt);
                break;
            }
        }
    }

    // Thick outline: 4-way connectivity (point has at least one 4-neighbor outside)
    const int neighbors4[] = {-1,0, 1,0, 0,-1, 0,1};
    std::set<Point> thick_outline_set;
    for (const auto& pt : inside_set) {
        int x = pt.first;
        int y = pt.second;
        for (int i = 0; i < 8; i += 2) {
            int nx = x + neighbors4[i];
            int ny = y + neighbors4[i + 1];
            if (inside_set.find({nx, ny}) == inside_set.end()) {
                thick_outline_set.insert(pt);
                break;
            }
        }
    }

    result.thin_outline.assign(thin_outline_set.begin(), thin_outline_set.end());
    result.thick_outline.assign(thick_outline_set.begin(), thick_outline_set.end());

    // Sort for consistent output
    std::sort(result.filled_points.begin(), result.filled_points.end(),
              [](const Point& a, const Point& b) {
                  return a.first != b.first ? a.first < b.first : a.second < b.second;
              });

    std::sort(result.thin_outline.begin(), result.thin_outline.end(),
              [](const Point& a, const Point& b) {
                  double angle_a = std::atan2(a.second, a.first);
                  double angle_b = std::atan2(b.second, b.first);
                  return angle_a < angle_b;
              });

    std::sort(result.thick_outline.begin(), result.thick_outline.end(),
              [](const Point& a, const Point& b) {
                  double angle_a = std::atan2(a.second, a.first);
                  double angle_b = std::atan2(b.second, b.first);
                  return angle_a < angle_b;
              });

    return result;
}

} // namespace ellipse

#endif // ELLIPSE_RASTERIZATION_H
