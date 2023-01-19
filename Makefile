CC = gcc
LD = gcc
CFLAGS = -g3 -Wall
LDFLAGS = -lm
ALL = ina226_emulate test

# Determine platform and set variables accordingly
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
		LDFLAGS += -L /usr/local/opt/argp-standalone/lib
		LDLIBS += -largp
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

ina226_monitor: ina226_monitor.o ina226.o i2c.o accum_mean.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ina226_emulate: ina226_emulate.o accum_mean.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

ina226_monitor.o: ina226_monitor.c ina226.h accum_mean.h

ina226_emulate.o: ina226_emulate.c accum_mean.h

accum_avg.o: accum_avg.c accum_avg.h

accum_mean.o: accum_mean.c accum_mean.h

ina226.o: ina226.c ina226.h i2c.h

i2c.o: i2c.c i2c.h

test_accum_avg.o: test_accum_avg.c accum_avg.h tester.h

test_accum_avg: test_accum_avg.o accum_avg.o tester.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

test_accum_mean.o: test_accum_mean.c accum_mean.h tester.h

test_accum_mean: test_accum_mean.o accum_mean.o tester.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f ina226_monitor
	rm -f ina226_emulate
	rm -f test_accum_avg
	rm -f test_accum_mean
	rm -f *.o

test: test_accum_avg test_accum_mean
	./test_accum_avg
	./test_accum_mean
	@echo OK!

tester.o: tester.c tester.h





