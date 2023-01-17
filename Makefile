CC=gcc
LD=gcc
CFLAGS=-g3 -Wall
LDFLAGS=-lm

ifeq ($(detected_OS),Darwin)        # Mac OS X
    CFLAGS += -D OSX
	LDFLAGS += -largp -L /usr/local/opt/argp-standalone/lib
	EXECUTABLES=ina226_emulate test
endif
ifeq ($(detected_OS),Linux)			# Linux
    CFLAGS += -D LINUX
	EXECUTABLES=ina226_emulate test
endif

.PHONY: clean

all: ina226_monitor ina226_emulate test
	echo detected_OS: $(detected_OS)

ina226_monitor: ina226_monitor.o ina226.o i2c.o AccumAvg.o 
	$(CC) -o ina226_monitor ina226_monitor.o ina226.o i2c.o AccumAvg.o $(LDFLAGS)
	
ina226_emulate: ina226_emulate.o AccumAvg.o 
	$(CC) -o ina226_emulate ina226_emulate.o AccumAvg.o $(LDFLAGS)

Aina226_monitor.o: ina226_monitor.c ina226.h 
	gcc -c -o ina226.o ina226.c 

ina226_emulate.o: ina226_emulate.c
	gcc -c -o ina226_emulate.o ina226_emulate.c 

ccumAvg.o: AccumAvg.c AccumAvg.h
	gcc -c -o AccumAvg.o AccumAvg.c

ina226.o: ina226.c ina226.h i2c.h
	gcc -c -o ina226.o ina226.c

test_accumavg.o: test_accumavg.c AccumAvg.h tester.h

test_accumavg: test_accumavg.o AccumAvg.o tester.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f ina226_monitor
	rm -f ina226_emulate
	rm -f test_accumavg
	rm -f *.o

test: test_accumavg
	./test_accumavg 
	@echo OK!

tester.o: tester.c tester.h





