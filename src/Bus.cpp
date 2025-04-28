#include "Bus.hpp"
#include "Cache.hpp"
#include "Core.hpp"
Bus::Bus() : invalidations(0), dataTrafficBytes(0) {}

void Bus::registerCache(Cache *cache)
{
    caches.push_back(cache);
}

//  R : Mem Read    W : RWITM(Write Miss)     I : Invalidate(Write Hit)    B : Writeback

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
            caches[i]->core->totalCycles += penaltyCycles_for_snooping;
        }
    }

    // update the whole below of the logic correctly...when there is traffic when there is not or invalidations
    if (op == 'W' || op == 'I')
    {
        //  it always reads from memory as per the protocol for now. 
        // So no traffic here
        if (cache_sharing) {
            ++invalidations;
        }
    }
    if (op == 'R' || op == 'W')
    {
        int blockSizeBytes = 1 << caches[0]->getBlockBits();
        dataTrafficBytes += blockSizeBytes;
    }
    else if (op == 'I')
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
