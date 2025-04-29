#include "Core.hpp"
#include "Cache.hpp"
#include "Bus.hpp"
#include <iostream>
#include <cstring>
#include <fstream>

// Global debug flag
bool DEBUG_MODE = true;

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
        else if (strcmp(argv[i], "-d") == 0)
            DEBUG_MODE = true;
        else if (strcmp(argv[i], "-h") == 0)
        {
            std::cout << "Usage: ./L1simulate -t <tracefile> -s <s> -E <E> -b <b> -o <outfile> [-d]\n";
            std::cout << "  -d: Enable debug mode\n";
            return 0;
        }
    }

    if (DEBUG_MODE)
    {
        std::cout << "===== SIMULATION PARAMETERS =====\n";
        std::cout << "Trace prefix: " << tracePrefix << "\n";
        std::cout << "Cache params: s=" << s << " E=" << E << " b=" << b << "\n";
        std::cout << "Output file: " << outFile << "\n";
        std::cout << "================================\n\n";
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
        if (DEBUG_MODE)
        {
            std::cout << "[INIT] Created Core " << i << " with Cache\n";
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        std::string traceFile = tracePrefix + "_proc" + std::to_string(i) + ".trace";
        cores[i]->recordTrace(traceFile);
        if (DEBUG_MODE)
        {
            std::cout << "[INIT] Core " << i << " reading trace file: " << traceFile << "\n";
        }
    }

    bool finished = false;
    int currentCycle = 0;
    while (!finished)
    {
        if (DEBUG_MODE && currentCycle % 1000 == 0)
        {
            std::cout << "\n[CYCLE] ========== Starting Cycle " << currentCycle << " ==========\n";
        }

        finished = true;
        for (auto &core : cores)
        {
            if (!(core->infile.eof()) || core->repeat)
            {
                finished = false;
                if (DEBUG_MODE)
                {
                    std::cout << "[CYCLE " << currentCycle << "] Processing Core " << core->id << "\n";
                }
                core->processTrace(currentCycle);
            }
        }
        currentCycle++;
    }

    if (DEBUG_MODE)
    {
        std::cout << "\n===== SIMULATION COMPLETED =====\n";
        std::cout << "Total Cycles: " << currentCycle << "\n";
        std::cout << "================================\n\n";
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
