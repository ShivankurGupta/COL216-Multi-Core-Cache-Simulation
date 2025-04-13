#include "Core.hpp"
#include "Cache.hpp"
#include "Bus.hpp"
#include "Utils.hpp"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    // Parse args
    std::string tracePrefix;
    int s = 0, E = 0, b = 0;
    std::string outFile;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-t") == 0) tracePrefix = argv[++i];
        else if (strcmp(argv[i], "-s") == 0) s = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "-E") == 0) E = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "-b") == 0) b = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "-o") == 0) outFile = argv[++i];
        else if (strcmp(argv[i], "-h") == 0) {
            std::cout << "Usage: ./L1simulate -t <tracefile> -s <s> -E <E> -b <b> -o <outfile>\n";
            return 0;
        }
    }

    Bus bus;
    std::vector<Core*> cores;

    for (int i = 0; i < 4; ++i) {
        Cache* cache = new Cache(s, E, b, i, &bus);
        bus.registerCache(cache);
        cores.push_back(new Core(i, cache));
    }

    for (int i = 0; i < 4; ++i) {
        std::string traceFile = tracePrefix + "_proc" + std::to_string(i) + ".trace";
        cores[i]->recordTrace(traceFile);
    }

    bool finished = false;
    int currentCycle = 0;
    while (!finished) {
        finished = true;
        for (auto& core : cores) {
            if (!core->infile.eof()) {
                finished = false;
                core->processTrace(currentCycle);
            }
        }
        currentCycle++;
    }

    // Output stats
    for (auto& core : cores) {
        std::cout << "Core " << core->id << ":\n";
        std::cout << "  Reads: " << core->readCount << "\n";
        std::cout << "  Writes: " << core->writeCount << "\n";
        std::cout << "  Miss Rate: " << (double)core->cacheMisses / core->totalAccesses << "\n";
        std::cout << "  Total Cycles: " << core->totalCycles << "\n";
        std::cout << "  Idle Cycles: " << core->idleCycles << "\n";
        std::cout << "  Evictions: " << core->evictions << "\n";
        std::cout << "  Writebacks: " << core->writebacks << "\n";
    }

    std::cout << "Invalidations: " << bus.invalidations << "\n";
    std::cout << "Data Traffic (bytes): " << bus.dataTrafficBytes << "\n";
}
