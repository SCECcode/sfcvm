# Autoconf/automake file

bin_PROGRAMS = geomodelgrids_query geomodelgrids_example

AM_CFLAGS = -g -DDYNAMIC_LIBRARY -Wall -std=c99
AM_LDFLAGS = -lm -ldl -L$(prefix)/lib -lgeomodelgrids
AM_CPPFLAGS = -I$(prefix)/include

TARGETS = $(bin_PROGRAMS)

all: $(bin_PROGRAMS)

############################################
# Executables
############################################

geomodelgrids_query : geomodelgrids_query.o
	$(CC) -g -o $@ $^ $(AM_LDFLAGS)

clean:
	rm -rf *~ *.o *.out *.log $(bin_PROGRAMS) 

install:
	mkdir -p ${prefix}/test_geomodelgrids
	cp geomodelgrids_query  ${prefix}/test_geomodelgrids
	cp geomodelgrids_example  ${prefix}/test_geomodelgrids
