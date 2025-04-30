# Cache Parameter Study Report

## Overview
This report analyzes the impact of different cache parameters on execution time.

## Parameters Tested
- Set Index Bits (s): [3, 4, 5] (corresponding to [8, 16, 32] sets)
- Associativity (E): [1, 2, 4]
- Block Size Bits (b): [4, 5, 6] (corresponding to [16, 32, 64] bytes)

## Summary of Results
```
       Test  Sets (2^s)  Associativity (E)  Block Size (bytes)  Cache Size (KB)  Max Execution Time  Core 0 Execution Time  Core 1 Execution Time  Core 2 Execution Time  Core 3 Execution Time
0  s3_E1_b5           8                  1                  32             0.25                 102                    102                     18                     18                    101
1  s4_E1_b5          16                  1                  32             0.50                 102                    102                     18                     18                    101
2  s5_E1_b5          32                  1                  32             1.00                 102                    102                     18                     18                    101
3  s4_E2_b5          16                  2                  32             1.00                 102                    102                     18                     18                    101
4  s4_E4_b5          16                  4                  32             2.00                 102                    102                     18                     18                    101
5  s4_E1_b4          16                  1                  16             0.25                 210                    102                     10                    210                    101
6  s4_E1_b6          16                  1                  64             1.00                 102                    102                     34                     34                    101
```

## Observations

### Impact of Cache Size (varying s)
Increasing the number of sets typically leads to better performance up to a point, as it reduces conflict misses. However, beyond that point, the benefit may diminish due to other factors like associativity and block size limitations.

### Impact of Associativity (varying E)
Higher associativity generally reduces conflict misses, improving performance. However, very high associativity may lead to diminishing returns and increased hardware complexity. The sweet spot often depends on the specific workload.

### Impact of Block Size (varying b)
Larger block sizes can improve spatial locality and reduce compulsory misses. However, too large blocks can lead to cache pollution and increased miss penalties. The optimal block size depends on the spatial locality in the workload.

## Plots
See the plots directory for visual representations of these relationships.

## Conclusion
The optimal cache configuration depends on the specific workload characteristics. This study helps identify how different parameters affect execution time for the given application traces.