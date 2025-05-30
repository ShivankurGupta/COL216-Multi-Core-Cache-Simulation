#pragma once
#include "CacheSet.hpp"
#include <vector>

class Bus;
class Core;

class Cache
{
private:
    int s, E, b;
    int numSets;
    std::vector<CacheSet> sets;
    Bus *bus; // Pointer to Bus
    int coreId;
    
    public:
    Core* core; // Pointer to Core
    Cache(int s, int E, int b, int coreId, Bus *bus);
    void add_core(Core* core);
    std::pair<bool, bool> access(uint32_t address, char op, int cycle, int &penaltyCycles);
    bool snoop(uint32_t address, char op, int &penaltyCycles,bool Addtraffic);
    int getBlockBits() const { return b; }
};
