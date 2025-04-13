#pragma once
#include "CacheLine.hpp"
#include <vector>

class CacheSet {
public:
    std::vector<CacheLine> lines;

    CacheSet(int associativity);
    int findLine(uint32_t tag);
    int findVictim();
    void updateLRU(int lineIndex, int currentCycle);
};
