.PHONY: clean

all:	ina226 test_accumavg

ina226.o: ina226.c ina226.h 
	gcc -c -o ina226.o ina226.c 

ina226-emulate.o: ina226-emulate.c
	gcc -c -o ina226-emulate.o ina226-emulate.c 

ina226: ina226.o AccumAvg.o 
	gcc -o ina226 ina226.o AccumAvg.o -lwiringPi -lm

ina226-emulate: ina226-emulate.o AccumAvg.o 
	gcc -o ina226-emulate ina226-emulate.o AccumAvg.o -lm -largp -L /usr/local/opt/argp-standalone/lib 

AccumAvg.o: AccumAvg.c AccumAvg.h
	gcc -c -o AccumAvg.o AccumAvg.c

test_accumavg.o: test_accumavg.c AccumAvg.h
	gcc -c -o test_accumavg.o test_accumavg.c

test_accumavg: test_accumavg.o AccumAvg.o
	gcc -o test_accumavg test_accumavg.o AccumAvg.o

xclean:
	rm -rf ina226
