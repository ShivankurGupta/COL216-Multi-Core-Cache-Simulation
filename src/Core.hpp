#pragma once
#include "Cache.hpp"
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

class Core {
public:
    ifstream infile;
    int id;
    Cache* cache;
    int totalAccesses;
    int readCount, writeCount;
    int cacheMisses;
    int evictions;
    int writebacks;
    int totalCycles;
    int idleCycles;

    bool repeat = false;
    char repeat_op;
    uint32_t repeat_address;

    Core(int id, Cache* cache);
    void recordTrace(const std::string& filename);
    void processTrace(int currentCycle);
};
