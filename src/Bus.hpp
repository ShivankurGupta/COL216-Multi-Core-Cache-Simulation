#pragma once
#include <vector>
#include <cstdint>

class Cache;

class Bus
{
public:
    std::vector<Cache *> caches;
    int invalidations;
    int dataTrafficBytes;
    int transactions;
    bool broadcast(uint32_t address, char op, int sourceId); // Changed return type to bool
    void registerCache(Cache *cache);
    int bus_cycles = -1;
    Bus();
};
