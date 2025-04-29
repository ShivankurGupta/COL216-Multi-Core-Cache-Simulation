#include "CacheSet.hpp"
#include <iostream>

// External debug flag declaration
extern bool DEBUG_MODE;

// Helper function declared for debugging
const char *stateToString(MESIState state);

CacheSet::CacheSet(int associativity)
{
    lines.resize(associativity);
    for (auto &line : lines)
    {
        line.state = EMPTY; // Initialize all lines to EMPTY state
        line.lastUsedCycle = -1;
    }
}

int CacheSet::findLine(uint32_t tag)
{
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (!(lines[i].state == INVALID || lines[i].state == EMPTY) && lines[i].tag == tag)
        {
            if (DEBUG_MODE)
            {
                std::cout << "[SET] Found line with tag 0x" << std::hex << tag << " at index "
                          << std::dec << i << " with state " << stateToString(lines[i].state) << std::endl;
            }
            return static_cast<int>(i);
        }
    }
    if (DEBUG_MODE)
    {
        std::cout << "[SET] Line with tag 0x" << std::hex << tag << " not found" << std::dec << std::endl;
    }
    return -1; // Not found
}

int CacheSet::findVictim()
{
    int lruIndex = 0;
    int minCycle = lines[0].lastUsedCycle;

    for (size_t i = 1; i < lines.size(); ++i)
    {
        if (lines[i].state == EMPTY)
        {
            if (DEBUG_MODE)
            {
                std::cout << "[SET] Found EMPTY line at index " << i << " for victim" << std::endl;
            }
            return static_cast<int>(i); // Choose an invalid line if available
        }
        if (lines[i].lastUsedCycle < minCycle)
        {
            lruIndex = static_cast<int>(i);
            minCycle = lines[i].lastUsedCycle;
        }
    }

    if (DEBUG_MODE)
    {
        std::cout << "[SET] Selected LRU victim at index " << lruIndex
                  << " with lastUsedCycle=" << minCycle
                  << ", state=" << stateToString(lines[lruIndex].state) << std::endl;
    }

    return lruIndex;
}

void CacheSet::updateLRU(int lineIndex, int currentCycle)
{
    if (lineIndex >= 0 && lineIndex < static_cast<int>(lines.size()))
    {
        if (DEBUG_MODE)
        {
            std::cout << "[SET] Updating LRU for line " << lineIndex
                      << " from cycle " << lines[lineIndex].lastUsedCycle
                      << " to " << currentCycle << std::endl;
        }
        lines[lineIndex].lastUsedCycle = currentCycle;
    }
}
