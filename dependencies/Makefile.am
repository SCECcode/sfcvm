# GNU Automake config

GEOMODELGRIDS_URL=https://g-c662a6.a78b8.36fe.data.globus.org/ucvm/v23_4/libs/geomodelgrids_0000.tar.gz
GEOMODELGRIDS_VER=0000

all: installed_geomodelgrids

# ------------------------------------------------------------------------------
# GEOMODELGRIDS
# ------------------------------------------------------------------------------
geomodelgrids:
if INSTALL_GEOMODELGRIDS
GEOMODELGRIDS_TARBALL=geomodelgrids-$(GEOMODELGRIDS_VER).tar.gz

	if [ ! -f "$(GEOMODELGRIDS_TARBALL)" ]; then $(CURL) -L -O $(GEOMODELGRIDS_URL)/$(GEOMODELGRIDS_TARBALL); fi
	$(TAR) -xf $(GEOMODELGRIDS_TARBALL)
	$(MKDIR_P) geomodelgrids-build
	cd geomodelgrids-build && \
		../geomodelgrids-$(GEOMODELGRIDS_VER)/configure \
			--prefix=$(prefix) \
			CC=$(CC) CPPFLAGS="$(CPPFLAGS)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"; \
		make -C libsrc && make install -C libsrc && \
		touch $(abs_builddir)/installed_$@
else
	@echo "Using existing $@ installation."
	touch $(abs_builddir)/installed_$@
endif

installed_geomodelgrids:
	$(MAKE) $(AM_MAKEFLAGS) geomodelgrids

