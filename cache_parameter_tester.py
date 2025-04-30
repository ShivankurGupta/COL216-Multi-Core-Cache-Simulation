import subprocess
import os
import re
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from pathlib import Path
import argparse

# Parse command line arguments
def parse_args():
    parser = argparse.ArgumentParser(description='Run cache parameter study')
    parser.add_argument('-t', '--trace', default='traces/app4', help='Trace file prefix')
    parser.add_argument('-s', '--set-bits', type=int, default=3, help='Default set index bits')
    parser.add_argument('-E', '--associativity', type=int, default=4, help='Default associativity')
    parser.add_argument('-b', '--block-bits', type=int, default=5, help='Default block bits')
    parser.add_argument('-e', '--executable', default='./cache_sim', help='Simulator executable')
    parser.add_argument('--s-values', type=int, nargs='+', help='Set index bits values to test')
    parser.add_argument('--E-values', type=int, nargs='+', help='Associativity values to test')
    parser.add_argument('--b-values', type=int, nargs='+', help='Block bits values to test')
    return parser.parse_args()

# Configuration with default values
args = parse_args()

EXECUTABLE = args.executable
TRACE_PREFIX = args.trace
DEFAULT_S = args.set_bits
DEFAULT_E = args.associativity
DEFAULT_B = args.block_bits

# Parameters to test (in powers of 2)
S_VALUES = args.s_values if args.s_values else [2, 3, 4, 5]  # 4, 8, 16, 32 sets
E_VALUES = args.E_values if args.E_values else [1, 2, 4, 8]  # direct-mapped, 2-way, 4-way, 8-way
B_VALUES = args.b_values if args.b_values else [4, 5, 6, 7]  # 16, 32, 64, 128 bytes

# Output directories
RESULTS_DIR = "parameter_test_results"
PLOTS_DIR = os.path.join(RESULTS_DIR, "plots")

# Create output directories
os.makedirs(RESULTS_DIR, exist_ok=True)
os.makedirs(PLOTS_DIR, exist_ok=True)

def run_simulation(s, E, b, test_name):
    """Run the cache simulator with given parameters"""
    output_file = os.path.join(RESULTS_DIR, f"{test_name}.txt")
    
    # Build command
    cmd = [EXECUTABLE, "-t", TRACE_PREFIX, "-s", str(s), "-E", str(E), "-b", str(b), "-o", output_file]
    print(f"Running: {' '.join(cmd)}")
    
    # Execute simulator
    process = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    
    if process.returncode != 0:
        print(f"Error running simulation: {process.stderr.decode()}")
        return None
    
    return output_file

def parse_results(output_file):
    """Parse the simulator output file to extract execution times"""
    if not os.path.exists(output_file):
        print(f"Output file {output_file} not found")
        return None
    
    results = {}
    core_cycle_pattern = re.compile(r'Core (\d+) Statistics:\s+.*?Total Execution Cycles: (\d+)', re.DOTALL)
    
    with open(output_file, 'r') as f:
        content = f.read()
    
    # Extract execution cycles for each core
    matches = core_cycle_pattern.findall(content)
    for core_id, cycles in matches:
        results[int(core_id)] = int(cycles)
    
    # Get maximum execution time across cores
    if results:
        results['max'] = max(results.values())
    
    return results

def plot_parameter_impact(parameter_name, x_values, y_values, x_label, y_label):
    """Create a plot showing the impact of a cache parameter on execution time"""
    plt.figure(figsize=(10, 6))
    plt.plot(x_values, y_values, 'o-', linewidth=2, markersize=8)
    plt.grid(True)
    plt.title(f"Impact of {parameter_name} on Maximum Execution Time")
    plt.xlabel(x_label)
    plt.ylabel(y_label)
    
    # Save the plot
    plot_file = os.path.join(PLOTS_DIR, f"{parameter_name.lower().replace(' ', '_')}_impact.png")
    plt.savefig(plot_file)
    plt.close()
    return plot_file

def plot_consolidated_cache_size_impact(all_results):
    """Create a consolidated plot showing impact of cache size across all parameter variations"""
    plt.figure(figsize=(12, 8))
    
    # Prepare data for plotting
    s_varying_data = []
    e_varying_data = []
    b_varying_data = []
    
    for test_name, results in all_results.items():
        match = re.match(r's(\d+)_E(\d+)_b(\d+)', test_name)
        if match:
            s, E, b = map(int, match.groups())
            cache_size_kb = (2**s * E * 2**b) / 1024
            max_time = results['max']
            
            # Determine which parameter is being varied
            if s != DEFAULT_S and E == DEFAULT_E and b == DEFAULT_B:
                s_varying_data.append((cache_size_kb, max_time, s))
            elif s == DEFAULT_S and E != DEFAULT_E and b == DEFAULT_B:
                e_varying_data.append((cache_size_kb, max_time, E))
            elif s == DEFAULT_S and E == DEFAULT_E and b != DEFAULT_B:
                b_varying_data.append((cache_size_kb, max_time, b))
    
    # Sort data by cache size
    s_varying_data.sort(key=lambda x: x[0])
    e_varying_data.sort(key=lambda x: x[0])
    b_varying_data.sort(key=lambda x: x[0])
    
    # Plot with different markers for each parameter type
    if s_varying_data:
        x_vals, y_vals, labels = zip(*s_varying_data)
        plt.plot(x_vals, y_vals, 'o-', linewidth=2, markersize=8, label=f"Varying Sets (s)")
        for i, label in enumerate(labels):
            plt.annotate(f"s={label}", (x_vals[i], y_vals[i]), textcoords="offset points", xytext=(0,10), ha='center')
    
    if e_varying_data:
        x_vals, y_vals, labels = zip(*e_varying_data)
        plt.plot(x_vals, y_vals, 's-', linewidth=2, markersize=8, label=f"Varying Associativity (E)")
        for i, label in enumerate(labels):
            plt.annotate(f"E={label}", (x_vals[i], y_vals[i]), textcoords="offset points", xytext=(0,10), ha='center')
    
    if b_varying_data:
        x_vals, y_vals, labels = zip(*b_varying_data)
        plt.plot(x_vals, y_vals, '^-', linewidth=2, markersize=8, label=f"Varying Block Size (b)")
        for i, label in enumerate(labels):
            plt.annotate(f"b={label}", (x_vals[i], y_vals[i]), textcoords="offset points", xytext=(0,10), ha='center')
    
    plt.grid(True)
    plt.title("Impact of Cache Size on Maximum Execution Time (All Parameters)")
    plt.xlabel("Cache Size (KB)")
    plt.ylabel("Max Execution Time (cycles)")
    plt.legend()
    
    # Save the plot
    plot_file = os.path.join(PLOTS_DIR, "consolidated_cache_size_impact.png")
    plt.savefig(plot_file)
    plt.close()
    return plot_file

def main():
    print(f"Running parameter study with defaults: s={DEFAULT_S}, E={DEFAULT_E}, b={DEFAULT_B}")
    print(f"Trace prefix: {TRACE_PREFIX}")
    print(f"Testing s values: {S_VALUES}")
    print(f"Testing E values: {E_VALUES}")
    print(f"Testing b values: {B_VALUES}")
    
    all_results = {}
    
    # 1. Vary set index bits (s) - impacts number of sets
    s_results = []
    for s in S_VALUES:
        test_name = f"s{s}_E{DEFAULT_E}_b{DEFAULT_B}"
        output_file = run_simulation(s, DEFAULT_E, DEFAULT_B, test_name)
        if output_file:
            results = parse_results(output_file)
            if results:
                s_results.append((s, results['max']))
                all_results[test_name] = results
    
    # 2. Vary associativity (E)
    e_results = []
    for e in E_VALUES:
        test_name = f"s{DEFAULT_S}_E{e}_b{DEFAULT_B}"
        output_file = run_simulation(DEFAULT_S, e, DEFAULT_B, test_name)
        if output_file:
            results = parse_results(output_file)
            if results:
                e_results.append((e, results['max']))
                all_results[test_name] = results
    
    # 3. Vary block size (b) - (2^b bytes)
    b_results = []
    for b in B_VALUES:
        test_name = f"s{DEFAULT_S}_E{DEFAULT_E}_b{b}"
        output_file = run_simulation(DEFAULT_S, DEFAULT_E, b, test_name)
        if output_file:
            results = parse_results(output_file)
            if results:
                b_results.append((b, results['max']))
                all_results[test_name] = results
    
    # Create plots
    if s_results:
        s_values, max_times = zip(*s_results)
        sets_values = [2**s for s in s_values]
        cache_sizes = [(2**s) * DEFAULT_E * (2**DEFAULT_B) / 1024 for s in s_values]
        plot_parameter_impact("Cache Sets", s_values, max_times, "Set Index Bits (s)", "Max Execution Time (cycles)")
        plot_parameter_impact("Cache Size", cache_sizes, max_times, "Cache Size (KB)", "Max Execution Time (cycles)")
    
    if e_results:
        e_values, max_times = zip(*e_results)
        plot_parameter_impact("Associativity", e_values, max_times, "Associativity (E)", "Max Execution Time (cycles)")
    
    if b_results:
        b_values, max_times = zip(*b_results)
        block_sizes = [2**b for b in b_values]
        plot_parameter_impact("Block Size", block_sizes, max_times, "Block Size (bytes)", "Max Execution Time (cycles)")
    
    # Add consolidated cache size plot
    if all_results:
        plot_consolidated_cache_size_impact(all_results)
    
    # Create a summary table
    summary_data = []
    for test_name, results in all_results.items():
        match = re.match(r's(\d+)_E(\d+)_b(\d+)', test_name)
        if match:
            s, E, b = map(int, match.groups())
            cache_size_kb = (2**s * E * 2**b) / 1024
            block_size_bytes = 2**b
            sets = 2**s
            
            row = {
                'Test': test_name,
                'Sets (2^s)': sets,
                'Associativity (E)': E, 
                'Block Size (bytes)': block_size_bytes,
                'Cache Size (KB)': cache_size_kb,
                'Max Execution Time': results['max']
            }
            
            # Add individual core results
            for core_id in range(4):
                if core_id in results:
                    row[f'Core {core_id} Execution Time'] = results[core_id]
            
            summary_data.append(row)
    
    # Create summary DataFrame and save to CSV
    if summary_data:
        df = pd.DataFrame(summary_data)
        csv_file = os.path.join(RESULTS_DIR, "cache_parameter_study.csv")
        df.to_csv(csv_file, index=False)
        print(f"Summary results saved to {csv_file}")
        
        # Generate a report with observations
        generate_report(df, all_results)
    else:
        print("No data collected. Check if simulations ran correctly.")

def generate_report(df, all_results):
    """Generate a report with observations about the parameter study"""
    report_file = os.path.join(RESULTS_DIR, "cache_parameter_study_report.md")
    
    with open(report_file, 'w') as f:
        f.write("# Cache Parameter Study Report\n\n")
        
        f.write("## Overview\n")
        f.write("This report analyzes the impact of different cache parameters on execution time.\n\n")
        
        f.write("## Parameters Tested\n")
        f.write(f"- Set Index Bits (s): {S_VALUES} (corresponding to {[2**s for s in S_VALUES]} sets)\n")
        f.write(f"- Associativity (E): {E_VALUES}\n")
        f.write(f"- Block Size Bits (b): {B_VALUES} (corresponding to {[2**b for b in B_VALUES]} bytes)\n\n")
        
        f.write("## Summary of Results\n")
        f.write("```\n")
        f.write(df.to_string())
        f.write("\n```\n\n")
        
        f.write("## Observations\n\n")
        
        f.write("### Impact of Cache Size (varying s)\n")
        f.write("Increasing the number of sets typically leads to better performance up to a point, ")
        f.write("as it reduces conflict misses. However, beyond that point, the benefit may diminish ")
        f.write("due to other factors like associativity and block size limitations.\n\n")
        
        f.write("### Impact of Associativity (varying E)\n")
        f.write("Higher associativity generally reduces conflict misses, improving performance. ")
        f.write("However, very high associativity may lead to diminishing returns and increased ")
        f.write("hardware complexity. The sweet spot often depends on the specific workload.\n\n")
        
        f.write("### Impact of Block Size (varying b)\n")
        f.write("Larger block sizes can improve spatial locality and reduce compulsory misses. ")
        f.write("However, too large blocks can lead to cache pollution and increased miss penalties. ")
        f.write("The optimal block size depends on the spatial locality in the workload.\n\n")
        
        f.write("## Plots\n")
        f.write("See the plots directory for visual representations of these relationships.\n\n")
        
        f.write("## Conclusion\n")
        f.write("The optimal cache configuration depends on the specific workload characteristics. ")
        f.write("This study helps identify how different parameters affect execution time for the given application traces.")
    
    print(f"Report generated at {report_file}")

if __name__ == "__main__":
    main()