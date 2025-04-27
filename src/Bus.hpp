#pragma once
#include <vector>
#include <cstdint>
#include "Cache.hpp"

class Bus
{
public:
    std::vector<Cache *> caches;
    int invalidations;
    int dataTrafficBytes;
    bool broadcast(uint32_t address, char op, int sourceId); // Changed return type to bool
    void registerCache(Cache *cache);
    int bus_cycles = 0;
    Bus();
};
