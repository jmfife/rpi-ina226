.PHONY: clean

all:	ina226 test_accumavg

ina226.o: ina226.c ina226.h 
	g++ -c -o ina226.o ina226.c 

ina226-emulate.o: ina226-emulate.c
	gcc -c -o ina226-emulate.o ina226-emulate.c 

ina226: ina226.o AccumAvg.o 
	g++ -o ina226 ina226.o AccumAvg.o -lwiringPi -lm

ina226-emulate: ina226-emulate.o AccumAvg.o 
	gcc -o ina226-emulate ina226-emulate.o AccumAvg.o -lm -largp -L /usr/local/opt/argp-standalone/lib 

AccumAvg.o: AccumAvg.cpp AccumAvg.h
	g++ -c -o AccumAvg.o AccumAvg.cpp

test_accumavg.o: test_accumavg.cpp AccumAvg.h
	g++ -c -o test_accumavg.o test_accumavg.cpp

test_accumavg: test_accumavg.o AccumAvg.o
	g++ -o test_accumavg test_accumavg.o AccumAvg.o

xclean:
	rm -rf ina226
