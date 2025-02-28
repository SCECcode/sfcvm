# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS}
AM_FCFLAGS = ${FCFLAGS}
AM_LDFLAGS = ${LDFLAGS}

all:
if WITH_SFCVM_LARGEDATA_DIR
	echo "USING WITH_SFCVM_LARGEDATA_DIR in sfcvm"
else
	./make_data_files.py
endif

install:
if WITH_SFCVM_LARGEDATA_DIR
	if [ ! -d "${prefix}/data/sfcvm" ]; then \
		$(MKDIR_P) ${prefix}/data; \
		$(LN_S) -f $(SFCVM_LARGEDATA_DIR) ${prefix}/data/sfcvm; \
		cp config ${prefix}/data; \
		echo "USING WITH_SFCVM_LARGEDATA_DIR in sfcvm"; \
	fi
else
	if [ ! -d "${prefix}/data/sfcvm" ]; then \
		$(MKDIR_P) ${prefix}/data; \
		mv sfcvm ${prefix}/data; \
		$(LN_S) -f ${prefix}/data/sfcvm .; \
	fi
endif
	cp config ${prefix}/data

dist-clean :
	rm -rf sfcvm
