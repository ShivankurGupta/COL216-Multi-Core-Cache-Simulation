#include "Core.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

Core::Core(int id, Cache *cache)
    : id(id), cache(cache), totalAccesses(0), readCount(0), writeCount(0),
      cacheMisses(0), evictions(0), writebacks(0), totalCycles(0), idleCycles(0) {}

void Core::recordTrace(const string &filename)
{
    infile = ifstream(filename);
    if (!infile.is_open())
    {
        cerr << "Error opening trace file: " << filename << endl;
        return;
    }
}

void Core::processTrace(int currentCycle)
{
    if (currentCycle != totalCycles)
    {
        return;
    }
    char op;
    uint32_t address;
    if (!repeat)
    {
        string line;
        getline(infile, line);
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
    }
    else
    {
        op = repeat_op;
        address = repeat_address;
        repeat = false;
    }

    int penalty = 0;

    pair<bool, bool> temp = cache->access(address, op, currentCycle, penalty);
    bool hit = temp.first;
    bool should_repeat = temp.second;
    if (should_repeat)
    {
        repeat = true;
        repeat_op = op;
        repeat_address = address;
        totalCycles += 1;
        idleCycles += 1;
        return;
    }

    if (!hit)
        cacheMisses++;

    totalCycles += penalty > 0 ? penalty : 1;
    idleCycles += penalty;

    infile.close();
}
