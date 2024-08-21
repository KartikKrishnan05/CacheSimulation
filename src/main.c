#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdint.h>

// struct für das Endregebnis
struct Result {
    size_t cycles;
    size_t misses;
    size_t hits;
    size_t primitiveGateCount;
};

// Einzelne Anfrage von der Eingabedatei
struct Request {
    int we;
    uint32_t addr;
    uint32_t data;
};


// Alle Anfragen gespeichert in einem Array & und gesamt Anzahl an Anfragen  
struct Requests {
    size_t count;
    struct Request* requests;
};

// Externe deklaration der Method run_simulation
extern struct Result run_simulation(int cycles, unsigned l1CacheLines, unsigned l2CacheLines, unsigned cacheLineSize,
                                 unsigned l1CacheLatency, unsigned l2CacheLatency, unsigned memoryLatency,
                                 size_t numRequests, struct Request requests[], const char* tracefile);


int main(int argc, char *argv[]) {
    printf("\nANFANG UNSERER SIMULATION: \n");

    //Deklaration der Veriabeln 
    int cycles; 
    unsigned cacheline_size; 
    unsigned l1_lines; 
    unsigned l2_lines;
    unsigned l1_latency;
    unsigned l2_latency;
    unsigned memory_latency;
    
    const char *tracefile = NULL;
    const char *inputfile = NULL;

    int option_index = 0;
    int caseValue;

    // struct option mit allen Eingabeoptionen 
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"cacheline-size", required_argument, 0, 0},
        {"l1-lines", required_argument, 0, 0},
        {"l2-lines", required_argument, 0, 0},
        {"l1-latency", required_argument, 0, 0},
        {"l2-latency", required_argument, 0, 0},
        {"memory-latency", required_argument, 0, 0},
        {"tf", optional_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

// Zuweisung von der Eingabe zu den Variabeln bzw. Ausgabe des Help Text
while ((caseValue = getopt_long(argc, argv, "c:h", long_options, &option_index)) != -1) {
        switch (caseValue) {
            case 'h':
                printf("Benutzung: %s [optionen] <EingabeDatei>\n", argv[0]);
                printf("Optionen:\n");
                printf("  -c <Zahl>/--cycles <Zahl>: Anzahl der Zyklen, die simuliert werden sollen\n");
                printf("  --cacheline-size <Zahl>: Größe einer Cachezeile in Byte\n");
                printf("  --l1-lines <Zahl>: Anzahl der Cachezeilen des L1 Caches\n");
                printf("  --l2-lines <Zahl>: Anzahl der Cachezeilen des L2 Caches\n");
                printf("  --l1-latency <Zahl>: Latenzzeit des L1 Caches in Zyklen\n");
                printf("  --l2-latency <Zahl>: Latenzzeit des L2 Caches in Zyklen\n");
                printf("  --memory-latency <Zahl>: Latenzzeit des Hauptspeichers in Zyklen\n");
                printf("  --tf=<Dateiname>: Ausgabedatei für ein Tracefile mit allen Signalen\n");
                printf("  <Dateiname>: Eingabedatei, die die zu verarbeitenden Daten enthält\n");
                exit(EXIT_SUCCESS);
            case 'c':
                cycles = atoi(optarg);
                break;
            case 0:
                if (strcmp(long_options[option_index].name, "cacheline-size") == 0)
                    cacheline_size = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "cycles") == 0)
                    cycles = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "l1-lines") == 0)
                    l1_lines = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "l2-lines") == 0)
                    l2_lines = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "l1-latency") == 0)
                    l1_latency = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "l2-latency") == 0)
                    l2_latency = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "memory-latency") == 0)
                    memory_latency = atoi(optarg);
                else if (strcmp(long_options[option_index].name, "tf") == 0)
                    tracefile = optarg;
                break;
            default:
                fprintf(stderr, "Die Option gibt es nicht. Wende dich an './main -h' \n");
                exit(EXIT_FAILURE);
        }
    }

    // Zuweisung des non-option Arguments für die Eingabedatei und falls nicht vorhanden Fehlermeldung ausgeben
    if (optind < argc) {
        inputfile = argv[optind];
    } else {
        fprintf(stderr, "Input Datei ist gebraucht\n");
        exit(EXIT_FAILURE);
    }

    // Eingelesenen Daten ausgeben um Korrektheit zu überprüfen 
    printf("\nDie ausgewählten Simulationsparameter sind:\n\n");
    printf("Cycles: %d\n", cycles);
    printf("Cacheline size: %d bytes\n", cacheline_size);
    printf("L1 lines: %d\n", l1_lines);
    printf("L2 lines: %d\n", l2_lines);
    printf("L1 latency: %d cycles\n", l1_latency);
    printf("L2 latency: %d cycles\n", l2_latency);
    printf("Memory latency: %d cycles\n", memory_latency);

    // Eingabedatei öffnen und checken ob es geklappt hat 
    FILE *filepointer = fopen(inputfile, "r");
    
    if (filepointer == NULL) {
        fprintf(stderr, "Fehler beim öffnen der Datei\n");
        exit(EXIT_FAILURE);
    }



    // Anzahl der Anfragen zählen und pointer zurücksetzen 
    int Linie = 1;
    char var;

    while ((var = fgetc(filepointer)) != EOF) {
        if (var == '\n') {
            Linie++;
        }
    }

    rewind(filepointer);


    // Speicherplatz im Heap Freischalten und Verfikation 
    struct Requests *anfragen = (struct Requests *)malloc(sizeof(struct Requests));

    if (anfragen == NULL) {
        fprintf(stderr, "Nicht genug Speicher konnte Frei gemacht werden\n");
        fclose(filepointer);
        exit(EXIT_FAILURE);
    }

    anfragen->requests = (struct Request *)malloc((Linie + 1) * sizeof(struct Request)); 

    if (anfragen->requests == NULL) {
        fprintf(stderr, "Nicht genug Speicher konnte Frei gemacht werden\n");
        fclose(filepointer);
        free(anfragen);
        exit(EXIT_FAILURE);
    }

    anfragen->count = Linie;

    int line_number = 0;
    // While Loop bis alle Linien durchgegangen sind 
    while (line_number < Linie) {
        char operation;
        uint32_t address;
        uint32_t value;
        line_number++;

        // Beendigung falls keine Operation erkannt wird
        if(fscanf(filepointer, " %c", &operation) == 0){
            fprintf(stderr, "Leere Zeile oder keine Operation erkannt");
            fclose(filepointer);
            free(anfragen);
            exit(EXIT_FAILURE);
        }
        // Unterschiedung zwischen 'R' und 'W' und sonstige 
        if(operation == 'R'){
            printf("R: %c\n", operation);
            char a[35];
            // Überprüfung ob Adresse angegeben wurde
            if (fscanf(filepointer, ",%34[^,\n],", a) != 1) {
                fprintf(stderr,"Es fehlt eine Adresse");
                fclose(filepointer);
                free(anfragen);
                exit(EXIT_FAILURE);
            }

            // Umwandlnug je nachdem ob die Adresse als Hex oder Decimal angegeben wurde zu unsigned long integer
            if (a[1] == 'x') {
                address = strtoul(a, NULL, 16);
            } else {
                address = strtoul(a, NULL, 10);
            }
            
            anfragen->requests[line_number - 1].addr = address;
            anfragen->requests[line_number - 1].data = 0; 
            anfragen->requests[line_number - 1].we = 0;
        } else if(operation == 'W'){
            printf("W: %c\n", operation);
            char a[35];
            char b[35];
            // Überprüfung ob Adresse und Wert angegeben worden sind 
            if (fscanf(filepointer, ",%34[^,],%34[^\n]", a, b) != 2) {
                fprintf(stderr,"Es fehlt eine Adresse oder ein Wert");
                fclose(filepointer);
                free(anfragen);
                exit(EXIT_FAILURE);
            }


            // Umwandlnug je nachdem ob die Eingabe als Hex oder Decimal angegeben wurde zu unsigned long integer
            if (a[1] == 'x') {
                address = strtoul(a, NULL, 16);
            } else {
                address = strtoul(a, NULL, 10);
            }
            if  (b[1] == 'x') {
                value = strtoul(b, NULL, 16);
            } else {
                value = strtoul(b, NULL, 10);
            }

            anfragen->requests[line_number - 1].addr = address;
            anfragen->requests[line_number - 1].data = value; 
            anfragen->requests[line_number - 1].we = 1;
        } else {
            // Fall falls Operation kein 'R' oder 'W' ist  
            printf("Nicht erlaubt: %c\n", operation);
            fprintf(stderr, "Bitte nutze nur 'W' oder 'R' als Operationen");
            fclose(filepointer);
            free(anfragen);
            exit(EXIT_FAILURE);
        }
    }

    fclose(filepointer);


    // Ausgabe um die Eingabe zu überprüfen 
    for (int i = 0; i < anfragen->count; i++) {
        if (anfragen->requests[i].we == 0) {
            printf("Operation: %d, Address: 0x%X\n", anfragen->requests[i].we, anfragen->requests[i].addr);
        } else if (anfragen->requests[i].we == 1) {
            printf("Operation: %d, Address: 0x%X, Value: 0x%X\n", anfragen->requests[i].we, anfragen->requests[i].addr, anfragen->requests[i].data);
        }
    }

    //Aufruf der run_simulation Methode aus simulation.cpp
    struct Result result = run_simulation(cycles, l1_lines, l2_lines, cacheline_size, l1_latency, l2_latency, memory_latency, anfragen->count, anfragen->requests, tracefile);

    // Ausgabe für die Überprüfung der Korrektheit 
    printf("\nCycles: %zx, Hits: %zd, Misses: %zd, Gates: %zd\n", result.cycles, result.hits, result.misses, result.primitiveGateCount);
    
    printf("\nFinished");

    //Speicherplatz wieder freistellen 
    free(anfragen->requests); 
    free(anfragen); 

    return 0;
}
