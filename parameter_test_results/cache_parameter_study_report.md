# Cache Parameter Study Report

## Overview
This report analyzes the impact of different cache parameters on execution time.

## Parameters Tested
- Set Index Bits (s): [2] (corresponding to [4] sets)
- Associativity (E): [1]
- Block Size Bits (b): [5, 6, 7, 8, 9, 10] (corresponding to [32, 64, 128, 256, 512, 1024] bytes)

## Summary of Results
```
        Test  Sets (2^s)  Associativity (E)  Block Size (bytes)  Cache Size (KB)  Max Execution Time  Core 0 Execution Time  Core 1 Execution Time  Core 2 Execution Time  Core 3 Execution Time
0   s2_E1_b4           4                  1                  16           0.0625                 704                    704                    704                    704                    704
1   s2_E1_b5           4                  1                  32           0.1250                 704                    704                    704                    704                    704
2   s2_E1_b6           4                  1                  64           0.2500                 704                    704                    704                    704                    704
3   s2_E1_b7           4                  1                 128           0.5000                 604                    604                    604                    604                    604
4   s2_E1_b8           4                  1                 256           1.0000                 404                    404                    404                    404                    404
5   s2_E1_b9           4                  1                 512           2.0000                 604                    604                    504                    604                    504
6  s2_E1_b10           4                  1                1024           4.0000                 704                    704                    604                    604                    604
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