#include <iostream>
#include <vector>
#include "ellipse_algorithms.h"

int main() {
    using namespace ellipse;
    
    std::cout << "Testing axis-flip algorithm...\n";
    
    // Test adaptive reference
    std::cout << "\nTest Adaptive (reference): 20x15\n";
    try {
        auto [half_ref, full_ref] = generate_ellipse_heights_adaptive(20, 15);
        std::cout << "  Adaptive full_heights (" << full_ref.size() << "): [";
        for (size_t i = 0; i < full_ref.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << full_ref[i];
        }
        std::cout << "]\n";
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
    }
    
    // Test 1: 10x10 (square - should use direct path)
    std::cout << "\nTest 1: 10x10\n";
    try {
        auto [half1, full1] = generate_ellipse_heights_incremental_fast(10, 10);
        auto [half2, full2] = generate_ellipse_heights_incremental_fast_axis_flip(10, 10);
        bool match = (half1 == half2 && full1 == full2);
        std::cout << "Square ellipse: " << (match ? "PASS" : "FAIL") << "\n";
        if (!match) {
            std::cout << "  IncrementalFast half_heights size: " << half1.size() << "\n";
            std::cout << "  AxisFlip half_heights size: " << half2.size() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        return 1;
    }
    
    // Test 2: 20x15 (wide - should swap axes)
    std::cout << "\nTest 2: 20x15 (wide)\n";
    try {
        auto [half1, full1] = generate_ellipse_heights_incremental_fast(20, 15);
        auto [half2, full2] = generate_ellipse_heights_incremental_fast_axis_flip(20, 15);
        
        // Also show swapped computation
        auto [swap_half, swap_full] = generate_ellipse_heights_incremental_fast(15, 20);
        std::cout << "  Swapped (15x20) half_heights size: " << swap_half.size() << ", full_heights size: " << swap_full.size() << "\n";
        std::cout << "  Swapped full_heights: [";
        for (size_t i = 0; i < swap_full.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << swap_full[i];
        }
        std::cout << "]\n";
        
        std::cout << "  IncrementalFast full_heights (" << full1.size() << "): [";
        for (size_t i = 0; i < full1.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << full1[i];
        }
        std::cout << "]\n";
        
        std::cout << "  AxisFlip full_heights (" << full2.size() << "): [";
        for (size_t i = 0; i < full2.size(); i++) {
            if (i > 0) std::cout << ", ";
            std::cout << full2[i];
        }
        std::cout << "]\n";
        
        bool match = (half1 == half2 && full1 == full2);
        std::cout << "Wide ellipse: " << (match ? "PASS" : "FAIL") << "\n";
        
        if (!match) {
            std::cout << "  Differences at indices: ";
            for (size_t i = 0; i < std::min(full1.size(), full2.size()); i++) {
                if (full1[i] != full2[i]) {
                    std::cout << i << " (" << full1[i] << " vs " << full2[i] << ") ";
                }
            }
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\nAll tests completed.\n";
    return 0;
}
