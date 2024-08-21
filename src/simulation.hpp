#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <systemc>
#include "cache.hpp"

//Dieses Modul bekommt das Requests Array aus der Aufgabenstellung und generiert die benötigten Signale für cache.hpp


//Deklaration des Request und Result structs für unsere Eingabe bzw Ausgabe
struct Request {
    int we;
    uint32_t addr;
    uint32_t data;
};

struct Requests {
    size_t count;
    Request* requests;
};

struct Result {
    size_t cycles;
    size_t misses;
    size_t hits;
    size_t primitiveGateCount;
};

//Deklaration der run_simulation Methode
extern "C" {
    struct Result run_simulation(int cycles, unsigned l1CacheLines, unsigned l2CacheLines, unsigned cacheLineSize,
                                 unsigned l1CacheLatency, unsigned l2CacheLatency, unsigned memoryLatency,
                                 size_t numRequests, Request requests[], const char* tracefile);
};

//Methode mit read/write für I/O-Counter, Requets-Array und Output-Ports für Signale we, addr und data
SC_MODULE(SIMULATION) {
    int reads = 0;
    int writes = 0;
    size_t numRequests;
    sc_in<bool> clk;
    Request* requests;

    sc_out<int> we;
    sc_out<uint32_t> addr;
    sc_out<uint32_t> data;

    //Führe update bei jedem Clock Tick aus
    SC_CTOR(SIMULATION) {
        SC_METHOD(update);
        sensitive << clk.pos();
        dont_initialize();
    }

    void update() {
        //Statische Variable i für index im Array
        static int i = 0;

        if (i < numRequests) {
            //std::cout << sc_time_stamp() << ", value = " << clk->read() << std::endl;
            std::cout << "Request: " << i + 1 << std::endl;
            std::cout << "we: " << requests[i].we << std::endl;
            std::cout << "addr: " << requests[i].addr << std::endl;
            //Nur für Debugging
            if(requests[i].we == 1){
                std::cout << "data: " << requests[i].data << std::endl;
            }
            //Schreibe Daten von richtiger Stelle im Array in die Signale
            we.write(requests[i].we);
            addr.write(requests[i].addr);
            data.write(requests[i].data);
            writes += 3;
            //Inkrementiere i
            i++;
        } else {
            //Falls alle Requests abgelaufen, stoppe die Simulation
            sc_stop();
        }
    }
};

#endif 
