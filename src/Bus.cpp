#include "Bus.hpp"
#include "Cache.hpp"
#include "Core.hpp"
#include <iostream>

// External debug flag declaration
extern bool DEBUG_MODE;

Bus::Bus() : invalidations(0), dataTrafficBytes(0), bus_cycles(-1)
{
    if (DEBUG_MODE)
    {
        std::cout << "[BUS] Initialized" << std::endl;
    }
}

void Bus::registerCache(Cache *cache)
{
    caches.push_back(cache);
    if (DEBUG_MODE)
    {
        std::cout << "[BUS] Registered cache " << caches.size() - 1 << std::endl;
    }
}

//  R : Mem Read    W : RWITM(Write Miss)     I : Invalidate(Write Hit)    B : Writeback

bool Bus::broadcast(uint32_t address, char op, int sourceId)
{
    if (DEBUG_MODE)
    {
        std::cout << "[BUS] Broadcasting: " << op << " 0x" << std::hex << address
                  << " from source " << std::dec << sourceId << std::endl;

        std::string opName;
        switch (op)
        {
        case 'R':
            opName = "Read (BusRd)";
            break;
        case 'W':
            opName = "RWITM (BusRdX)";
            break;
        case 'I':
            opName = "Invalidate";
            break;
        case 'B':
            opName = "Writeback";
            break;
        default:
            opName = "Unknown";
            break;
        }
        std::cout << "[BUS] Operation: " << opName << std::endl;
    }

    bool cache_sharing = false;
    for (size_t i = 0; i < caches.size(); ++i)
    {
        if ((int)i != sourceId)
        {
            int penaltyCycles_for_snooping = 0;

            if (DEBUG_MODE)
            {
                std::cout << "[BUS] Snooping cache " << i << std::endl;
            }

            bool responded = caches[i]->snoop(address, op, penaltyCycles_for_snooping);
            cache_sharing = cache_sharing || responded;

            if (DEBUG_MODE && responded)
            {
                std::cout << "[BUS] Cache " << i << " responded, penalty="
                          << penaltyCycles_for_snooping << " cycles" << std::endl;
            }

            caches[i]->core->totalCycles += penaltyCycles_for_snooping;
        }
    }

    if (DEBUG_MODE)
    {
        std::cout << "[BUS] Cache sharing: " << (cache_sharing ? "YES" : "NO") << std::endl;
    }

    if (op == 'W' || op == 'I')
    {
        if (cache_sharing)
        {
            ++invalidations;
            if (DEBUG_MODE)
            {
                std::cout << "[BUS] Incrementing invalidations to " << invalidations << std::endl;
            }
        }
    }

    if (op == 'R' || op == 'W')
    {
        int blockSizeBytes = 1 << caches[0]->getBlockBits();
        dataTrafficBytes += blockSizeBytes;
        if (DEBUG_MODE)
        {
            std::cout << "[BUS] Adding " << blockSizeBytes << " bytes to data traffic, total="
                      << dataTrafficBytes << std::endl;
        }
    }
    else if (op == 'I')
    {
        dataTrafficBytes += 0;
    }
    else if (op == 'B')
    { // Handle writeback
        int blockSizeBytes = 1 << caches[0]->getBlockBits();
        dataTrafficBytes += blockSizeBytes;
        if (DEBUG_MODE)
        {
            std::cout << "[BUS] Writeback: adding " << blockSizeBytes
                      << " bytes to data traffic, total=" << dataTrafficBytes << std::endl;
        }
    }

    return cache_sharing;
}
