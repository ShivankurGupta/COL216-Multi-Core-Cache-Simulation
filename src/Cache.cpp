#include "Cache.hpp"
#include "Bus.hpp"
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

    // Update LRU for the accessed line (independent of hit/miss)
    set.updateLRU(lineIndex, cycle);
    // Cache hit
    if (lineIndex != -1 && set.lines[lineIndex].state != INVALID)
    {
        if (op == 'W')
        {
            // set.lines[lineIndex].dirty = true;
            if (set.lines[lineIndex].state == SHARED)
            {
                // Upgrade to MODIFIED (BusUpgr)
                bus->broadcast(address, 'U', coreId);
                // set.lines[lineIndex].state = MODIFIED;
                // penaltyCycles += 2;
            }
            // else if (set.lines[lineIndex].state == EXCLUSIVE)
            // {
            //     set.lines[lineIndex].state = MODIFIED;
            // }
            set.lines[lineIndex].state = MODIFIED;
        }
        return true;
    }

    // Cache miss

    // Check if other caches have that value. Send a bus request for this address

    // penaltyCycles += 100; // Memory fetch  --> We don't know whether memory access or other cache gives value
    int victimIndex = set.findVictim();

    // there are two cases now, either we have a free line or we need to evict a line
    //  for a free line the initial state is INVALID, so it won't go into the next if 

    if (set.lines[victimIndex].state == MODIFIED)
    {
        // Writeback dirty line
        penaltyCycles += 100;
        bus->broadcast(address, 'B', coreId); // Corrected from recordWriteback
    }

    // BusRd or BusRdX
    if (op == 'R')
    {
        bool shared = bus->broadcast(address, 'R', coreId); // Ensure shared status is returned
        set.lines[victimIndex] = {tag, shared ? SHARED : EXCLUSIVE, cycle};
        penaltyCycles += shared ? 2 * (1 << b) : 100;
    }
    else
    {
        bus->broadcast(address, 'W', coreId); // Ensure shared status is returned
        set.lines[victimIndex] = {tag, MODIFIED, cycle};
        penaltyCycles += 100;
    }

    return false;
}

bool Cache::snoop(uint32_t address, char op, int &penaltyCycles)
{
    uint32_t setIndex = (address >> b) & ((1 << s) - 1);
    uint32_t tag = address >> (s + b);

    CacheSet &set = sets[setIndex];
    int lineIndex = set.findLine(tag);

    if (lineIndex == -1 || set.lines[lineIndex].state == INVALID)
        return false;

    MESIState &state = set.lines[lineIndex].state;
    
    // if there is sharing then always return true.

    if (op == 'R')
    {
        if (state == MODIFIED)
        {
            // write back to the memory 
            bus->broadcast(address, 'B', coreId);
            // increase the penalty cycles for this snooping cache
            penaltyCycles += 100;

            // set.lines[lineIndex].dirty = false;
        }
        state = SHARED;
    }
    else if (op == 'W' || op == 'U')
    {
        if (state == MODIFIED) {
            //             Snooping processor sees this
            //  Blocks RWITM request
            //  Takes control of bus
            //  Writes back its copy to memory
            bus->broadcast(address, 'B', coreId);
            penaltyCycles += 100;
        }

        // W when the access cache had write miss, U when it was in shared
        state = INVALID;
    }
    return true ;
    // set.updateLRU(lineIndex, cycle);
}