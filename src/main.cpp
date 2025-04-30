#include "Core.hpp"
#include "Cache.hpp"
#include "Bus.hpp"
#include <iostream>
#include <cstring>
#include <fstream>

// Global debug flag
bool DEBUG_MODE = false;

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
    // Simulation Parameters
    outFileStream << "Simulation Parameters:\n";
    outFileStream << "Trace Prefix: " << tracePrefix << "\n";
    outFileStream << "Set Index Bits: " << s << "\n";
    outFileStream << "Associativity: " << E << "\n";
    outFileStream << "Block Bits: " << b << "\n";
    outFileStream << "Block Size (Bytes): " << (1 << b) << "\n";
    outFileStream << "Number of Sets: " << (1 << s) << "\n";
    outFileStream << "Cache Size (KB per core): " << ((1 << s) * E * (1 << b)) / 1024.0 << "\n";
    outFileStream << "MESI Protocol: Enabled\n";
    outFileStream << "Write Policy: Write-back, Write-allocate\n";
    outFileStream << "Replacement Policy: LRU\n";
    outFileStream << "Bus: Central snooping bus\n\n";

    // Core Statistics
    for (auto &core : cores)
    {
        outFileStream << "Core " << core->id << " Statistics:\n";
        outFileStream << "Total Instructions: " << core->readCount + core->writeCount << "\n";
        outFileStream << "Total Reads: " << core->readCount << "\n";
        outFileStream << "Total Writes: " << core->writeCount << "\n";
        outFileStream << "Total Execution Cycles: " << core->execCycle << "\n";
        outFileStream << "Idle Cycles: " << core->idleCycles << "\n";
        outFileStream << "Cache Misses: " << core->cacheMisses << "\n";
        outFileStream << "Cache Miss Rate: " << (core->totalAccesses > 0 ? (double)core->cacheMisses / core->totalAccesses * 100 : 0) << "%\n";
        outFileStream << "Cache Evictions: " << core->evictions << "\n";
        outFileStream << "Writebacks: " << core->writebacks << "\n";
        outFileStream << "Bus Invalidations: " << core->invalidations << "\n";
        outFileStream << "Data Traffic (Bytes): " << core->dataTraffic << "\n\n";
    }

    // Overall Bus Summary
    outFileStream << "Overall Bus Summary:\n";
    outFileStream << "Total Bus Transactions: " << bus.transactions << "\n";
    outFileStream << "Total Bus Traffic (Bytes): " << bus.dataTrafficBytes << "\n";

    outFileStream.close();
}
