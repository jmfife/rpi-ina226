CC=gcc
LD=gcc
CFLAGS=-g3 -Wall
LDFLAGS=-lm
ALL = ina226_emulate test

ifeq ($(OS),Windows_NT)
    CCFLAGS += -D WIN32
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        CCFLAGS += -D AMD64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            CCFLAGS += -D AMD64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            CCFLAGS += -D IA32
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CCFLAGS += -D LINUX
		ALL += ina226_monitor
    endif
    ifeq ($(UNAME_S),Darwin)
        CCFLAGS += -D OSX
		LDFLAGS += -largp -L /usr/local/opt/argp-standalone/lib
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        CCFLAGS += -D AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        CCFLAGS += -D IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        CCFLAGS += -D ARM
    endif
endif

.PHONY: clean

all: $(ALL)

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





