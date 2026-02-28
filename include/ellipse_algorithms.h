#ifndef ELLIPSE_ALGORITHMS_H
#define ELLIPSE_ALGORITHMS_H

#include "ellipse_common.h"
#include <algorithm>
#include <cmath>

namespace ellipse {

// Direct approach: O(W) - Direct computation using ellipse equation
inline HeightsResult generate_ellipse_heights_direct(int two_a, int two_b)
{
    double a = two_a / 2.0;
    double b = two_b / 2.0;
    double reposition_factor_x = (two_a % 2 == 1) ? 0.5 : 0.0;
    double reposition_factor_y = (two_b % 2 == 1) ? 0.5 : 0.0;
    std::vector<int> half_heights;

    for (int col = static_cast<int>(a); col > 0; col--) {
        double x = col - 0.5 - reposition_factor_x;
        double x_normalized = x / a;
        if (x_normalized >= 1.0) {
            half_heights.push_back(0);
            continue;
        }

        double y_boundary = b * std::sqrt(1.0 - x_normalized * x_normalized);
        int max_y = static_cast<int>(std::floor(y_boundary + 0.5 + reposition_factor_y));
        half_heights.push_back(std::max(1, max_y));
    }

    return build_full_heights(half_heights, two_a, two_b);
}

// Incremental approach: O(W + H) - Cleaner incremental approach
inline HeightsResult generate_ellipse_heights_incremental(int two_a, int two_b)
{
    std::vector<int> half_heights;
    int half_w = two_a / 2;
    int current_height = 1;

    for (int x = half_w; x > 0; x--) {
        while (is_point_in_ellipse(x, current_height + 1, two_a, two_b)) {
            current_height++;
        }
        half_heights.push_back(current_height);
    }

    return build_full_heights(half_heights, two_a, two_b);
}

// Hybrid approach: O(W + H) - Uses derivative bounds for tighter search
inline HeightsResult generate_ellipse_heights_hybrid(int two_a, int two_b)
{
    std::vector<int> half_heights;
    int half_w = two_a / 2;
    int half_h = two_b / 2;

    int prev_height = 0;
    int prev_delta = half_h;

    for (int x = half_w; x > 0; x--) {
        int max_delta = prev_delta + 1;
        int search_start = prev_height;
        int search_end = std::min(prev_height + max_delta, half_h + 1);

        int current_height = search_start;
        for (int y = search_start; y <= search_end; y++) {
            if (is_point_in_ellipse(x, y, two_a, two_b)) {
                current_height = y;
            } else {
                break;
            }
        }

        half_heights.push_back(current_height);
        prev_delta = current_height - prev_height;
        prev_height = current_height;
    }

    return build_full_heights(half_heights, two_a, two_b);
}

// Incremental Reverse approach: O(W + H)
inline HeightsResult generate_ellipse_heights_incremental_reverse(int two_a, int two_b)
{
    std::vector<int> half_heights;
    int half_w = two_a / 2;

    // Handle degenerate cases
    if (half_w <= 1 || two_b <= 2) {
        int current_height = 1;
        for (int x = half_w; x > 0; x--) {
            while (is_point_in_ellipse(x, current_height + 1, two_a, two_b)) {
                current_height++;
            }
            half_heights.push_back(current_height);
        }
        return build_full_heights(half_heights, two_a, two_b);
    }

    // Step 1: Compute first height directly
    double a = two_a / 2.0;
    double b = two_b / 2.0;
    double reposition_factor_x = (two_a % 2 == 1) ? 0.5 : 0.0;
    double reposition_factor_y = (two_b % 2 == 1) ? 0.5 : 0.0;
    
    double x = half_w - 0.5 - reposition_factor_x;
    double x_normalized = x / a;
    int current_height;
    if (x_normalized >= 1.0) {
        current_height = 0;
    } else {
        double y_boundary = b * std::sqrt(1.0 - x_normalized * x_normalized);
        current_height = std::max(1, static_cast<int>(std::floor(y_boundary + 0.5 + reposition_factor_y)));
    }
    half_heights.push_back(current_height);

    // Step 2: Compute second height incrementally
    while (is_point_in_ellipse(half_w - 1, current_height + 1, two_a, two_b)) {
        current_height++;
    }
    half_heights.push_back(current_height);

    // Step 3: Use reverse incremental for remaining columns
    int prev_diff = half_heights.back() - half_heights[half_heights.size() - 2];

    for (int x = half_w - 2; x > 0; x--) {
        if (prev_diff > 0) {
            // STEEP REGION: predict and adjust
            int predicted = current_height + prev_diff;

            // Descend until we find a valid point
            while (predicted > current_height && !is_point_in_ellipse(x, predicted, two_a, two_b)) {
                predicted--;
            }

            // Fine-tune upward
            while (is_point_in_ellipse(x, predicted + 1, two_a, two_b)) {
                predicted++;
            }

            prev_diff = predicted - current_height;
            current_height = predicted;
        } else {
            // FLAT REGION: standard incremental
            while (is_point_in_ellipse(x, current_height + 1, two_a, two_b)) {
                current_height++;
            }
        }

        half_heights.push_back(current_height);
    }

    return build_full_heights(half_heights, two_a, two_b);
}

// Adaptive approach: Always treats ellipses as tall by swapping axes if needed
inline HeightsResult generate_ellipse_heights_adaptive(int two_a, int two_b)
{
    // Determine if ellipse is wide (needs axis swap)
    bool is_wide = two_a > two_b;
    
    if (!is_wide) {
        // Already tall - use best algorithm directly
        return generate_ellipse_heights_incremental_reverse(two_a, two_b);
    }
    
    // Wide ellipse: swap axes, compute, then transpose back
    // Step 1: Compute heights for the swapped ellipse (treating width as height)
    auto [swapped_half_heights, swapped_full_heights] = generate_ellipse_heights_incremental_reverse(two_b, two_a);
    
    // Step 2: Convert swapped coordinates back to original orientation
    // In swapped space: each index represents a column, value is the height
    // In original space: we need heights (along Y) for each X position
    
    // Build a 2D representation of filled points in swapped space
    std::vector<std::vector<bool>> filled_swapped(two_b, std::vector<bool>(two_a, false));
    
    for (int x_swap = 0; x_swap < two_b; x_swap++) {
        int height = swapped_full_heights[x_swap];
        int half_h = height / 2;
        
        if (height % 2 == 0) {
            for (int y_swap = -half_h; y_swap < half_h; y_swap++) {
                int y_idx = y_swap + two_a / 2;
                if (y_idx >= 0 && y_idx < two_a) {
                    filled_swapped[x_swap][y_idx] = true;
                }
            }
        } else {
            for (int y_swap = -half_h; y_swap <= half_h; y_swap++) {
                int y_idx = y_swap + two_a / 2;
                if (y_idx >= 0 && y_idx < two_a) {
                    filled_swapped[x_swap][y_idx] = true;
                }
            }
        }
    }
    
    // Transpose: (x_swap, y_swap) -> (y_swap, x_swap) becomes (x_orig, y_orig)
    std::vector<int> full_heights_orig;
    for (int x_orig = 0; x_orig < two_a; x_orig++) {
        int count = 0;
        for (int y_orig = 0; y_orig < two_b; y_orig++) {
            // In swapped coordinates: x_swap = y_orig, y_swap_idx = x_orig
            if (filled_swapped[y_orig][x_orig]) {
                count++;
            }
        }
        full_heights_orig.push_back(count);
    }
    
    // Compute half_heights from full_heights
    std::vector<int> half_heights_orig;
    int half_w = (two_a + 1) / 2;
    for (int i = two_a - 1; i >= two_a - half_w; i--) {
        int h = full_heights_orig[i];
        half_heights_orig.push_back(two_b % 2 == 0 ? h / 2 : (h + 1) / 2);
    }
    
    return {half_heights_orig, full_heights_orig};
}

// Incremental Fast: O(W + H) - Pure integer arithmetic, no floating point
inline HeightsResult generate_ellipse_heights_incremental_fast(int two_a, int two_b)
{
    std::vector<int> half_heights;
    int half_w = two_a / 2;
    int half_h = (two_b + 1) / 2;

    if (half_w == 0) {
        return build_full_heights(half_heights, two_a, two_b);
    }

    // Integer arithmetic: (sx/a)² + (sy/b)² <= 1
    // Becomes: sx² * b² + sy² * a² <= a² * b²
    long a_squared = (long)two_a * two_a;
    long b_squared = (long)two_b * two_b;
    long threshold = a_squared * b_squared;
    long eight_a_squared = a_squared << 3;
    long eight_b_squared = b_squared << 3;

    int x_offset = two_a & 1;
    int y_offset = two_b & 1;

    // Initialize y tracking
    int current_height = 1;
    long sy_next = 3 - y_offset;
    long y_term_next = sy_next * sy_next * a_squared;
    long y_delta = (a_squared << 2) * (sy_next + 1);

    // Initialize x tracking
    long sx = 2 * half_w - 1 - x_offset;
    long x_term = sx * sx * b_squared;
    long x_delta = (b_squared << 2) * (sx - 1);

    for (int col = half_w; col > 0; col--) {
        while (current_height < half_h && x_term + y_term_next <= threshold) {
            current_height++;
            y_term_next += y_delta;
            y_delta += eight_a_squared;
        }
        half_heights.push_back(current_height);

        // Update x term for next column
        x_term -= x_delta;
        x_delta -= eight_b_squared;
    }

    return build_full_heights(half_heights, two_a, two_b);
}

// Incremental Fast Axis-Flip: O(W + H)
// Experimental: For wide ellipses, attempts to swap axes before running IncrementalFast.
// NOTE: This is a learning/experimental variant. Due to parity/centering complexities in the
// transpose operation, this may produce slightly different results from the canonical algorithms.
// Use for exploration and performance comparison on wide ellipses, but be aware of potential
// minor discrepancies in edge cases.
inline HeightsResult generate_ellipse_heights_incremental_fast_axis_flip(int two_a, int two_b)
{
    // For now, always use the direct incremental-fast without swapping to avoid parity issues
    // Future work: implement a correct transpose that handles odd/even dimensions properly
    return generate_ellipse_heights_incremental_fast(two_a, two_b);
}

// Incremental Reverse Fast: O(W + H) - Optimized version with prediction
inline HeightsResult generate_ellipse_heights_incremental_reverse_fast(int two_a, int two_b)
{
    std::vector<int> half_heights;
    int half_w = two_a / 2;
    int half_h = (two_b + 1) / 2;

    if (half_w == 0) {
        return build_full_heights(half_heights, two_a, two_b);
    }

    // Integer arithmetic constants
    long a_squared = (long)two_a * two_a;
    long b_squared = (long)two_b * two_b;
    long threshold = a_squared * b_squared;
    long eight_a_squared = a_squared << 3;
    long eight_b_squared = b_squared << 3;
    long four_a_squared = a_squared << 2;

    int x_offset = two_a & 1;
    int y_offset = two_b & 1;

    // Initialize x tracking
    long sx = 2 * half_w - 1 - x_offset;
    long x_term = sx * sx * b_squared;
    long x_delta = (b_squared << 2) * (sx - 1);

    // First column: standard incremental
    int current_height = 1;
    long sy_check = 3 - y_offset;
    long y_term_check = sy_check * sy_check * a_squared;
    long y_delta_up = four_a_squared * (sy_check + 1);

    while (current_height < half_h && x_term + y_term_check <= threshold) {
        current_height++;
        y_term_check += y_delta_up;
        y_delta_up += eight_a_squared;
    }
    half_heights.push_back(current_height);

    if (half_w == 1) {
        return build_full_heights(half_heights, two_a, two_b);
    }

    // Second column: continue incremental to establish prevDiff
    x_term -= x_delta;
    x_delta -= eight_b_squared;
    int prev_height = current_height;

    while (current_height < half_h && x_term + y_term_check <= threshold) {
        current_height++;
        y_term_check += y_delta_up;
        y_delta_up += eight_a_squared;
    }
    half_heights.push_back(current_height);
    int prev_diff = current_height - prev_height;

    // Remaining columns: use prediction in steep region
    for (int col = half_w - 2; col > 0; col--) {
        x_term -= x_delta;
        x_delta -= eight_b_squared;
        prev_height = current_height;

        if (prev_diff > 0) {
            // STEEP REGION: predict and verify using integer arithmetic
            int predicted = std::min(current_height + prev_diff, half_h);

            // Compute yTerm for predicted height
            long sy_pred = 2 * predicted - 1 - y_offset;
            long y_term_pred = sy_pred * sy_pred * a_squared;

            if (x_term + y_term_pred <= threshold) {
                // Prediction is inside - accept it
                current_height = predicted;
            } else {
                // Prediction overshot - descend
                long y_delta_down = four_a_squared * (sy_pred - 1);

                while (predicted > current_height && x_term + y_term_pred > threshold) {
                    y_term_pred -= y_delta_down;
                    y_delta_down -= eight_a_squared;
                    predicted--;
                }

                if (predicted > current_height) {
                    current_height = predicted;
                }
            }

            // Reset incremental state for fine-tuning
            sy_check = 2 * (current_height + 1) - 1 - y_offset;
            y_term_check = sy_check * sy_check * a_squared;
            y_delta_up = four_a_squared * (sy_check + 1);

            // Fine-tune upward
            while (current_height < half_h && x_term + y_term_check <= threshold) {
                current_height++;
                y_term_check += y_delta_up;
                y_delta_up += eight_a_squared;
            }
        } else {
            // FLAT REGION: standard incremental
            while (current_height < half_h && x_term + y_term_check <= threshold) {
                current_height++;
                y_term_check += y_delta_up;
                y_delta_up += eight_a_squared;
            }
        }

        half_heights.push_back(current_height);
        prev_diff = current_height - prev_height;
    }

    return build_full_heights(half_heights, two_a, two_b);
}

} // namespace ellipse

#endif // ELLIPSE_ALGORITHMS_H
