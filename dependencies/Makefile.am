
GEOMODELGRIDS_URL=https://g-c662a6.a78b8.36fe.data.globus.org/ucvm/v24_4/lib/geomodelgrids.tar.gz

all: installed_geomodelgrids

# ------------------------------------------------------------------------------
# GEOMODELGRIDS
# ------------------------------------------------------------------------------
geomodelgrids-build:
if INSTALL_GEOMODELGRIDS
GEOMODELGRIDS_TARBALL=${srcdir}/geomodelgrids.tar.gz
GEOMODELGRIDS_SRC=${srcdir}/geomodelgrids
	if [ ! -d "$(GEOMODELGRIDS_SRC)" ]; then \
		if [ ! -f "$(GEOMODELGRIDS_TARBALL)" ]; then \
			$(CURL) -L -O $(GEOMODELGRIDS_URL); \
			$(TAR) -xf $(GEOMODELGRIDS_TARBALL); \
 		fi \
	; fi
	$(MKDIR_P) geomodelgrids-build
	cd ${abs_builddir}/geomodelgrids && \
		libtoolize && \
		aclocal -I m4 && \
		autoconf && \
		automake --add-missing -f
	cd ${abs_builddir}/geomodelgrids-build && \
		${abs_builddir}/geomodelgrids/configure \
			--prefix=$(prefix) \
			--enable-python \
			--with-hdf5-incdir=$(UCVM_HDF5_INC) --with-hdf5-libdir=$(UCVM_HDF5_LIB) \
			--with-proj-incdir=$(UCVM_PROJ_INC) --with-proj-libdir=$(UCVM_PROJ_LIB) \
			CC=$(CC) CXX=$(CXX) CPPFLAGS="$(CPPFLAGS)" CFLAGS="$(CFLAGS)" CXXFLAGS="\\"-Wall -std=c++14\\"" LDFLAGS="$(LDFLAGS)"; \
		make -C libsrc && make install -C libsrc && \
		touch $(abs_builddir)/installed_$@
else
	@echo "Using existing $@ installation."
	touch $(abs_builddir)/installed_$@
endif

installed_geomodelgrids:
	$(MAKE) $(AM_MAKEFLAGS) geomodelgrids-build

clean: 
	rm -rf installed_geomodelgrids geomodelgrids-build
