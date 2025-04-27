#include "Core.hpp"
#include "Cache.hpp"
#include "Bus.hpp"
#include <iostream>
#include <cstring>
#include <fstream>

int main(int argc, char *argv[])
{
    // Parse args
    std::string tracePrefix;
    int s = 0, E = 0, b = 0;
    std::string outFile;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-t") == 0)
            tracePrefix = argv[++i];
        else if (strcmp(argv[i], "-s") == 0)
            s = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "-E") == 0)
            E = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "-b") == 0)
            b = std::stoi(argv[++i]);
        else if (strcmp(argv[i], "-o") == 0)
            outFile = argv[++i];
        else if (strcmp(argv[i], "-h") == 0)
        {
            std::cout << "Usage: ./L1simulate -t <tracefile> -s <s> -E <E> -b <b> -o <outfile>\n";
            return 0;
        }
    }

    std::ofstream outFileStream(outFile);
    if (!outFileStream.is_open())
    {
        std::cerr << "Error: Could not open output file " << outFile << "\n";
        return 1;
    }

    Bus bus;
    std::vector<Core *> cores;

    for (int i = 0; i < 4; ++i)
    {
        Cache *cache = new Cache(s, E, b, i, &bus);
        bus.registerCache(cache);
        cores.push_back(new Core(i, cache));
        (*cache).add_core(cores[i]);
    }

    for (int i = 0; i < 4; ++i)
    {
        std::string traceFile = tracePrefix + "_proc" + std::to_string(i) + ".trace";
        cores[i]->recordTrace(traceFile);
    }

    bool finished = false;
    int currentCycle = 0;
    while (!finished)
    {
        finished = true;
        for (auto &core : cores)
        {
            if (!core->infile.eof())
            {
                finished = false;
                core->processTrace(currentCycle);
            }
        }
        currentCycle++;
    }

    // Output stats
    for (auto &core : cores)
    {
        outFileStream << "Core " << core->id << ":\n";
        outFileStream << "  Reads: " << core->readCount << "\n";
        outFileStream << "  Writes: " << core->writeCount << "\n";
        outFileStream << "  Miss Rate: " << (double)core->cacheMisses / core->totalAccesses << "\n";
        outFileStream << "  Total Cycles: " << core->totalCycles << "\n";
        outFileStream << "  Idle Cycles: " << core->idleCycles << "\n";
        outFileStream << "  Evictions: " << core->evictions << "\n";
        outFileStream << "  Writebacks: " << core->writebacks << "\n";
    }

    outFileStream << "Invalidations: " << bus.invalidations << "\n";
    outFileStream << "Data Traffic (bytes): " << bus.dataTrafficBytes << "\n";

    outFileStream.close();
}
