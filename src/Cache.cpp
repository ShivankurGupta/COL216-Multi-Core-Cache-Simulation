#include "Cache.hpp"
#include <cmath>

Cache::Cache(int s, int E, int b, int coreId, Bus *bus)
    : s(s), E(E), b(b), coreId(coreId), bus(bus)
{
    numSets = 1 << s;
    sets.reserve(numSets);
    for (int i = 0; i < numSets; ++i)
    {
        sets.emplace_back(E);
    }
}

bool Cache::access(uint32_t address, char op, int cycle, int &penaltyCycles)
{
    uint32_t blockOffset = address & ((1 << b) - 1);
    uint32_t setIndex = (address >> b) & ((1 << s) - 1);
    uint32_t tag = address >> (s + b);

    CacheSet &set = sets[setIndex];
    int lineIndex = set.findLine(tag);

    // Cache hit
    if (lineIndex != -1 && set.lines[lineIndex].state != INVALID)
    {
        set.updateLRU(lineIndex, cycle);
        if (op == 'W')
        {
            set.lines[lineIndex].dirty = true;
            if (set.lines[lineIndex].state == SHARED)
            {
                // Upgrade to MODIFIED (BusUpgr)
                bus->broadcast(address, 'U', coreId);
                set.lines[lineIndex].state = MODIFIED;
                penaltyCycles += 2;
            }
            else if (set.lines[lineIndex].state == EXCLUSIVE)
            {
                set.lines[lineIndex].state = MODIFIED;
            }
        }
        return true;
    }

    // Cache miss
    penaltyCycles += 100; // Memory fetch
    int victimIndex = set.findVictim();

    if (set.lines[victimIndex].valid && set.lines[victimIndex].dirty)
    {
        // Writeback dirty line
        penaltyCycles += 100;
        bus->broadcast(address, 'B', coreId); // Corrected from recordWriteback
    }

    // BusRd or BusRdX
    if (op == 'R')
    {
        bool shared = bus->broadcast(address, 'R', coreId); // Ensure shared status is returned
        set.lines[victimIndex] = {tag, shared ? SHARED : EXCLUSIVE, true, false, cycle};
        penaltyCycles += shared ? 2 * (1 << b) : 0;
    }
    else
    {
        bool shared = bus->broadcast(address, 'W', coreId); // Ensure shared status is returned
        set.lines[victimIndex] = {tag, MODIFIED, true, true, cycle};
        penaltyCycles += shared ? 2 * (1 << b) : 0;
    }

    return false;
}

void Cache::snoop(uint32_t address, char op, int cycle)
{
    uint32_t setIndex = (address >> b) & ((1 << s) - 1);
    uint32_t tag = address >> (s + b);

    CacheSet &set = sets[setIndex];
    int lineIndex = set.findLine(tag);

    if (lineIndex == -1 || set.lines[lineIndex].state == INVALID)
        return;

    MESIState &state = set.lines[lineIndex].state;

    if (op == 'R')
    {
        if (state == MODIFIED)
        {
            set.lines[lineIndex].dirty = false;
        }
        state = SHARED;
    }
    else if (op == 'W' || op == 'U')
    {
        state = INVALID;
    }

    set.updateLRU(lineIndex, cycle);
}

int Cache::getBlockBits() const
{
    return b;
}
