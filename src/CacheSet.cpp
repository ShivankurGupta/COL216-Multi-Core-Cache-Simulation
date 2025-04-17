#include "CacheSet.hpp"

CacheSet::CacheSet(int associativity) {
    lines.resize(associativity);
    for (auto& line : lines) {
        line.state = INVALID;
        line.lastUsedCycle = -1;
    }
}

int CacheSet::findLine(uint32_t tag) {
    for (size_t i = 0; i < lines.size(); ++i) {
        if (!(lines[i].state == INVALID) && lines[i].tag == tag) {
            return static_cast<int>(i);
        }
    }
    return -1; // Not found
}

int CacheSet::findVictim() {
    int lruIndex = 0;
    int minCycle = lines[0].lastUsedCycle;

    for (size_t i = 1; i < lines.size(); ++i) {
        if (lines[i].state == INVALID) {
            return static_cast<int>(i); // Choose an invalid line if available
        }
        if (lines[i].lastUsedCycle < minCycle) {
            lruIndex = static_cast<int>(i);
            minCycle = lines[i].lastUsedCycle;
        }
    }
    return lruIndex;
}

void CacheSet::updateLRU(int lineIndex, int currentCycle) {
    if (lineIndex >= 0 && lineIndex < static_cast<int>(lines.size())) {
        lines[lineIndex].lastUsedCycle = currentCycle;
    }
}
