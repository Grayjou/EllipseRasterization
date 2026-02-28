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
    auto [half_heights, full_heights] = generate_ellipse_heights_incremental_fast(two_a, two_b);
    
    // ── Point counts ──
    auto raster = rasterize_ellipse(full_heights);
    
    std::cout << "Results:\n";
    std::cout << "  Filled points:               " << raster.filled_points.size() << "\n";
    std::cout << "  Thin outline  (8-connected):  " << raster.thin_outline.size()  << "\n";
    std::cout << "  Full outline  (4-connected):  " << raster.thick_outline.size() << "\n\n";
    
    // ── Thin outline (8-connected) ──
    std::cout << "=== Thin outline (8-connected) ===\n";
    std::cout << "Every outline cell has at most 2 outline-neighbors in 8 directions.\n";
    std::cout << "This produces a 1-pixel-wide boundary.\n\n";
    print_thin_outline_ascii(full_heights);
    
    // ── Full outline (4-connected) ──
    std::cout << "\n=== Full outline (4-connected) ===\n";
    std::cout << "Boundary defined by missing cardinal (N/S/E/W) neighbors.\n";
    std::cout << "This produces a thicker boundary where diagonals create gaps.\n\n";
    print_full_outline_ascii(full_heights);
    
    // ── Combined view ──
    std::cout << "\n=== Combined view ===\n";
    std::cout << "Legend: solid=full outline, medium=thin-only, light=interior\n\n";
    print_combined_ascii(full_heights);
    
    // ── Standalone outline functions (no full rasterization needed) ──
    std::cout << "\n=== Direct outline from heights (no point-set needed) ===\n";
    auto thin = heights_to_thin_outline(full_heights);
    auto full = heights_to_full_outline(full_heights);
    std::cout << "  heights_to_thin_outline() -> " << thin.size() << " points\n";
    std::cout << "  heights_to_full_outline() -> " << full.size() << " points\n";
    
    // Show that full outline is a subset of thin outline
    std::set<Point> thin_set(thin.begin(), thin.end());
    bool subset = true;
    for (const auto& pt : full) {
        if (thin_set.find(pt) == thin_set.end()) { subset = false; break; }
    }
    std::cout << "  full_outline subset of thin_outline: " << (subset ? "yes" : "no") << "\n";
    
    // ── Small ellipse for detailed view ──
    std::cout << "\n=== Small ellipse 10x8 ===\n\n";
    auto [hh2, fh2] = generate_ellipse_heights_incremental_fast(10, 8);
    
    std::cout << "Heights: [";
    for (size_t i = 0; i < fh2.size(); i++) {
        if (i > 0) std::cout << ", ";
        std::cout << fh2[i];
    }
    std::cout << "]\n\n";
    
    std::cout << "Thin outline:\n";
    print_thin_outline_ascii(fh2);
    
    std::cout << "\nFull outline:\n";
    print_full_outline_ascii(fh2);
    
    return 0;
}
