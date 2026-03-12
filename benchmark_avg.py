import csv
from collections import defaultdict
import statistics

# Read the benchmark results
data = defaultdict(list)
with open('results/benchmark_results.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        algo = row['Algorithm']
        time = float(row['Time_us'])
        data[algo].append(time)

# Calculate and display averages, std dev, and percentiles
print("Algorithm\t\tMean (us) ± Std Dev\t1st %ile\t5th %ile")
print("-" * 70)
for algo in sorted(data.keys()):
    times = data[algo]
    mean = statistics.mean(times)
    stdev = statistics.stdev(times)
    percentiles = statistics.quantiles(times, n=100)
    p1 = percentiles[0]  # 1st percentile
    p5 = percentiles[4]  # 5th percentile
    print(f"{algo}\t\t{mean:.4f} ± {stdev:.4f}\t{p1:.4f}\t{p5:.4f}")