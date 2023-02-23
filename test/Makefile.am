# Autoconf/automake file

bin_PROGRAMS = query unittest 

AM_CFLAGS = -DDYNAMIC_LIBRARY -I../src -Wall -std=c99
AM_LDFLAGS = -L../src -lsfcvm -lm -ldl
AM_CPPFLAGS = -I$(prefix)/include

# Dist sources
#unittest_SOURCES = *.c *.h
accepttest_SOURCES = *.c *.h

query_SOURCES = query.c
query_LDADD = $(prefix)/lib/libgeomodelgrids.la

TARGETS = $(bin_PROGRAMS)

.PHONY = run_unit run_accept

all: $(bin_PROGRAMS)

############################################
# Executables
############################################

unittest: unittest.o unittest_defs.o test_helper.o test_vx_sfcvm_exec.o test_sfcvm_exec.o
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
