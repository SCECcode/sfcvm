# Autoconf/automake file

bin_PROGRAMS = geomodelgrids_query geomodelgrids_example

AM_CFLAGS = -DDYNAMIC_LIBRARY ${CFLAGS} ${OPENSSL_INCLUDES} ${PROJ_INCLUDES} ${HDF5_INCLUDES} ${GEOMODELGRIDS_INCLUDES} -I../src
AM_LDFLAGS = ${LDFLAGS} ${OPENSSL_LDFLAGS} ${PROJ_LDFLAGS} ${HDF5_LDFLAGS} ${GEOMODELGRIDS_LDFLAGS} -L../src -lsfcvm -lm -ldl
AM_CPPFLAGS = -I../src -I$(prefix)/include

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
