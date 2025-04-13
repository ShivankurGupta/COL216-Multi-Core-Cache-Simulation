#include "Bus.hpp"

Bus::Bus() : invalidations(0), dataTrafficBytes(0) {}

void Bus::registerCache(Cache *cache)
{
    caches.push_back(cache);
}

bool Bus::broadcast(uint32_t address, char op, int sourceId)
{
    bool shared = false;
    for (size_t i = 0; i < caches.size(); ++i)
    {
        if ((int)i != sourceId)
        {
            caches[i]->snoop(address, op, 0); // Passing 0 as cycle for snoops from other cores
            if (op == 'R' || op == 'W')
            {
                shared = true; // If any cache responds, the block is shared
            }
        }
    }

    if (op == 'W' || op == 'U')
    {
        ++invalidations;
    }

    if (op == 'R' || op == 'W')
    {
        int blockSizeBytes = 1 << caches[0]->getBlockBits();
        dataTrafficBytes += blockSizeBytes;
    }
    else if (op == 'U')
    {
        dataTrafficBytes += 0;
    }
    else if (op == 'B')
    { // Handle writeback
        int blockSizeBytes = 1 << caches[0]->getBlockBits();
        dataTrafficBytes += blockSizeBytes;
    }

    return shared;
}
