#pragma once
#include <cstdint>
#include <vector>

enum MESIState { MODIFIED, EXCLUSIVE, SHARED, INVALID, EMPTY };

struct CacheLine {
    uint32_t tag;
    MESIState state;
    // bool valid;
    // bool dirty;
    int lastUsedCycle;
};
