# Autoconf/automake file

bin_PROGRAMS = cvmhlabn_api_validate cvmhlabn_vxlite_validate

# General compiler/linker flags
AM_CFLAGS = -DDYNAMIC_LIBRARY -Wall -O3 -std=c99 -D_LARGEFILE_SOURCE \
        -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -I../src
AM_LDFLAGS = -L../src -lcvmhlabn -L../gctpc/source -lgctpc -lm -ldl

# Dist sources
cvmhlabn_api_validate_SOURCES = cvmhlabn_api_validate.c
cvmhlabn_vxlite_validate_SOURCES = cvmhlabn_vxlite_validate.c

.PHONY = run_validate run_validate_with_ucvm

all: $(bin_PROGRAMS)

############################################
# Executables
############################################

cvmhlabn_vxlite_validate : cvmhlabn_vxlite_validate.o 
	$(CC) -o $@ $^ $(AM_LDFLAGS)

cvmhlabn_api_validate : cvmhlabn_api_validate.o 
	$(CC) -o $@ $^ $(AM_LDFLAGS)

### only run after ucvm is built and installed
if UCVM

UCVM_CFLAGS = -I$(UCVM_INSTALL_PATH)/include
UCVM_LDFLAGS = -L$(UCVM_INSTALL_PATH)/lib -lucvm -dynamic  -L$(UCVM_INSTALL_PATH)/lib/euclid3/lib -letree -L$(UCVM_INSTALL_PATH)/lib/proj-5/lib -lproj -lpthread -L$(UCVM_INSTALL_PATH)/lib/fftw/lib -lfftw3 -lm -ldl

CVMH_LDFLAGS = -L$(UCVM_INSTALL_PATH)/model/cvmh/lib -lvxapi -L$(UCVM_INSTALL_PATH)/model/cvmh/lib -lgeo
CENCAL_LDFLAGS = -L$(UCVM_INSTALL_PATH)/model/cencal/lib -lcencalvm -lstdc++

CVMH_EXISTS := $(shell [[ -e $(UCVM_INSTALL_PATH)/model/cvmh/include/vx_sub.h ]] && echo 1 || echo 0 )
CENCAL_EXISTS := $(shell [[ -e $(UCVM_INSTALL_PATH)/model/cencal/include/cencalvm/query/cvmerror.h ]] && echo 1 || echo 0 )

cvmhlabn_ucvm_validate.o : cvmhlabn_ucvm_validate.c
	$(CC) -c -o $@ $^ $(UCVM_CFLAGS) $(AM_CFLAGS) $(CFLAGS)

cvmhlabn_ucvm_validate : cvmhlabn_ucvm_validate.o 
	$(CC) -o $@ $^ $(UCVM_LDFLAGS) $(AM_LDFLAGS) $(LDFLAGS)

cvmhlabn_ucvm_rerun.o : cvmhlabn_ucvm_rerun.c
	$(CC) -c -o $@ $^ $(UCVM_CFLAGS) $(AM_CFLAGS) $(CFLAGS)

cvmhlabn_ucvm_rerun : cvmhlabn_ucvm_rerun.o 
	$(CC) -o $@ $^ $(UCVM_LDFLAGS) $(AM_LDFLAGS) $(LDFLAGS)

cvmhlabn_ucvm_retry.o : cvmhlabn_ucvm_retry.c
	$(CC) -c -o $@ $^ $(UCVM_CFLAGS) $(AM_CFLAGS) $(CFLAGS)

cvmhlabn_ucvm_retry : cvmhlabn_ucvm_retry.o 
	$(CC) -o $@ $^ $(UCVM_LDFLAGS) $(AM_LDFLAGS) $(LDFLAGS)

endif

############################################
# Special build targets
############################################

run_validate: cvmhlabn_api_validate cvmhlabn_vxlite_validate
	./run_vxlite_validate.sh
	./run_api_validate.sh

run_validate_with_ucvm: cvmhlabn_ucvm_validate cvmhlabn_ucvm_rerun cvmhlabn_ucvm_retry
	./run_ucvm_validate.sh

clean:
	rm -rf *~ *.o $(bin_PROGRAMS) cvmhlabn_ucvm_validate cvmhlabn_ucvm_rerun cvmhlabn_ucvm_retry *.txt


