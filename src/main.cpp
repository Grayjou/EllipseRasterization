#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <cmath>
#include "ellipse_algorithms.h"
#include "ellipse_rasterization.h"

// Benchmark function wrapper
double benchmark_approach(
    std::function<HeightsResult(int, int)> approach_func,
    int two_a, int two_b, int num_runs = 10)
{
    std::vector<double> times;
    for (int i = 0; i < num_runs; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = approach_func(two_a, two_b);
        auto end = std::chrono::high_resolution_clock::now();
        
        double elapsed_us = std::chrono::duration<double, std::micro>(end - start).count();
        times.push_back(elapsed_us);
    }
    
    // Return average
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    return sum / times.size();
}

// Verify all approaches produce consistent results
void verify_consistency()
{
    using namespace ellipse;
    
    std::map<std::string, std::function<HeightsResult(int, int)>> approaches = {
        {"Direct", generate_ellipse_heights_direct},
        {"Incremental", generate_ellipse_heights_incremental},
        {"IncrementalFast", generate_ellipse_heights_incremental_fast},
        {"Hybrid", generate_ellipse_heights_hybrid},
        {"IncrementalReverse", generate_ellipse_heights_incremental_reverse},
        {"IncrementalReverseFast", generate_ellipse_heights_incremental_reverse_fast}
    };

    std::vector<std::pair<int, int>> test_sizes = {{10, 10}, {20, 15}, {30, 25}};

    for (const auto& [two_a, two_b] : test_sizes) {
        std::cout << "\nVerifying consistency for " << two_a << "x" << two_b << ":\n";
        std::map<std::string, HeightsResult> results;
        
        for (const auto& [name, func] : approaches) {
            try {
                auto result = func(two_a, two_b);
                results[name] = result;
                std::cout << "  " << name << ": ";
                std::cout << "half_heights=[";
                for (size_t i = 0; i < result.first.size(); i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << result.first[i];
                }
                std::cout << "], full_heights=[";
                for (size_t i = 0; i < result.second.size() && i < 10; i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << result.second[i];
                }
                if (result.second.size() > 10) std::cout << ", ...";
                std::cout << "]\n";
            } catch (const std::exception& e) {
                std::cout << "  " << name << ": Error - " << e.what() << "\n";
            }
        }

        // Check if all results are the same
        if (!results.empty()) {
            const auto& first_result = results.begin()->second;
            bool all_same = true;
            for (const auto& [name, result] : results) {
                if (result.first != first_result.first || result.second != first_result.second) {
                    all_same = false;
                    break;
                }
            }
            
            if (all_same) {
                std::cout << "  ✓ All approaches produce identical results\n";
            } else {
                std::cout << "  ✗ Approaches produce different results!\n";
            }
        }
    }
}

// Demonstrate rasterization
void demonstrate_rasterization()
{
    using namespace ellipse;
    
    std::cout << "\n\n=== Rasterization Demonstration ===\n\n";
    
    int two_a = 20;
    int two_b = 12;
    
    auto [half_heights, full_heights] = generate_ellipse_heights_direct(two_a, two_b);
    auto raster = rasterize_ellipse(full_heights);
    
    std::cout << "Ellipse: " << two_a << "x" << two_b << "\n";
    std::cout << "Total filled points: " << raster.filled_points.size() << "\n";
    std::cout << "Thin outline points (8-way): " << raster.thin_outline.size() << "\n";
    std::cout << "Thick outline points (4-way): " << raster.thick_outline.size() << "\n";
    
    // Print a few sample points
    std::cout << "\nFirst 10 filled points:\n";
    for (size_t i = 0; i < std::min(size_t(10), raster.filled_points.size()); i++) {
        const auto& [x, y] = raster.filled_points[i];
        std::cout << "  (" << x << ", " << y << ")\n";
    }
    
    std::cout << "\nFirst 10 thin outline points:\n";
    for (size_t i = 0; i < std::min(size_t(10), raster.thin_outline.size()); i++) {
        const auto& [x, y] = raster.thin_outline[i];
        std::cout << "  (" << x << ", " << y << ")\n";
    }
    
    // Visualization
    if (!raster.filled_points.empty()) {
        int min_x = raster.filled_points[0].first;
        int max_x = raster.filled_points[0].first;
        int min_y = raster.filled_points[0].second;
        int max_y = raster.filled_points[0].second;
        
        std::set<Point> filled_set(raster.filled_points.begin(), raster.filled_points.end());
        std::set<Point> thin_set(raster.thin_outline.begin(), raster.thin_outline.end());
        std::set<Point> thick_set(raster.thick_outline.begin(), raster.thick_outline.end());
        
        for (const auto& [x, y] : raster.filled_points) {
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
        
        std::cout << "\nVisualization (█=thin outline, ▓=thick outline, ░=interior):\n\n";
        for (int y = max_y; y >= min_y; y--) {
            for (int x = min_x; x <= max_x; x++) {
                if (thin_set.count({x, y})) {
                    std::cout << "█ ";
                } else if (thick_set.count({x, y})) {
                    std::cout << "▓ ";
                } else if (filled_set.count({x, y})) {
                    std::cout << "░ ";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << "\n";
        }
    }
}

int main(int argc, char* argv[])
{
    using namespace ellipse;
    
    std::cout << "=== Ellipse Rasterization Benchmark (C++) ===\n\n";

    std::cout << "Verifying consistency of approaches...\n";
    verify_consistency();

    // Benchmark setup
    std::vector<int> sizes;
    for (int i = 1; i <= 401; i += 20) {
        sizes.push_back(i);
    }

    std::map<std::string, std::function<HeightsResult(int, int)>> approaches = {
        {"Direct", generate_ellipse_heights_direct},
        {"Incremental", generate_ellipse_heights_incremental},
        {"IncrementalFast", generate_ellipse_heights_incremental_fast},
        {"Hybrid", generate_ellipse_heights_hybrid},
        {"IncrementalReverse", generate_ellipse_heights_incremental_reverse},
        {"IncrementalReverseFast", generate_ellipse_heights_incremental_reverse_fast}
    };

    // Perform benchmarks
    std::map<std::string, std::vector<std::vector<double>>> results;
    for (const auto& [name, func] : approaches) {
        results[name] = std::vector<std::vector<double>>(sizes.size(), 
                                                        std::vector<double>(sizes.size(), 0.0));
    }

    int num_benchmark_runs = 100;  // More runs for statistical significance
    std::cout << "\nStarting benchmarks (" << num_benchmark_runs << " full runs)...\n";

    for (int run = 0; run < num_benchmark_runs; run++) {
        std::cout << "Run " << (run + 1) << "/" << num_benchmark_runs << "\n";

        for (size_t i = 0; i < sizes.size(); i++) {
            for (size_t j = 0; j < sizes.size(); j++) {
                int two_a = sizes[i];
                int two_b = sizes[j];

                for (const auto& [name, func] : approaches) {
                    try {
                        double time_taken = benchmark_approach(func, two_a, two_b);
                        results[name][i][j] += time_taken;
                    } catch (const std::exception& e) {
                        results[name][i][j] += std::numeric_limits<double>::quiet_NaN();
                    }
                }
            }
        }
    }

    // Average the results
    for (auto& [name, data] : results) {
        for (auto& row : data) {
            for (auto& val : row) {
                val /= num_benchmark_runs;
            }
        }
    }

    std::cout << "\nBenchmarks complete.\n";

    // Save results to CSV
    std::string results_dir = "results";
    std::string csv_path = results_dir + "/benchmark_results.csv";
    std::ofstream csv_file(csv_path);
    if (csv_file.is_open()) {
        csv_file << "Algorithm,Width,Height,Time_us\n";
        for (const auto& [name, data] : results) {
            for (size_t i = 0; i < sizes.size(); i++) {
                for (size_t j = 0; j < sizes.size(); j++) {
                    csv_file << name << "," << sizes[i] << "," << sizes[j] << "," 
                             << std::fixed << std::setprecision(4) << data[i][j] << "\n";
                }
            }
        }
        csv_file.close();
        std::cout << "Results saved to " << csv_path << "\n";
    } else {
        std::cerr << "Warning: Could not create CSV file at " << csv_path << "\n";
    }

    // Print results
    for (const auto& [name, data] : results) {
        std::cout << "\n" << name << " Approach Times (microseconds, averaged over " 
                  << num_benchmark_runs << " runs):\n";
        std::cout << "Size\t";
        for (int s : sizes) {
            std::cout << s << "\t";
        }
        std::cout << "\n";

        for (size_t i = 0; i < sizes.size(); i++) {
            std::cout << sizes[i] << "\t";
            for (size_t j = 0; j < sizes.size(); j++) {
                double time = data[i][j];
                if (std::isnan(time)) {
                    std::cout << "NaN\t";
                } else {
                    std::cout << std::fixed << std::setprecision(2) << time << "\t";
                }
            }
            std::cout << "\n";
        }
    }

    // Run rasterization demonstration
    demonstrate_rasterization();

    return 0;
}
