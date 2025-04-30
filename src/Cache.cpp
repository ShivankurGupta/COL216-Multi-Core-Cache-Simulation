#include "Cache.hpp"
#include "Core.hpp"
#include "Bus.hpp"
#include <cmath>
using namespace std;
#include <iostream>

// External debug flag declaration
extern bool DEBUG_MODE;

// Helper function to convert MESI state to string for debugging
const char *stateToString(MESIState state)
{
    switch (state)
    {
    case MODIFIED:
        return "MODIFIED";
    case EXCLUSIVE:
        return "EXCLUSIVE";
    case SHARED:
        return "SHARED";
    case INVALID:
        return "INVALID";
    case EMPTY:
        return "EMPTY";
    default:
        return "UNKNOWN";
    }
}

Cache::Cache(int s, int E, int b, int coreId, Bus *bus)
    : s(s), E(E), b(b), bus(bus), coreId(coreId)
{
    numSets = 1 << s;
    sets.reserve(numSets);
    for (int i = 0; i < numSets; ++i)
    {
        sets.emplace_back(E);
    }
    if (DEBUG_MODE)
    {
        cout << "[CACHE " << coreId << "] Created with " << numSets << " sets, "
             << E << " lines per set, " << (1 << b) << " bytes per block" << endl;
    }
}

pair<bool, bool> Cache::access(uint32_t address, char op, int cycle, int &penaltyCycles)
{
    uint32_t blockOffset = address & ((1 << b) - 1);
    uint32_t setIndex = (address >> b) & ((1 << s) - 1);
    uint32_t tag = address >> (s + b);

    if (DEBUG_MODE)
    {
        cout << "[CACHE " << coreId << "] Access: " << op << " 0x" << hex << address
             << " (set=" << dec << setIndex << ", tag=0x" << hex << tag
             << ", offset=" << dec << blockOffset << ")" << endl;
    }

    CacheSet &set = sets[setIndex];
    int lineIndex = set.findLine(tag);

    // Update LRU for the accessed line (independent of hit/miss)
    set.updateLRU(lineIndex, cycle);

    // Cache hit
    if (lineIndex != -1 && set.lines[lineIndex].state != INVALID)
    {
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] HIT in line " << lineIndex
                 << ", state=" << stateToString(set.lines[lineIndex].state) << endl;
        }

        if (op == 'W')
        {
            if (set.lines[lineIndex].state == SHARED)
            {
                if (bus->bus_cycles >= cycle)
                {
                    if (DEBUG_MODE)
                    {
                        cout << "[CACHE " << coreId << "] Bus busy, will repeat access" << endl;
                    }
                    return {true, true};
                }

                if (DEBUG_MODE)
                {
                    cout << "[CACHE " << coreId << "] Write to SHARED, broadcasting invalidate" << endl;
                }

                bus->broadcast(address, 'I', coreId);
                bus->bus_cycles += 1;
            }

            if (DEBUG_MODE)
            {
                cout << "[CACHE " << coreId << "] Changing state to MODIFIED" << endl;
            }

            set.lines[lineIndex].state = MODIFIED;
        }
        return {true, false};
    }

    // Cache miss
    if (DEBUG_MODE)
    {
        cout << "[CACHE " << coreId << "] MISS" << endl;
    }

    if (bus->bus_cycles >= cycle)
    {
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Bus busy, will repeat access" << endl;
        }
        return {true, true};
    }

    bus->bus_cycles = cycle;

    int victimIndex = set.findVictim();
    
    if (!(set.lines[victimIndex].state == EMPTY))
    {
        core->evictions++;
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Evicting line " << victimIndex
                 << ", state=" << stateToString(set.lines[victimIndex].state)
                 << ", tag=0x" << hex << set.lines[victimIndex].tag << dec << endl;
        }
    }

    if (set.lines[victimIndex].state == MODIFIED)
    {
        core->execCycle += 100;
        // Writeback dirty line
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Writeback required for modified line" << endl;
        }

        penaltyCycles += 100;
        bus->broadcast(address, 'B', coreId);
        core->dataTraffic += (1<<b);
        core->writebacks++;
        bus->bus_cycles += 100;
    }

    // BusRd or BusRdX
    if (op == 'R')
    {
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Read miss, broadcasting BusRd" << endl;
        }

        bool shared = bus->broadcast(address, 'R', coreId);

        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Line " << (shared ? "SHARED" : "EXCLUSIVE")
                 << " in other caches" << endl;
        }

        set.lines[victimIndex] = {tag, shared ? SHARED : EXCLUSIVE, cycle};
        penaltyCycles += shared ? 2 * (1 << (b - 2)) : 100;
        bus->bus_cycles += shared ? 2 * (1 << (b - 2)) : 100;

        core->dataTraffic += (1<<b);

        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Added penalty: " << penaltyCycles
                 << " cycles, new state=" << stateToString(set.lines[victimIndex].state) << endl;
        }
    }
    else
    {
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Write miss, broadcasting BusRdX" << endl;
        }

        bus->broadcast(address, 'W', coreId);
        bus->bus_cycles += 100;
        core->dataTraffic += (1<<b);
        set.lines[victimIndex] = {tag, MODIFIED, cycle};
        penaltyCycles += 200;

        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Added penalty: " << penaltyCycles
                 << " cycles, new state=MODIFIED" << endl;
        }
    }

    return {false, false};
}

bool Cache::snoop(uint32_t address, char op, int &penaltyCycles)
{
    uint32_t setIndex = (address >> b) & ((1 << s) - 1);
    uint32_t tag = address >> (s + b);

    if (DEBUG_MODE)
    {
        cout << "[CACHE " << coreId << "] Snooping: " << op << " 0x" << hex << address
             << " (set=" << dec << setIndex << ", tag=0x" << hex << tag << ")" << endl;
    }

    CacheSet &set = sets[setIndex];
    int lineIndex = set.findLine(tag);

    if (lineIndex == -1 || set.lines[lineIndex].state == INVALID || set.lines[lineIndex].state == EMPTY)
    {
        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Snooping: Line not found or invalid" << endl;
        }
        return false;
    }

    MESIState &state = set.lines[lineIndex].state;

    if (DEBUG_MODE)
    {
        cout << "[CACHE " << coreId << "] Snooping: Found line " << lineIndex
             << " with state=" << stateToString(state) << endl;
    }

    if (op == 'R')
    {
        if (state == MODIFIED)
        {
            if (DEBUG_MODE)
            {
                cout << "[CACHE " << coreId << "] Snooping: MODIFIED line, writeback required" << endl;
            }

            bus->broadcast(address, 'B', coreId);
            bus->bus_cycles += 100;
            core->writebacks++;
            penaltyCycles += 100;
        }

        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Snooping: Changing state to SHARED" << endl;
        }
        core->dataTraffic += (1<<b);
        state = SHARED;
    }
    else if (op == 'W' || op == 'I')
    {
        if (state == MODIFIED)
        {
            if (DEBUG_MODE)
            {
                cout << "[CACHE " << coreId << "] Snooping: MODIFIED line, writeback required" << endl;
            }

            bus->broadcast(address, 'B', coreId);
            core->dataTraffic += (1<<b);
            bus->bus_cycles += 100;
            core->writebacks++;
            penaltyCycles += 100;
        }

        if (DEBUG_MODE)
        {
            cout << "[CACHE " << coreId << "] Snooping: Invalidating line" << endl;
        }

        state = INVALID;
    }
    return true;
}

void Cache::add_core(Core *core)
{
    this->core = core;
}