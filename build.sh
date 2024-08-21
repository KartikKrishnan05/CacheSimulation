make clean
make
./main --cycles 50000 --cacheline-size 128 --l1-lines 256 --l2-lines 512 --l1-latency 5 --l2-latency 25 --memory-latency 120 --tf=MatrixMultiplikationTracefile examples/Eingabedatei.csv