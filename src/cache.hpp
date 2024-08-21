#ifndef CACHE_HPP
#define CACHE_HPP

#include <systemc>

using namespace sc_core;

SC_MODULE(CACHE) {
    int reads = 0;
    int writes = 0;
    unsigned l1CacheLines;
    unsigned l2CacheLines;
    unsigned lineSize;
    size_t numRequests;
    int max_cycles;

    unsigned l1CacheLatency;
    unsigned l2CacheLatency;
    unsigned memoryLatency;

    unsigned memIndex = 0;

    //Clock und input ports für Requests
    sc_in<bool> clk;
    sc_in<int> we;
    sc_in<uint32_t> addr;
    sc_in<uint32_t> data_in;

    //Output ports für Tracefile
    sc_out<bool> hit;
    sc_out<uint32_t> data_out;

    //Ergebnisvariablen
    size_t cycles = 0;
    size_t misses = 0;
    size_t hits = 0;
    size_t primitiveGateCount = 0;

    //Arrays um Daten des L1Cache, L2Cache und Hauptspeicher zu speichern
    uint8_t* dataStorage1;
    uint8_t* tagStorage1;
    uint8_t* dataStorage2;
    uint8_t* tagStorage2;
    uint8_t* tagStorageMem;
    uint8_t* dataStorageMem;


    //Konstruktur mit 3 Methoden und Schätzung von primitiveGateCount
    SC_CTOR(CACHE) {

        SC_METHOD(write);
        sensitive << clk.neg();
        dont_initialize();

        SC_METHOD(read);
        sensitive << clk.neg();
        dont_initialize();

        SC_METHOD(updateCycles);
        sensitive << clk.neg();
        dont_initialize();

        primitiveGateCount += 32+ 4*l2CacheLines;


    }

    //Methode um Anzahl der Indexbits zu bestimmen
    int numIndex(unsigned numLines) {
        int val = 0;
        while (numLines != 1) {
            numLines /= 2;
            val++;
        }
        return val;
    }

    //Methode um bei jedem Clock Tick die Anzahl der Zyklen zu inkrementieren
    void updateCycles() {
        //std::cout << "Cycles: " << cycles << std::endl;
        if (cycles >= max_cycles || cycles==SIZE_MAX) {
            cycles = SIZE_MAX;
        } else{
            cycles++;
        }
    }

    //Methode um Zyklen zu inkrementieren, wird genutzt um Latency einzubauen 
    void incrementCycles(unsigned additionalCycles) {
        if (cycles + additionalCycles > max_cycles || cycles==SIZE_MAX) {
            cycles = SIZE_MAX;
        } else {
            cycles += additionalCycles;
        }
    }

    
    //Methode um Daten in Caches und Hauptspeicher zu schreiben
    void write() {
        //std::cout << sc_time_stamp() << ", value (Write) = " << clk->read() << std::endl;
        //std::cout << "L1: " << l1CacheLatency << " L2: " << l2CacheLatency << " Mem: " << memoryLatency <<std::endl;

        //Methode wird nur bei we==1 ausgeführt, sonst return (s. else)
        if (we.read() == 1) {
            //Inkrementiere Read-Counter, wird für Schaltkreisanalyse genutzt
            reads++;
            //std::cout << "Cycles(in write): " << cycles << std::endl;

            //Lese Adresse und Daten die geschrieben werden sollen
            uint32_t address = addr.read();
            uint32_t data = data_in.read();
            reads += 2;

            //Berechne Anzahl der Indexbits für L1 und L2
            int l1IndexBits = numIndex(l1CacheLines);
            int l2IndexBits = numIndex(l2CacheLines);

            

            //std::cout << "addr = " << std::bitset<32>(address)  << std::endl;

            //Berchne Index- und Tagbits für L1
            uint32_t index = (address % l1CacheLines)*4;
            uint32_t tag = address >> l1IndexBits;

            //Berchne Index- und Tagbits für L2
            uint32_t index2 = (address % l2CacheLines)*4; 
            uint32_t tag2 = address >> l2IndexBits;

            if (index < l1CacheLines*4 && index2 < l2CacheLines*4) {
                //Schreibe Daten und Tags in L1 und L2
                tagStorage1[index] = (tag >> 24) & 0xFF;
                tagStorage1[index + 1] = (tag >> 16) & 0xFF;
                tagStorage1[index + 2] = (tag >> 8) & 0xFF;
                tagStorage1[index + 3] = tag & 0xFF;

                dataStorage1[index] = (data >> 24) & 0xFF;
                dataStorage1[index+1] = (data >> 16) & 0xFF;
                dataStorage1[index+2] = (data >> 8) & 0xFF;
                dataStorage1[index+3] = data & 0xFF;
                
                tagStorage2[index2] = (tag2 >> 24) & 0xFF;
                tagStorage2[index2 + 1] = (tag2 >> 16) & 0xFF;
                tagStorage2[index2 + 2] = (tag2 >> 8) & 0xFF;
                tagStorage2[index2 + 3] = tag2 & 0xFF;
                
                dataStorage2[index2] = (data >> 24) & 0xFF;
                dataStorage2[index2+1] = (data >> 16) & 0xFF;
                dataStorage2[index2+2] = (data >> 8) & 0xFF;
                dataStorage2[index2+3] = data & 0xFF;

                //Suche nach Datum im Hauptspeicher
                bool found = false;
                for (int i = 0; i < numRequests*4; i+=4) {
                    uint32_t memtag = (u_int32_t(tagStorageMem[i]) << 24) | (u_int32_t(tagStorageMem[i+1]) << 16) | (u_int32_t(tagStorageMem[i+2]) << 8) | (u_int32_t(tagStorageMem[i+3]));
                    if (memtag == address) {
                        dataStorageMem[i] = (data >> 24) & 0xFF;
                        dataStorageMem[i+1] = (data >> 16) & 0xFF;
                        dataStorageMem[i+2] = (data >> 8) & 0xFF;
                        dataStorageMem[i+3] = data & 0xFF;
                        found = true;
                        break;
                    }
                }

                //Falls das Datum im Hauptspeicher nicht vorhanden, schreibe es rein
                if (!found && memIndex < numRequests*4) {
                    dataStorageMem[memIndex] = (data >> 24) & 0xFF;
                    dataStorageMem[memIndex+1] = (data >> 16) & 0xFF;
                    dataStorageMem[memIndex+2] = (data >> 8) & 0xFF;
                    dataStorageMem[memIndex+3] = data & 0xFF;
                    //tagStorageMem[memIndex] = address;
                    tagStorage2[memIndex] = (address >> 24) & 0xFF;
                    tagStorage2[memIndex + 1] = (address >> 16) & 0xFF;
                    tagStorage2[memIndex + 2] = (address >> 8) & 0xFF;
                    tagStorage2[memIndex + 3] = address & 0xFF;
                    memIndex+=4;
                }

                //Erhöhe anzahl der Zyklen und Latency von L1, L2 und Hauptspeicher
                incrementCycles(l1CacheLatency + l2CacheLatency + memoryLatency);
                return;
            } else {
                return;
            }
        }
    }

    //Methode um Daten aus den caches bzw Hauptspeicher zu lesen
    void read() {
        //Methode wird nur bei we==0 ausgeführt, sonst return (s. else)
        if (we.read() == 0) {
            reads++;
            //std::cout << "Cycles(in read): " << cycles << std::endl;
            //std::cout << sc_time_stamp() << ", read = " << clk->read() << std::endl;

            //Lese Adresse wo das geforderte Datum steht
            uint32_t address = addr.read();
            reads++;

            //Berchne Anzahl der Indexbits für L1 und L2
            int l1IndexBits = numIndex(l1CacheLines);
            int l2IndexBits = numIndex(l2CacheLines);

            //Berchne Index- und Tagbits für L1
            uint32_t index = (address % l1CacheLines)*4;
            uint32_t tag = address >> l1IndexBits;

            uint32_t data = 0;
            //Falls Daten im L1 sind
            uint32_t l1tag = (uint32_t(tagStorage1[index]) << 24) | (uint32_t(tagStorage1[index+1]) << 16) | (uint32_t(tagStorage1[index+2]) << 8) | (uint32_t(tagStorage1[index+3]));
            if (l1tag == tag) {
                std::cout << "read from l1" << std::endl;
                //Erhöhe Anzahl der Hits und Write-Counter
                hits++;
                hit.write(true);
                writes++;
                //Lese Daten
                data = (uint32_t(dataStorage1[index]) << 24) | (uint32_t(dataStorage1[index+1]) << 16) | (uint32_t(dataStorage1[index+2]) << 8) | (uint32_t(dataStorage1[index+3]));
                data_out.write(data);
                writes++;
                //Erhöhe Anzahl der Zyklen um L1-Latency
                incrementCycles(l1CacheLatency);

            //Falls Daten nicht im L1
            } else {
                //Berechne Index- und Tagbits von L2
                uint32_t index2 = (address % l2CacheLines)*4; 
                uint32_t tag2 = address >> l2IndexBits;
                //Falls Daten im L2
                uint32_t l2tag = (uint32_t(tagStorage2[index2]) << 24) | (uint32_t(tagStorage2[index2+1]) << 16) | (uint32_t(tagStorage2[index2+2]) << 8) | (uint32_t(tagStorage2[index2+3]));
                if (l2tag == tag2) {
                    std::cout << "read from l2" << std::endl;
                    //Erhöhe Anzahl der hits und misses um 1 
                    misses++;
                    hits++;
                    //Lese Daten 
                    tagStorage1[index] = (tag >> 24) & 0xFF;
                    tagStorage1[index + 1] = (tag >> 16) & 0xFF;
                    tagStorage1[index + 2] = (tag >> 8) & 0xFF;
                    tagStorage1[index + 3] = tag & 0xFF;
                    dataStorage1[index] = dataStorage2[index2];
                    dataStorage1[index+1] = dataStorage2[index2+1];
                    dataStorage1[index+2] = dataStorage2[index2+2];
                    dataStorage1[index+3] = dataStorage2[index2+3];
                    data = (uint32_t(dataStorage1[index]) << 24) | (uint32_t(dataStorage1[index+1]) << 16) | (uint32_t(dataStorage1[index+2]) << 8) | (uint32_t(dataStorage1[index+3]));
                    data_out.write(data);
                    hit.write(true);
                    writes += 2;
                    //Erhöhe Anzahl der Zyklen um L1-Latency und L2-Latency
                    incrementCycles(l1CacheLatency + l2CacheLatency);
                //Falls Daten weder in L1 noch L2, hole sie aus dem Hauptspeicher
                } else {
                    std::cout << "read from MainMem" << std::endl;
                    //Erhöhe Anzahl der misses
                    misses++;
                    hit.write(false);
                    writes++;
                    //Finde Daten im Hauptspeicher
                    for (int i = 0; i < numRequests*4; i+=4) {
                        uint32_t memtag = (uint32_t(tagStorageMem[i]) << 24) | (uint32_t(tagStorageMem[i+1]) << 16) | (uint32_t(tagStorageMem[i+2]) << 8) | (uint32_t(tagStorageMem[i+3]));
                        if (memtag == address) {
                            data = (uint32_t(dataStorageMem[i]) << 24) | (uint32_t(dataStorageMem[i+1]) << 16) | (uint32_t(dataStorageMem[i+2]) << 8) | (uint32_t(dataStorageMem[i+3]));
                            break;
                        }
                    }
                    //Lade das Datum auch in L1 und L2 Cache (s.lru)
                    tagStorage1[index] = (tag >> 24) & 0xFF;
                    tagStorage1[index + 1] = (tag >> 16) & 0xFF;
                    tagStorage1[index + 2] = (tag >> 8) & 0xFF;
                    tagStorage1[index + 3] = tag & 0xFF;
                    tagStorage2[index2] = (tag2 >> 24) & 0xFF;
                    tagStorage2[index2 + 1] = (tag2 >> 16) & 0xFF;
                    tagStorage2[index2 + 2] = (tag2 >> 8) & 0xFF;
                    tagStorage2[index2 + 3] = tag2 & 0xFF;
                    dataStorage1[index] = (data >> 24) & 0xFF;
                    dataStorage1[index+1] = (data >> 16) & 0xFF;
                    dataStorage1[index+2] = (data >> 8) & 0xFF;
                    dataStorage1[index+3] = data & 0xFF;
                    dataStorage2[index2] = (data >> 24) & 0xFF;
                    dataStorage2[index2+1] = (data >> 16) & 0xFF;
                    dataStorage2[index2+2] = (data >> 8) & 0xFF;
                    dataStorage2[index2+3] = data & 0xFF;
                    data_out.write(data);
                    writes++;
                    //Erhöhe Anzahl der Zyklen
                    incrementCycles(l1CacheLatency + l2CacheLatency + memoryLatency);
                }
                }
                return;

        }
    }
};

#endif // CACHE_HPP
