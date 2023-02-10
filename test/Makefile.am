# Autoconf/automake file

bin_PROGRAMS = unittest accepttest

# General compiler/linker flags
AM_CFLAGS = -DDYNAMIC_LIBRARY -Wall -O3 -std=c99 -D_LARGEFILE_SOURCE \
        -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -I../src
AM_LDFLAGS = -L../src -lcvmhlabn -L../gctpc/source -lgctpc -lm -ldl

# Dist sources
unittest_SOURCES = *.c *.h
accepttest_SOURCES = *.c *.h

TARGETS = $(bin_PROGRAMS)

.PHONY = run_unit run_accept

all: $(bin_PROGRAMS)

############################################
# Executables
############################################

unittest: unittest.o unittest_defs.o test_helper.o \
	test_vx_lite_cvmhlabn_exec.o test_vx_cvmhlabn_exec.o test_cvmhlabn_exec.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

run_unit : unittest
	./run_unit

accepttest: accepttest.o unittest_defs.o test_helper.o test_grid_exec.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)

run_accept: accepttest
	./run_accept


clean:
	rm -rf *~ *.o *.out $(bin_PROGRAMS) 

install:
	mkdir -p ${prefix}/test
	cp -R inputs ${prefix}/test
	cp -R ref ${prefix}/ref
