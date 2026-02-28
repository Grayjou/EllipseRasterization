#include <iostream>
#include "../include/ellipse_algorithms.h"
#include "../include/ellipse_rasterization.h"

int main() {
    using namespace ellipse;
    
    // Example: 30x20 ellipse
    int two_a = 30;  // width
    int two_b = 20;  // height
    
    std::cout << "Rasterizing " << two_a << "x" << two_b << " ellipse...\n\n";
    
    // Generate heights using fastest algorithm
    auto [half_heights, full_heights] = generate_ellipse_heights_incremental_reverse(two_a, two_b);
    
    // Convert to point coordinates
    auto raster = rasterize_ellipse(full_heights);
    
    // Display results
    std::cout << "Results:\n";
    std::cout << "  Filled points: " << raster.filled_points.size() << "\n";
    std::cout << "  Thin outline (8-way): " << raster.thin_outline.size() << "\n";
    std::cout << "  Thick outline (4-way): " << raster.thick_outline.size() << "\n\n";
    
    // Show first few points
    std::cout << "First 10 filled points:\n";
    for (size_t i = 0; i < std::min(size_t(10), raster.filled_points.size()); i++) {
        auto [x, y] = raster.filled_points[i];
        std::cout << "  (" << x << ", " << y << ")\n";
    }
    
    // Visualize (small ellipse only)
    if (two_a <= 40 && two_b <= 30) {
        std::set<Point> filled_set(raster.filled_points.begin(), raster.filled_points.end());
        std::set<Point> thin_set(raster.thin_outline.begin(), raster.thin_outline.end());
        
        int min_x = raster.filled_points[0].first;
        int max_x = raster.filled_points[0].first;
        int min_y = raster.filled_points[0].second;
        int max_y = raster.filled_points[0].second;
        
        for (const auto& [x, y] : raster.filled_points) {
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
        
        std::cout << "\nVisualization (█=outline, ░=interior):\n\n";
        for (int y = max_y; y >= min_y; y--) {
            for (int x = min_x; x <= max_x; x++) {
                if (thin_set.count({x, y})) {
                    std::cout << "██";
                } else if (filled_set.count({x, y})) {
                    std::cout << "░░";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << "\n";
        }
    }
    
    return 0;
}
