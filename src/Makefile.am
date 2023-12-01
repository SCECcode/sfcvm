# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} -I$(prefix)/include 
AM_LDFLAGS = ${LDFLAGS} -L$(prefix)/lib -lgeomodelgrids

TARGETS = sfcvm_query libsfcvm.a libsfcvm.so

all: $(TARGETS)

install:
	mkdir -p ${prefix}
	mkdir -p ${prefix}/lib
	mkdir -p ${prefix}/include
	mkdir -p ${prefix}/bin
	cp libsfcvm.so ${prefix}/lib
	cp libsfcvm.a ${prefix}/lib
	cp sfcvm.h ${prefix}/include
	cp sfcvm_query ${prefix}/bin
	cp run_sfcvm_query.sh ${prefix}/bin

libsfcvm.a: sfcvm_static.o cJSON.o 
	$(AR) rcs $@ $^

libsfcvm.so: sfcvm.o cJSON.o 
	$(CC) -shared $(AM_CFLAGS) -o libsfcvm.so $^ $(AM_LDFLAGS)

sfcvm.o: sfcvm.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY $(AM_CFLAGS) -o $@ -c $^ 
	
sfcvm_static.o: sfcvm.c
	$(CC) $(AM_CFLAGS) -o $@ -c $^ 

sfcvm_query.o: sfcvm_query.c 
	$(CC) $(AM_CFLAGS) -o $@ -c $^ 

sfcvm_query : sfcvm_query.o libsfcvm.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

cJSON.o: cJSON.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY $(AM_CFLAGS) -o $@ -c $^ 
clean:
	rm -rf $(TARGETS)
	rm -rf *.o 

