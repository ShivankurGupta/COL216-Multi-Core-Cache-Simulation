# Cache Parameter Study Report

## Overview
This report analyzes the impact of different cache parameters on execution time.

## Parameters Tested
- Set Index Bits (s): [2] (corresponding to [4] sets)
- Associativity (E): [1]
- Block Size Bits (b): [2, 3, 4, 5, 6, 7] (corresponding to [4, 8, 16, 32, 64, 128] bytes)

## Summary of Results
```
       Test  Sets (2^s)  Associativity (E)  Block Size (bytes)  Cache Size (KB)  Max Execution Time  Core 0 Execution Time  Core 1 Execution Time  Core 2 Execution Time  Core 3 Execution Time
0  s2_E1_b4           4                  1                  16         0.062500                 404                    404                    404                    404                    404
1  s2_E1_b2           4                  1                   4         0.015625                 704                    704                    704                    704                    704
2  s2_E1_b3           4                  1                   8         0.031250                 604                    604                    604                    604                    604
3  s2_E1_b5           4                  1                  32         0.125000                 604                    604                    504                    604                    504
4  s2_E1_b6           4                  1                  64         0.250000                 704                    704                    604                    604                    604
5  s2_E1_b7           4                  1                 128         0.500000                 704                    704                    704                    704                    704
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