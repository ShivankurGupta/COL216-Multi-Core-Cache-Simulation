#include "Bus.hpp"

Bus::Bus() : invalidations(0), dataTrafficBytes(0) {}

void Bus::registerCache(Cache *cache)
{
    caches.push_back(cache);
}

//  R : Read    W : Write     U : Update    B : Writeback

bool Bus::broadcast(uint32_t address, char op, int sourceId)
{
    // bool shared = false;
    bool cache_sharing = false;
    for (size_t i = 0; i < caches.size(); ++i)
    {
        if ((int)i != sourceId)
        {
            int penaltyCycles_for_snooping = 0;
            cache_sharing = cache_sharing || (caches[i]->snoop(address, op, penaltyCycles_for_snooping)); 
            // update the penalty cycles for the snooping cache
            // caches[i]->
            // if (op == 'R' || op == 'W')
            // {
            //     shared = true; // If any cache responds, the block is shared
            // }
        }
    }

    // update the whole below of the logic correctly...when there is traffic when there is not or invalidations
    if (op == 'W' || op == 'U')
    {
        if (cache_sharing) {
            ++invalidations;
        }
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

    return cache_sharing;
}
