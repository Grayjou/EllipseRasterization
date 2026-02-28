#!/usr/bin/env python3
"""
Visualize ellipse rasterization benchmark results.
Generates heatmaps comparing algorithm performance.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from pathlib import Path

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

if __name__ == '__main__':
    # Load results
    csv_path = Path('results/benchmark_results.csv')
    
    if not csv_path.exists():
        print(f"Error: {csv_path} not found!")
        print("Please run the C++ benchmark first to generate results.")
        exit(1)
    
    df = load_results(csv_path)
    
    # Generate visualizations
    create_heatmaps(df)
    create_comparison_plot(df)
    create_relative_heatmaps(df)
    create_speedup_analysis(df)
    
    # Print statistics
    print_statistics(df)
    
    print("\n✓ All visualizations generated successfully!")
