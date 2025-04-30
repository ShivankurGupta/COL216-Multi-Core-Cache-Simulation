# L1 Cache Simulator for Quad-Core Processors

## Overview
This project implements an L1 cache simulator for quad-core processors with MESI cache coherence protocol. The simulator models a system where each core has its private L1 data cache, and the caches are kept coherent using a central snooping bus. The simulator is trace-driven and provides detailed statistics on cache performance and coherence behavior.

---

## Directory Structure

```
COL216_A3/
├── src/                     # Source code for the simulator
│   ├── main.cpp             # Entry point of the simulator
│   ├── Core.hpp             # Core class definition
│   ├── Core.cpp             # Core class implementation
│   ├── Cache.hpp            # Cache class definition
│   ├── Cache.cpp            # Cache class implementation
│   ├── CacheSet.hpp         # CacheSet class definition
│   ├── CacheSet.cpp         # CacheSet class implementation
│   ├── CacheLine.hpp        # CacheLine structure definition
│   ├── Bus.hpp              # Bus class definition
│   ├── Bus.cpp              # Bus class implementation
├── traces/                  # Input trace files for simulation
│   ├── app1_proc0.trace     # Trace file for Core 0 (app1)
│   ├── app1_proc1.trace     # Trace file for Core 1 (app1)
│   ├── app1_proc2.trace     # Trace file for Core 2 (app1)
│   ├── app1_proc3.trace     # Trace file for Core 3 (app1)
│   └── ...                  # Additional trace files for other applications
├── parameter_test_results/  # Results of parameter variation experiments
│   ├── s2_E1_b4.txt         # Output for specific cache parameters
│   ├── cache_parameter_study.csv  # Summary of parameter study
│   ├── cache_parameter_study_report.md  # Markdown report of parameter study
├── report.latex             # LaTeX report for the assignment
├── Makefile                 # Makefile to compile the simulator
├── README.md                # This README file
```

### Key Components
1. **`src/`**: Contains the source code for the simulator, including the main logic for cores, caches, and the bus.
2. **`traces/`**: Contains input trace files for different applications. Each application has four trace files, one for each core.
3. **`parameter_test_results/`**: Contains results of experiments with varying cache parameters, including detailed logs and summaries.
4. **`report.latex`**: The LaTeX report documenting the implementation, experiments, and analysis.
5. **`Makefile`**: Automates the compilation of the simulator.
6. **`README.md`**: This file, providing an overview and instructions.

---

## How to Compile

1. Ensure you have a C++ compiler (e.g., `g++`) installed.
2. Navigate to the project directory.
3. Run the following command to compile the simulator:
   ```bash
   make
   ```
4. This will generate an executable named `L1simulate`.

---

## How to Run

The simulator takes the following command-line arguments:

```bash
./L1simulate -t <tracefile> -s <s> -E <E> -b <b> -o <outfilename> [-d] [-h]
```

### Command-Line Arguments
- **`-t <tracefile>`**: Prefix of the trace files (e.g., `traces/app1`).
- **`-s <s>`**: Number of set index bits (number of sets = \(2^s\)).
- **`-E <E>`**: Associativity (number of cache lines per set).
- **`-b <b>`**: Number of block bits (block size = \(2^b\)).
- **`-o <outfilename>`**: Output file to log simulation results.
- **`-d`**: Enable debug mode for detailed logs.
- **`-h`**: Display help message.

---

## Example Usage

1. Run the simulator with default parameters:
   ```bash
   ./L1simulate -t traces/app1 -s 6 -E 2 -b 5 -o results.txt
   ```

2. Enable debug mode for detailed logs:
   ```bash
   ./L1simulate -t traces/app1 -s 6 -E 2 -b 5 -o results.txt -d
   ```

3. Display help message:
   ```bash
   ./L1simulate -h
   ```

---

## Simulation Output

The simulator generates the following statistics for each core:
1. Number of read/write instructions.
2. Total execution cycles.
3. Number of idle cycles.
4. Cache miss rate.
5. Number of cache evictions.
6. Number of writebacks.
7. Number of invalidations on the bus.
8. Data traffic (in bytes) on the bus.

Additionally, the overall bus statistics are logged.

---

## Experiments

1. **Default Parameters**:
   - Cache size: 4KB
   - Associativity: 2-way
   - Block size: 32 bytes
   - Run the simulator 10 times and analyze the distribution of outputs.

2. **Parameter Variation**:
   - Vary cache size, associativity, and block size (one at a time) in powers of 2.
   - Plot maximum execution time vs. each parameter.

3. **False Sharing Analysis**:
   - Analyze the impact of block size on false sharing using specific trace patterns.

---

## Report

The detailed implementation, experimental results, and analysis are documented in `report.latex`. Compile it using LaTeX to generate the PDF report.

---

## Assumptions

1. Memory address is 32-bit. Addresses less than 32 bits are zero-padded.
2. Word size is 4 bytes.
3. Write-back, write-allocate, and LRU replacement policies are used.
4. Cache coherence is maintained using the MESI protocol.
5. Cache hit latency: 1 cycle; memory access latency: 100 cycles.
6. Blocking caches: A cache miss halts the core until resolved.

---

## Contact

For any queries, contact:
- Vanshika (2023CS10746)
- Shivankur Gupta (2023CS10809)
