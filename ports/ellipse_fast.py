"""
Fast ellipse rasterization algorithms - Python port
Ports of IncrementalFast and IncrementalFastAxisFlip from C++
Includes outline extraction from heights (thin and full).
"""

from typing import List, Tuple

HeightsResult = Tuple[List[int], List[int]]


def is_point_in_ellipse(norm_x: int, norm_y: int, two_a: int, two_b: int) -> bool:
    """Check if a normalized point is inside the ellipse."""
    a = two_a / 2
    b = two_b / 2
    reposition_factor_x = 0.5 if two_a % 2 == 1 else 0
    reposition_factor_y = 0.5 if two_b % 2 == 1 else 0
    x = norm_x - 0.5 - reposition_factor_x
    y = norm_y - 0.5 - reposition_factor_y
    return (x**2 / a**2) + (y**2 / b**2) <= 1


def build_full_heights(half_heights: List[int], two_a: int, two_b: int) -> HeightsResult:
    """Mirror half-heights to get full column heights."""
    head = half_heights if two_a % 2 == 0 else half_heights[:-1]
    tail = half_heights[::-1]
    full_half_heights = head + tail

    full_heights = (
        [h * 2 for h in full_half_heights] if two_b % 2 == 0
        else [h * 2 - 1 for h in full_half_heights]
    )
    return full_half_heights, full_heights


def generate_ellipse_heights_incremental_fast(two_a: int, two_b: int) -> HeightsResult:
    """
    O(W + H) - Pure integer arithmetic, no floating point.
    
    Uses incremental computation with integer-only operations to avoid
    costly sqrt() calls and floating-point rounding issues.
    
    Args:
        two_a: Full width of ellipse (2 * semi-major axis a)
        two_b: Full height of ellipse (2 * semi-minor axis b)
    
    Returns:
        Tuple of (half_heights, full_heights) where:
        - half_heights: Heights for half the ellipse width
        - full_heights: Complete column heights for full width
    """
    half_heights = []
    half_w = two_a // 2
    half_h = (two_b + 1) // 2

    if half_w == 0:
        return build_full_heights(half_heights, two_a, two_b)

    # Integer arithmetic: (sx/a)² + (sy/b)² <= 1
    # Becomes: sx² * b² + sy² * a² <= a² * b²
    a_squared = two_a * two_a
    b_squared = two_b * two_b
    threshold = a_squared * b_squared
    eight_a_squared = a_squared << 3
    eight_b_squared = b_squared << 3

    x_offset = two_a & 1
    y_offset = two_b & 1

    # Initialize y tracking
    current_height = 1
    sy_next = 3 - y_offset
    y_term_next = sy_next * sy_next * a_squared
    y_delta = (a_squared << 2) * (sy_next + 1)

    # Initialize x tracking
    sx = 2 * half_w - 1 - x_offset
    x_term = sx * sx * b_squared
    x_delta = (b_squared << 2) * (sx - 1)

    for col in range(half_w, 0, -1):
        while current_height < half_h and x_term + y_term_next <= threshold:
            current_height += 1
            y_term_next += y_delta
            y_delta += eight_a_squared

        half_heights.append(current_height)

        # Update x term for next column
        x_term -= x_delta
        x_delta -= eight_b_squared

    return build_full_heights(half_heights, two_a, two_b)


def generate_ellipse_heights_incremental_fast_axis_flip(two_a: int, two_b: int) -> HeightsResult:
    """
    O(W + H) - Axis-flip variant of IncrementalFast.
    
    Experimental: For wide ellipses, this algorithm is intended to swap axes
    before running IncrementalFast for better performance characteristics.
    
    Current implementation: Calls IncrementalFast directly without swapping
    due to parity/centering complexities in the transpose operation.
    
    Args:
        two_a: Full width of ellipse
        two_b: Full height of ellipse
    
    Returns:
        Tuple of (half_heights, full_heights)
    """
    # For now, always use the direct incremental-fast without swapping
    # Future work: implement correct transpose handling for odd/even dimensions
    return generate_ellipse_heights_incremental_fast(two_a, two_b)


# ──────────────────────────────────────────────────────────────────────
#  Outline extraction from heights
# ──────────────────────────────────────────────────────────────────────

def _y_range(H: int) -> Tuple[int, int]:
    """Compute the inclusive y-range [ylo, yhi] for a column of height H."""
    half_h = H // 2
    if H % 2 == 1:
        return (-half_h, half_h)
    else:
        return (-half_h, half_h - 1)


def _compute_x_coords(W: int) -> List[int]:
    """Compute centered x-coordinates for a heights array of size W."""
    half_w = W // 2
    if W % 2 == 1:
        return list(range(-half_w, half_w + 1))
    else:
        return list(range(-half_w, half_w))


def heights_to_filled(heights: List[int]) -> List[Tuple[int, int]]:
    """Convert heights array to filled (x, y) coordinates."""
    W = len(heights)
    if W == 0:
        return []
    xs = _compute_x_coords(W)
    points = []
    for i in range(W):
        H = heights[i]
        if H <= 0:
            continue
        x = xs[i]
        ylo, yhi = _y_range(H)
        for y in range(ylo, yhi + 1):
            points.append((x, y))
    return points


def heights_to_full_outline(heights: List[int]) -> List[Tuple[int, int]]:
    """
    Full outline (4-connected boundary).
    
    A filled pixel is on the full outline if at least one of its 4 cardinal
    neighbors (up, down, left, right) is NOT inside the filled ellipse.
    This produces a thicker boundary because diagonal adjacency is not
    considered.
    
    Computed directly from heights in O(perimeter).
    
    Args:
        heights: Full column heights array.
    
    Returns:
        List of (x, y) outline points.
    """
    W = len(heights)
    if W == 0:
        return []
    xs = _compute_x_coords(W)
    outline = []

    for i in range(W):
        H = heights[i]
        if H <= 0:
            continue
        x = xs[i]
        ylo, yhi = _y_range(H)

        H_left  = heights[i - 1] if i > 0 else 0
        H_right = heights[i + 1] if i < W - 1 else 0

        ylo_L, yhi_L = _y_range(H_left)  if H_left  > 0 else (0, -1)
        ylo_R, yhi_R = _y_range(H_right) if H_right > 0 else (0, -1)

        if H_left == 0 and H_right == 0:
            for y in range(ylo, yhi + 1):
                outline.append((x, y))
            continue

        for y in range(ylo, yhi + 1):
            on_outline = False
            if y == yhi:
                on_outline = True
            if y == ylo:
                on_outline = True
            if H_left == 0 or y < ylo_L or y > yhi_L:
                on_outline = True
            if H_right == 0 or y < ylo_R or y > yhi_R:
                on_outline = True
            if on_outline:
                outline.append((x, y))

    return outline


def heights_to_thin_outline(heights: List[int]) -> List[Tuple[int, int]]:
    """
    Thin outline (8-connected boundary).
    
    A filled pixel is on the thin outline if at least one of its 8 neighbors
    (including diagonals) is NOT inside the filled ellipse. This produces a
    thinner, 1-pixel-wide boundary.
    
    Computed directly from heights in O(perimeter).
    
    Args:
        heights: Full column heights array.
    
    Returns:
        List of (x, y) outline points.
    """
    W = len(heights)
    if W == 0:
        return []
    xs = _compute_x_coords(W)
    outline = []

    for i in range(W):
        H = heights[i]
        if H <= 0:
            continue
        x = xs[i]
        ylo, yhi = _y_range(H)

        H_left  = heights[i - 1] if i > 0 else 0
        H_right = heights[i + 1] if i < W - 1 else 0

        ylo_L, yhi_L = _y_range(H_left)  if H_left  > 0 else (0, -1)
        ylo_R, yhi_R = _y_range(H_right) if H_right > 0 else (0, -1)

        if H_left == 0 and H_right == 0:
            for y in range(ylo, yhi + 1):
                outline.append((x, y))
            continue

        for y in range(ylo, yhi + 1):
            on_outline = False

            # Cardinal neighbors
            if y == yhi:
                on_outline = True
            if y == ylo:
                on_outline = True
            if H_left == 0 or y < ylo_L or y > yhi_L:
                on_outline = True
            if H_right == 0 or y < ylo_R or y > yhi_R:
                on_outline = True

            # Diagonal neighbors (only if cardinal didn't trigger)
            if not on_outline:
                if H_left == 0 or (y + 1) < ylo_L or (y + 1) > yhi_L:
                    on_outline = True
                if not on_outline and (H_left == 0 or (y - 1) < ylo_L or (y - 1) > yhi_L):
                    on_outline = True
                if not on_outline and (H_right == 0 or (y + 1) < ylo_R or (y + 1) > yhi_R):
                    on_outline = True
                if not on_outline and (H_right == 0 or (y - 1) < ylo_R or (y - 1) > yhi_R):
                    on_outline = True

            if on_outline:
                outline.append((x, y))

    return outline


# Example usage and testing
if __name__ == "__main__":
    print("Fast Ellipse Rasterization Algorithms - Python")
    print("=" * 50)
    
    # Test cases
    test_cases = [
        (10, 10, "Square 10x10"),
        (20, 15, "Wide 20x15"),
        (15, 20, "Tall 15x20"),
        (30, 25, "Wide 30x25"),
    ]
    
    for two_a, two_b, description in test_cases:
        print(f"\n{description}:")
        half_heights, full_heights = generate_ellipse_heights_incremental_fast(two_a, two_b)
        
        print(f"  Dimensions: {two_a}x{two_b}")
        print(f"  Half-heights count: {len(half_heights)}")
        print(f"  Full-heights count: {len(full_heights)}")
        print(f"  Full heights: {full_heights[:10]}{'...' if len(full_heights) > 10 else ''}")
        
        # Count total points
        total_points = sum(full_heights)
        print(f"  Total filled points: {total_points}")
        
        # Outline extraction
        thin = heights_to_thin_outline(full_heights)
        full = heights_to_full_outline(full_heights)
        print(f"  Thin outline (8-connected): {len(thin)} points")
        print(f"  Full outline (4-connected): {len(full)} points")
    
    print("\n" + "=" * 50)
    print("Outline visualization for 20x12 ellipse:\n")
    
    _, fh = generate_ellipse_heights_incremental_fast(20, 12)
    filled = set(heights_to_filled(fh))
    thin   = set(heights_to_thin_outline(fh))
    full   = set(heights_to_full_outline(fh))
    
    if filled:
        xs = [p[0] for p in filled]
        ys = [p[1] for p in filled]
        
        print("Thin outline (## = outline, .. = interior):")
        for y in range(max(ys), min(ys) - 1, -1):
            row = ""
            for x in range(min(xs), max(xs) + 1):
                if (x, y) in thin:
                    row += "##"
                elif (x, y) in filled:
                    row += ".."
                else:
                    row += "  "
            print(row)
        
        print("\nFull outline (## = outline, .. = interior):")
        for y in range(max(ys), min(ys) - 1, -1):
            row = ""
            for x in range(min(xs), max(xs) + 1):
                if (x, y) in full:
                    row += "##"
                elif (x, y) in filled:
                    row += ".."
                else:
                    row += "  "
            print(row)
        
        print("\nCombined (## = full outline, ++ = thin-only, .. = interior):")
        for y in range(max(ys), min(ys) - 1, -1):
            row = ""
            for x in range(min(xs), max(xs) + 1):
                if (x, y) in full:
                    row += "##"
                elif (x, y) in thin:
                    row += "++"
                elif (x, y) in filled:
                    row += ".."
                else:
                    row += "  "
            print(row)
    
    print("\n" + "=" * 50)
    print("Testing axis-flip variant...")
    
    two_a, two_b = 20, 15
    result_fast = generate_ellipse_heights_incremental_fast(two_a, two_b)
    result_flip = generate_ellipse_heights_incremental_fast_axis_flip(two_a, two_b)
    
    if result_fast == result_flip:
        print(f"  Both algorithms produce identical results for {two_a}x{two_b}")
    else:
        print(f"  Results differ for {two_a}x{two_b}")
