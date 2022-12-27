CC=gcc
CFLAGS=-g3 -Wall

.PHONY: clean

all:

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

test_accumavg.o: test_accumavg.c AccumAvg.h tester.h

test_accumavg: test_accumavg.o AccumAvg.o tester.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f ina226
	rm -f ina226-emulate
	rm -f test_accumavg
	rm -f *.o

test: test_accumavg
	./test_accumavg 
	@echo OK!

tester.o: tester.c tester.h





