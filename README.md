# CacheSimulation

A cycle-accurate simulation of a **two-level CPU cache (L1 + L2)** in front of main memory, written in **SystemC/C++** with a C front end. Built as a team project for the computer architecture lab (GRA) at TUM, 2024.

You give it a trace of memory reads and writes plus the cache geometry and latencies; it reports total cycles, hits and misses, a gate-count estimate, and optionally writes a VCD trace you can open in GTKWave.

**Example result** (matrix multiplication): 3840 cycles without caches vs. 3342 cycles with a 16-line L1 and 32-line L2 (12 hits, 14 misses).

## Build and run

Requires [SystemC](https://github.com/accellera-official/systemc) installed at `../systemc` (or change `SCPATH` in the `Makefile`).

```bash
make
./main --cycles 50000 --cacheline-size 128 \
       --l1-lines 256 --l2-lines 512 \
       --l1-latency 5 --l2-latency 25 --memory-latency 120 \
       --tf=trace examples/MatrixMultiplikation.csv
```

| Option | Meaning |
| --- | --- |
| `-c`, `--cycles` | Number of cycles to simulate |
| `--cacheline-size` | Cache line size in bytes |
| `--l1-lines`, `--l2-lines` | Number of lines in L1 / L2 |
| `--l1-latency`, `--l2-latency`, `--memory-latency` | Access latency in cycles |
| `--tf=<file>` | Write a VCD trace of all signals |

Input files are CSV with one access per line, `W,<address>,<value>` or `R,<address>` (decimal or hex). Examples for L1 hits, L2 hits, main-memory hits and a matrix multiplication are in [`examples/`](examples), with their traces in `examples/Tracefiles/`. Slides and the research summary are in [`slides/`](slides), the final report is [`UPDATED_GRA_13.pdf`](UPDATED_GRA_13.pdf).

## Team

Emirhan Afsin, Kartik Krishnan, Nirbheek Setia

---

## Projektbericht (Deutsch)

Inhalt:
Projekt
Erklärung der Aufgabenstellung
Zusammenfassung der Erkenntnisse der Literatur Recherche
Implementierungen
Methodik, Messumgebung der Analysemethoden
Ergebnisse: Zusammenfassung der Ergebnisse
Persönlicher Anteil der Mitglieder

### Einleitung: 
Das ziel des Projekts liegt in der Betrachtung von mehrstufigen Caches. Die effiziente Zusammenarbeit zwischen Cache und Prozessoren spielt eine immer größere Rolle. Speicherzugriffe dauern heutzutage viel zu lange und die Lösung sind halt Caches mit geringerer Latenz. Unser Projekt simuliert genau dies mit zwei Caches.

### Zusammenfassung der Erkenntnisse der Literaturrecherche: 
Der cache arbeitet mit dem Lokalitätsprinzip (zeitlich/physisch) um dide Zeit, die Daten aus dem Hauptspeicher zu laden, zu minimieren. L1 Cache ist der schnellste und kleinste Cache, wohingegen der L2 Cache bisschen größer ist. Im Vergleich zum L3 jedoch werden diese direkt am Prozessor eingebaut und nicht von allen Cores gleichzeitig genutzt. Die Größe hängt meistens von der Rechenarchitektur.


### Implementierungen: 
Das Rahmenprogramm, welches die Eingaben einließt und sie dann speichert und weitergibt, wurde in C geschrieben. Das Rahmenprogramm ließt auch die csv Dateien und gibt diese in Form eines Arrays weiter. Die Methode "run_simulation" wird mit den Parametern aufgerufen und simuliert das Cache verhalten. In der Modellierungs- und Simulationssprache SystemC nutzen wir die Funktionalität von SC_METHODs aus um dann jeweils immer aus dem Cache zu lesen und schreiben. Zum Schluss gibt das C++ Programm dann die Cycles, Hits, Misses und eine Schätzung der Gatter zurück an das Rahmenprogramm welches diese Ausgibt.  


### Zusammenfassung der Ergebnisse: 
Unsere Simulation hat gezeigt, dass die Nutzung von einem L1 und L2 Cache die Laufzeit von einem speicherintensiven Algorithmus deutlich verbessert. Als Beispiel haben wir die Matrixmultiplikation genommen. Wir führen eine 2x2 Matrixmultiplikation einmal ohne Nutzung von Zwischenspeichern, einmal mit Nutzung von zwei Zwischenspeichern. 

Ergebnis: 
- **Laufzeit ohne Cache:** 3840 Zyklen 
- **Laufzeit mit Caches (L1: 16 Zeilen, L2: 32 Zeilen):** 3342 Zyklen, 12 hits, 14 misses


### Persönlicher Anteil der Mitglieder:

Emirhan Afsin arbeitete an der Cachesimulation und implementierte die Anfangsstruktur in den Dateien und den gegebenen Code. Er programmierte aktiv die Dateien Simulation.cpp, Simulation.hpp und Cache.hpp, leistete einen großen Teil der Recherchearbeit und wirkte an der Präsentation mit. Zudem arbeitete er am Makefile und kollaborierte eng mit Nirbheek. Emirhan schrieb des Weiteren auch einen großen Bestand der Berichtdateie und die Readme-File mit Nirbheek.

Kartik Krishnan erstellte das Rahmenprogramm und arbeitete an der Verbindung zwischen C und C++ Code. Nachdem die Grundstruktur der Cachesimulation stand, half er bei der Fehlerbehebung und stellte sicher, dass die Simulation die richtigen Werte ausgibt, was er mithilfe von Tracefiles und Printstatements bestätigte. Er war für das Makefile und die Nutzung der build.sh Datei zum Testen auf Artemis verantwortlich. Schließlich erstellte er die Präsentation und fügte in Zusammenarbeit mit anderen alles ein.

Nirbheek Setia verbesserte den bestehenden Code periodisch und behob wiederkehrende Bugs. Er erstellte Methoden für die Cache-Operationen und arbeitete an den Simulationsdateien zusammen mit Emirhan und Kartik. Außerdem wirkte er stark am Projektbericht und an der Abschlusspräsentation mit. Nirbheek arbeitete eng mit Emirhan zusammen.


Quellen der Recherche:
https://hardwaretimes.com/difference-between-l1-l2-and-l3-cache-how-does-cpu-cache-work/
ERA-Vorlesungen (Caches)
https://geekysoumya.com/what-is-cache-memory-of-processor-what-are-l1-l2-l3-cache/
https://ar5iv.labs.arxiv.org/html/2103.14808
https://hardwaretimes.com/difference-between-l1-l2-and-l3-cache-how-does-cpu-cache-work/
https://en.wikipedia.org/wiki/CPU_cache#/media/File:Hwloc.png

Quellen zum Rahmenprogramm: 
Code Quelle: https://opensource.com/article/19/5/how-write-good-c-main-function [02.07.24]
Getopt_long: https://linux.die.net/man/3/getopt_long [11.07.24]
Struct option long_options[]: https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html [11.07.24]
C strcmp: https://www.programiz.com/c-programming/library-function/string.h/strcmp [11.07.24]
Header File: 
https://unstop.com/blog/header-files-in-c [02.07.24] 
https://medium.com/@prem112/header-files-in-c-e306e685c148 [02.07.24]
Struct: 
https://www.geeksforgeeks.org/how-to-initialize-structures-in-c/ [02.07.24]
https://www.programiz.com/c-programming/c-structures [02.07.24]
Eingabedatei einlesen: 
https://www.geeksforgeeks.org/basics-file-handling-c/  [05.07.24]
https://www.tutorialspoint.com/c_standard_library/c_function_feof.htm [05.07.24]
https://stackoverflow.com/questions/32366665/resetting-pointer-to-the-start-of-file  [05.07.24]
Check if Hex or Decimal: 
https://community.particle.io/t/how-to-convert-string-to-uint32-t/36362/3 [15.07.24]
https://stackoverflow.com/questions/5489060/fscanf-in-c-how-to-determine-comma [15.07.24]
https://stackoverflow.com/questions/25728608/i-dont-understand-what-is-n [15.07.24]