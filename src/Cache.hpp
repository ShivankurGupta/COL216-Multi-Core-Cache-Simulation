#pragma once
#include "CacheSet.hpp"
#include "Bus.hpp"

class Cache {
private:
    int s, E, b;
    int numSets;
    std::vector<CacheSet> sets;
    Bus* bus;
    int coreId;
public:
    Cache(int s, int E, int b, int coreId, Bus* bus);
    bool access(uint32_t address, char op, int cycle, int& penaltyCycles);
    void snoop(uint32_t address, char op, int cycle);
    int getBlockBits() const { return b; }

};
