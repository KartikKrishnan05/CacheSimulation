//#include "simulation.hpp"
#include "cache.hpp"
#include "simulation.hpp"
#include <stdio.h>
#include <systemc>
#include <systemc.h>



//Run_simulation Methode
Result run_simulation(int cycles, unsigned l1CacheLines, unsigned l2CacheLines, unsigned cacheLineSize,
                      unsigned l1CacheLatency, unsigned l2CacheLatency, unsigned memoryLatency,
                      size_t numRequests, Request requests[], const char* tracefile) {

    
    std::cout << "\n Simulation Start: \n" << std::endl;


    
    //Initialisiere Result struct für Rückgabe
    Result result;

    //Initialisiere Cache- und Simulationsmodul sowie Clock
    CACHE cache("cache");
    SIMULATION sim("sim");
    sc_clock clock("clk", 10, SC_SEC, 0.4, 5, SC_SEC, true);

    //Initialisiere signale für we, addr, data_in, hit & data_out
    sc_signal<int> we;
    sc_signal<uint32_t> addr;
    sc_signal<uint32_t> data_in;
    sc_signal<bool> hit;
    sc_signal<uint32_t> data_out;
    //Initialize Eingaben von simulation :)
    sim.clk(clock);
    sim.requests = requests;
    sim.numRequests = numRequests;

    //Initialisiere Tracefile 
    sc_trace_file* file = nullptr;

    //Erstelle Tracefile falls übergebener Parameter ungleich null ist
    if (tracefile != nullptr) {
        file = sc_create_vcd_trace_file(tracefile);
        sc_trace(file, clock, "clk");
        
        sc_trace(file, we, "we");
        sc_trace(file, addr, "addr");
        sc_trace(file, data_in, "data_in");
        sc_trace(file, data_out, "data_out");
        sc_trace(file, hit, "hit");
    }

    //Initialisiere Eingaben für Cache
    cache.hit(hit);
    cache.data_out(data_out);
    cache.max_cycles = cycles;
    cache.l1CacheLines = l1CacheLines;
    cache.l2CacheLines = l2CacheLines;
    cache.lineSize = cacheLineSize;
    cache.numRequests = numRequests;
    cache.l1CacheLatency = l1CacheLatency;
    cache.l2CacheLatency = l2CacheLatency;
    cache.memoryLatency = memoryLatency;
    cache.clk(clock);

    //Initialisiere arrays von cache
    cache.dataStorage1 = new uint8_t[l1CacheLines*4];
    cache.tagStorage1 = new uint8_t[l1CacheLines*4];
    cache.dataStorage2 = new uint8_t[l2CacheLines*4];
    cache.tagStorage2 = new uint8_t[l2CacheLines*4];
    cache.tagStorageMem = new uint8_t[numRequests*4];
    cache.dataStorageMem = new uint8_t[numRequests*4];


    //Binde signale an ports
    sim.we(we);
    sim.addr(addr);
    sim.data(data_in);

    //Initialisiere we, addr und data_in gleichen Signalen wie bei sim
    cache.we(we);
    cache.addr(addr);
    cache.data_in(data_in);

    //Anzahl der Gatter die zum Speichern von 32 bit Werten in dem L1 und L2 notwendig sind
    cache.primitiveGateCount += l1CacheLines*32*4 + l2CacheLines*32*4;

    //Starte Simulation und schließe Tarcefile falls nötig
    sc_start();

    if (file != nullptr) {
        sc_close_vcd_trace_file(file);
    }

    std::cout << "Reads: " << sim.reads + cache.reads << " Writes: " << sim.writes + cache.writes << std::endl;

    //Initialisiere Result Struct
    result.cycles = cache.cycles;
    result.misses = cache.misses;
    result.hits = cache.hits;
    result.primitiveGateCount = cache.primitiveGateCount;


    //Mache Speicher von Arrays frei
    delete[] cache.dataStorage1;
    delete[] cache.tagStorage1;
    delete[] cache.dataStorage2;
    delete[] cache.tagStorage2;
    delete[] cache.tagStorageMem;
    delete[] cache.dataStorageMem;

    //Gebe Result struct zurück
    return result;


}

int sc_main(int argc, char** argv){
    return 1;
}