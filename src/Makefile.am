# Autoconf/automake file

lib_LIBRARIES = libvxapi_cvmhlabn.a libcvmhlabn.a 
bin_PROGRAMS = vx_lite_cvmhlabn vx_cvmhlabn
include_HEADERS = vx_sub_cvmhlabn.h cvmhlabn.h
 
# General compiler/linker flags
AM_CFLAGS = -Wall -O3 -std=c99 -D_LARGEFILE_SOURCE \
            -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -fPIC
AM_LDFLAGS = -L../gctpc/source -lgctpc -lm

# Dist sources
libcvmhlabn_a_SOURCES = vx_sub_cvmhlabn.c vx_io.c 
vx_lite_cvmhlabn_SOURCES = vx_lite_cvmhlabn.c
vx_cvmhlabn_SOURCES = cvmhlabn.c vx_cvmhlabn.c

TARGETS = vx_lite_cvmhlabn vx_cvmhlabn libvxapi_cvmhlabn.a libcvmhlabn.a libcvmhlabn.so

all: $(TARGETS)

install:
	mkdir -p ${prefix}
	mkdir -p ${prefix}/lib
	mkdir -p ${prefix}/include
	cp libcvmhlabn.so ${prefix}/lib
	cp libcvmhlabn.a ${prefix}/lib
	cp cvmhlabn.h ${prefix}/include

libcvmhlabn.a: vx_sub_cvmhlabn.o vx_io.o utils.o cvmhlabn_static.o 
	$(AR) rcs $@ $^

cvmhlabn_static.o: cvmhlabn.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS)

libcvmhlabn.so: vx_sub_cvmhlabn.o vx_io.o utils.o cvmhlabn.o
	$(CC) -shared $(AM_CFLAGS) -o libcvmhlabn.so $^ $(AM_LDFLAGS)

libvxapi_cvmhlabn.a: vx_sub_cvmhlabn.o vx_io.o utils.o *.h
	$(AR) rcs $@ $^

cvmhlabn.o: cvmhlabn.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY -o $@ -c $^ $(AM_CFLAGS)

vx_lite_cvmhlabn.o : vx_lite_cvmhlabn.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS)

vx_lite_cvmhlabn : vx_lite_cvmhlabn.o libvxapi_cvmhlabn.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

vx_cvmhlabn.o : vx_cvmhlabn.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS)

vx_cvmhlabn : vx_cvmhlabn.o libcvmhlabn.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

clean:
	rm -rf $(TARGETS)
	rm -rf *.o 

