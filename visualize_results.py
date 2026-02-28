#!/usr/bin/env python3
"""
Visualize ellipse rasterization benchmark results and outline demonstrations.
Generates heatmaps comparing algorithm performance and outline renderings.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).parent / "ports"))
from ellipse_fast import (
    generate_ellipse_heights_incremental_fast,
    heights_to_filled,
    heights_to_thin_outline,
    heights_to_full_outline,
)

def load_results(csv_path='results/benchmark_results.csv'):
    """Load benchmark results from CSV."""
    df = pd.read_csv(csv_path)
    return df

def create_heatmaps(df, output_path='results/benchmark_heatmaps.png'):
    """Create comparison heatmaps for all algorithms."""
    
    algorithms = df['Algorithm'].unique()
    sizes = sorted(df['Width'].unique())
    
    # Create figure with subplots
    n_algos = len(algorithms)
    fig, axes = plt.subplots(2, 3, figsize=(18, 12))
    axes = axes.flatten()
    
    for idx, algo in enumerate(algorithms):
        if idx >= len(axes):
            break
            
        # Filter data for this algorithm
        algo_data = df[df['Algorithm'] == algo]
        
        # Create pivot table (heatmap data)
        heatmap_data = algo_data.pivot(index='Height', columns='Width', values='Time_us')
        
        # Plot heatmap
        ax = axes[idx]
        sns.heatmap(heatmap_data, annot=False, fmt='.2f', cmap='viridis', 
                    ax=ax, cbar_kws={'label': 'Time (μs)'})
        ax.set_title(f'{algo} Algorithm', fontsize=14, fontweight='bold')
        ax.set_xlabel('Width (2a)', fontsize=12)
        ax.set_ylabel('Height (2b)', fontsize=12)
        
        # Customize ticks
        tick_indices = [0, len(sizes)//4, len(sizes)//2, 3*len(sizes)//4, len(sizes)-1]
        tick_labels = [sizes[i] for i in tick_indices]
        ax.set_xticks([i + 0.5 for i in tick_indices])
        ax.set_xticklabels(tick_labels, rotation=0)
        ax.set_yticks([i + 0.5 for i in tick_indices])
        ax.set_yticklabels(tick_labels, rotation=0)
    
    # Hide unused subplots
    for idx in range(len(algorithms), len(axes)):
        axes[idx].axis('off')
    
    plt.suptitle('Ellipse Rasterization Algorithm Performance Comparison', 
                 fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Heatmaps saved to {output_path}")
    plt.close()

def create_comparison_plot(df, output_path='results/algorithm_comparison.png'):
    """Create line plot comparing algorithms at different sizes."""
    
    algorithms = df['Algorithm'].unique()
    
    # Use square ellipses (Width == Height) for comparison
    square_data = df[df['Width'] == df['Height']]
    
    fig, ax = plt.subplots(figsize=(12, 7))
    
    for algo in algorithms:
        algo_data = square_data[square_data['Algorithm'] == algo]
        algo_data = algo_data.sort_values('Width')
        ax.plot(algo_data['Width'], algo_data['Time_us'], 
               marker='o', label=algo, linewidth=2, markersize=6)
    
    ax.set_xlabel('Ellipse Size (Width = Height)', fontsize=12)
    ax.set_ylabel('Time (μs)', fontsize=12)
    ax.set_title('Algorithm Performance on Square Ellipses', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10, loc='upper left')
    ax.grid(True, alpha=0.3)
    ax.set_yscale('log')
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Comparison plot saved to {output_path}")
    plt.close()

def create_relative_heatmaps(df, baseline='Direct', output_path='results/relative_performance_heatmaps.png'):
    """Create heatmaps showing performance relative to Direct algorithm."""
    
    algorithms = [a for a in df['Algorithm'].unique() if a != baseline]
    sizes = sorted(df['Width'].unique())
    
    # Get baseline data
    baseline_df = df[df['Algorithm'] == baseline]
    baseline_pivot = baseline_df.pivot(index='Height', columns='Width', values='Time_us')
    
    # Create figure with subplots
    fig, axes = plt.subplots(2, 3, figsize=(18, 12))
    axes = axes.flatten()
    
    # Plot relative performance for each algorithm
    for idx, algo in enumerate(algorithms):
        if idx >= len(axes):
            break
        
        # Filter data for this algorithm
        algo_data = df[df['Algorithm'] == algo]
        algo_pivot = algo_data.pivot(index='Height', columns='Width', values='Time_us')
        
        # Calculate relative performance (Direct / Algorithm)
        # Values > 1 mean algorithm is faster than Direct
        relative = baseline_pivot / algo_pivot
        
        # Plot heatmap
        ax = axes[idx]
        sns.heatmap(relative, annot=False, fmt='.2f', cmap='RdYlGn', center=1.0,
                    ax=ax, cbar_kws={'label': 'Speedup vs Direct'}, 
                    vmin=0.8, vmax=1.2)
        ax.set_title(f'{algo} vs {baseline}', fontsize=14, fontweight='bold')
        ax.set_xlabel('Width (2a)', fontsize=12)
        ax.set_ylabel('Height (2b)', fontsize=12)
        
        # Customize ticks
        tick_indices = [0, len(sizes)//4, len(sizes)//2, 3*len(sizes)//4, len(sizes)-1]
        tick_labels = [sizes[i] for i in tick_indices]
        ax.set_xticks([i + 0.5 for i in tick_indices])
        ax.set_xticklabels(tick_labels, rotation=0)
        ax.set_yticks([i + 0.5 for i in tick_indices])
        ax.set_yticklabels(tick_labels, rotation=0)
    
    # Hide unused subplots
    for idx in range(len(algorithms), len(axes)):
        axes[idx].axis('off')
    
    plt.suptitle(f'Relative Performance vs {baseline} Algorithm\n(Green = Faster, Red = Slower)', 
                 fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Relative performance heatmaps saved to {output_path}")
    plt.close()

def create_speedup_analysis(df, baseline='Direct', output_path='results/speedup_analysis.png'):
    """Create speedup analysis relative to baseline algorithm."""
    
    algorithms = [a for a in df['Algorithm'].unique() if a != baseline]
    square_data = df[df['Width'] == df['Height']]
    
    fig, ax = plt.subplots(figsize=(12, 7))
    
    baseline_data = square_data[square_data['Algorithm'] == baseline].sort_values('Width')
    
    for algo in algorithms:
        algo_data = square_data[square_data['Algorithm'] == algo].sort_values('Width')
        speedup = baseline_data['Time_us'].values / algo_data['Time_us'].values
        ax.plot(algo_data['Width'], speedup, 
               marker='s', label=f'{algo} vs {baseline}', linewidth=2, markersize=6)
    
    ax.axhline(y=1.0, color='red', linestyle='--', alpha=0.5, label='No speedup')
    ax.set_xlabel('Ellipse Size (Width = Height)', fontsize=12)
    ax.set_ylabel(f'Speedup vs {baseline}', fontsize=12)
    ax.set_title(f'Algorithm Speedup Relative to {baseline}', fontsize=14, fontweight='bold')
    ax.legend(fontsize=10)
    ax.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Speedup analysis saved to {output_path}")
    plt.close()

def print_statistics(df):
    """Print summary statistics."""
    print("\n=== Performance Statistics ===\n")
    
    for algo in df['Algorithm'].unique():
        algo_data = df[df['Algorithm'] == algo]
        print(f"{algo}:")
        print(f"  Mean: {algo_data['Time_us'].mean():.2f} μs")
        print(f"  Median: {algo_data['Time_us'].median():.2f} μs")
        print(f"  Min: {algo_data['Time_us'].min():.2f} μs")
        print(f"  Max: {algo_data['Time_us'].max():.2f} μs")
        print()


def create_outline_visualization(output_path='results/outline_comparison.png'):
    """
    Create a figure showing thin vs full outlines for several ellipse sizes.
    Uses the Python port of IncrementalFast to generate heights.
    """
    test_cases = [
        (20, 12, "20x12"),
        (30, 20, "30x20"),
        (15, 25, "15x25"),
        (10, 10, "10x10"),
    ]

    fig, axes = plt.subplots(len(test_cases), 3, figsize=(18, 5 * len(test_cases)))

    for row, (two_a, two_b, label) in enumerate(test_cases):
        _, full_heights = generate_ellipse_heights_incremental_fast(two_a, two_b)
        filled = heights_to_filled(full_heights)
        thin   = heights_to_thin_outline(full_heights)
        full   = heights_to_full_outline(full_heights)

        filled_set = set(filled)
        thin_set   = set(thin)
        full_set   = set(full)

        xs = [p[0] for p in filled]
        ys = [p[1] for p in filled]
        x_lo, x_hi = min(xs) - 1, max(xs) + 1
        y_lo, y_hi = min(ys) - 1, max(ys) + 1
        W = x_hi - x_lo + 1
        H = y_hi - y_lo + 1

        def make_grid(outline_set):
            grid = np.zeros((H, W))
            for (x, y) in filled:
                grid[y_hi - y][x - x_lo] = 1  # interior
            for (x, y) in outline_set:
                grid[y_hi - y][x - x_lo] = 2  # outline
            return grid

        # Column 0: Thin outline
        ax = axes[row][0]
        grid = make_grid(thin_set)
        cmap = plt.cm.colors.ListedColormap(['white', '#d0d0ff', '#2020a0'])
        ax.imshow(grid, cmap=cmap, interpolation='nearest', aspect='equal')
        ax.set_title(f'{label} — Thin outline (8-connected)\n{len(thin)} points', fontsize=11)
        ax.set_xticks([]); ax.set_yticks([])
        ax.grid(True, which='both', color='#cccccc', linewidth=0.3)

        # Column 1: Full outline
        ax = axes[row][1]
        grid = make_grid(full_set)
        cmap2 = plt.cm.colors.ListedColormap(['white', '#d0ffd0', '#20a020'])
        ax.imshow(grid, cmap=cmap2, interpolation='nearest', aspect='equal')
        ax.set_title(f'{label} — Full outline (4-connected)\n{len(full)} points', fontsize=11)
        ax.set_xticks([]); ax.set_yticks([])
        ax.grid(True, which='both', color='#cccccc', linewidth=0.3)

        # Column 2: Combined
        ax = axes[row][2]
        grid = np.zeros((H, W))
        for (x, y) in filled:
            grid[y_hi - y][x - x_lo] = 1
        for (x, y) in thin:
            grid[y_hi - y][x - x_lo] = 2
        for (x, y) in full:
            grid[y_hi - y][x - x_lo] = 3
        cmap3 = plt.cm.colors.ListedColormap(['white', '#e8e8e8', '#ffaa44', '#cc2222'])
        ax.imshow(grid, cmap=cmap3, interpolation='nearest', aspect='equal')
        ax.set_title(f'{label} — Combined\nred=full({len(full)}), orange=thin-only({len(thin)-len(full)}), gray=interior',
                     fontsize=10)
        ax.set_xticks([]); ax.set_yticks([])
        ax.grid(True, which='both', color='#cccccc', linewidth=0.3)

    plt.suptitle('Ellipse Outline Comparison: Thin (8-connected) vs Full (4-connected)',
                 fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_path, dpi=200, bbox_inches='tight')
    print(f"Outline comparison saved to {output_path}")
    plt.close()


def create_outline_only_visualization(output_path='results/outline_only.png'):
    """
    Show just the outline pixels (no fill) for thin and full, side by side.
    """
    test_cases = [
        (30, 20, "30x20"),
        (40, 30, "40x30"),
        (20, 35, "20x35"),
    ]

    fig, axes = plt.subplots(len(test_cases), 2, figsize=(14, 5 * len(test_cases)))

    for row, (two_a, two_b, label) in enumerate(test_cases):
        _, full_heights = generate_ellipse_heights_incremental_fast(two_a, two_b)
        thin = heights_to_thin_outline(full_heights)
        full = heights_to_full_outline(full_heights)

        for col, (pts, name, color) in enumerate([
            (full,"Thin (8-conn)", '#2020a0'),
            (thin, "Full (4-conn)", '#20a020'),
        ]):
            ax = axes[row][col]
            if pts:
                xs = [p[0] for p in pts]
                ys = [p[1] for p in pts]
                ax.scatter(xs, ys, s=30, c=color, marker='s', edgecolors='none')
                ax.set_aspect('equal')
                ax.invert_yaxis()
            ax.set_title(f'{label} — {name} ({len(pts)} pts)', fontsize=12)
            ax.grid(True, alpha=0.3)

    plt.suptitle('Outline-Only View', fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_path, dpi=200, bbox_inches='tight')
    print(f"Outline-only visualization saved to {output_path}")
    plt.close()

if __name__ == '__main__':
    # Always generate outline visualizations (no benchmark data needed)
    create_outline_visualization()
    create_outline_only_visualization()
    
    # Load benchmark results if available
    csv_path = Path('results/benchmark_results.csv')
    
    if csv_path.exists():
        df = load_results(csv_path)
        
        # Generate benchmark visualizations
        create_heatmaps(df)
        create_comparison_plot(df)
        create_relative_heatmaps(df)
        create_speedup_analysis(df)
        
        # Print statistics
        print_statistics(df)
    else:
        print(f"Note: {csv_path} not found — skipping benchmark visualizations.")
        print("Run the C++ benchmark to generate benchmark_results.csv.")
    
    print("\n✓ All visualizations generated successfully!")
