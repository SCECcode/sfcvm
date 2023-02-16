# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS}
AM_LDFLAGS = ${LDFLAGS}

TARGETS = libsfcvm.a libsfcvm.so

all: $(TARGETS)

install:
	mkdir -p ${prefix}
	mkdir -p ${prefix}/lib
	mkdir -p ${prefix}/include
	cp libsfcvm.so ${prefix}/lib
	cp libsfcvm.a ${prefix}/lib
	cp sfcvm.h ${prefix}/include

libsfcvm.a: sfcvm_static.o 
	$(AR) rcs $@ $^

libsfcvm.so: sfcvm.o
	$(CC) -shared $(AM_CFLAGS) -o libsfcvm.so $^ $(AM_LDFLAGS)

sfcvm.o: sfcvm.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY -o $@ -c $^ $(AM_CFLAGS)
	
sfcvm_static.o: sfcvm.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS)
	
clean:
	rm -rf $(TARGETS)
	rm -rf *.o 

