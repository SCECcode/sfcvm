# Autoconf/automake file

bin_PROGRAMS = geomodelgrids_query 

AM_CFLAGS = -DDYNAMIC_LIBRARY -Wall -std=c99
AM_LDFLAGS = -lm -ldl
AM_CPPFLAGS = -I$(prefix)/include

query_SOURCES = geomodelgrids_query.c
query_LDFLAGS = -L$(prefix)/lib -lgeomodelgrids


run_SOURCES = run_geomodelgrids.c

TARGETS = $(bin_PROGRAMS)

all: $(bin_PROGRAMS)

############################################
# Executables
############################################

query_geomodelgrids : $(query_SOURCES)
	$(CC) -o $@ $^ $(query_LDFLAGS)


clean:
	rm -rf *~ *.o *.out *.log $(bin_PROGRAMS) 

install:
	mkdir -p ${prefix}/test_geomodelgrids
	cp geomodelgrids_query  ${prefix}/test_geomodelgrids
