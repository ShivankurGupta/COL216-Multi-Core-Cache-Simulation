#include "Core.hpp"
#include "Cache.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

// External debug flag declaration
extern bool DEBUG_MODE;

using namespace std;

Core::Core(int id, Cache *cache)
    : id(id), cache(cache), totalAccesses(0), readCount(0), writeCount(0),
      cacheMisses(0), evictions(0), writebacks(0), totalCycles(-1), idleCycles(0), execCycle(0) {}

void Core::recordTrace(const string &filename)
{
    infile = ifstream(filename);
    if (!infile.is_open())
    {
        cerr << "Error opening trace file: " << filename << endl;
        return;
    }
    if (DEBUG_MODE)
    {
        cout << "[CORE " << id << "] Opened trace file: " << filename << endl;
    }
}

void Core::processTrace(int currentCycle)
{
    if (currentCycle <= totalCycles)
    {
        if (DEBUG_MODE)
        {
            cout << "[CORE " << id << "] Cycle mismatch: current=" << currentCycle << ", total=" << totalCycles << ", skipping" << endl;
        }
        return;
    }

    char op;
    uint32_t address;
    if (!repeat)
    {
        if(DEBUG_MODE)
        {
            cout << "[CORE " << id << "] NOT IN REPEAT" << endl;
        }
        string line;
        getline(infile, line);
        if(DEBUG_MODE)
        {
            cout << "[CORE " << id << "] Read line: " << line << endl;
        }
        if (line.empty() || line[0] == '#')
            return;

        istringstream iss(line);

        if (!(iss >> op >> hex >> address))
        {
            cerr << "Malformed trace line: " << line << endl;
            return;
        }

        address &= 0xFFFFFFFF; // to ensure that all the addresses are 32 bits.

        totalAccesses++;
        if (op == 'R')
            readCount++;
        if (op == 'W')
            writeCount++;

        if (DEBUG_MODE)
        {
            cout << "[CORE " << id << "] Processing " << op << " 0x" << hex << address << dec
                 << " (Access #" << totalAccesses << ")" << endl;
        }
    }
    else
    {
        if(DEBUG_MODE)
        {
            cout << "[CORE " << id << "] IN REPEAT" << endl;
        }
        op = repeat_op;
        address = repeat_address;
        repeat = false;
        if (DEBUG_MODE)
        {
            cout << "[CORE " << id << "] Repeating " << op << " 0x" << hex << address << dec << " from previous cycle" << endl;
        }
    }

    int penalty = 0;
    // cout<<"access called for core id  "<< id << " for address " << hex << address << " with op " << op << endl;
    pair<bool, bool> temp = cache->access(address, op, currentCycle, penalty);
    bool hit = temp.first;
    bool should_repeat = temp.second;

    if (DEBUG_MODE)
    {
        cout << "[CORE " << id << "] Access result: "
             << (hit ? "HIT" : "MISS")
             << (should_repeat ? " (REPEATING)" : "")
             << ", Penalty: " << penalty << " cycles" << endl;
    }

    if (should_repeat)
    {
        repeat = true;
        repeat_op = op;
        repeat_address = address;
        totalCycles += 1;
        idleCycles += 1;
        if (DEBUG_MODE)
        {
            cout << "[CORE " << id << "] Will repeat in next cycle, adding 1 idle cycle" << endl;
        }
        return;
    }

    if (!hit)
        cacheMisses++;

    // AMBER and others -> told to change
    totalCycles += penalty + 1;
    execCycle += penalty;

    if (DEBUG_MODE)
    {
        cout << "[CORE " << id << "] Adding " << penalty << " cycles, total: " << totalCycles << endl;
        cout << "[CORE " << id << "] Stats: Misses=" << cacheMisses << ", Evictions=" << evictions
             << ", Writebacks=" << writebacks << endl;
    }

    // infile.close();
}
