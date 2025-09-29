# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([SFCVM],[1.0],[software@scec.org])
AM_INIT_AUTOMAKE([foreign no-exeext])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_INSTALL

LT_INIT
if test "$allow_undefined_flag" = unsupported; then
    # See issue119.
    AM_LDFLAGS="-no-undefined $AM_LDFLAGS"
fi
AM_CONDITIONAL([NO_UNDEFINED], [test "$allow_undefined_flag" = unsupported])
AC_SUBST(AM_LDFLAGS)


# Checks for libraries.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

if test x"$UCVM_INSTALL_PATH" = x; then
    UCVM_DUMDUM=x"$UCVM_INSTALL_PATH"
    UCVM_DUM=x"$UCVM_SRC_PATH"
fi

if test x"$UCVM_INSTALL_PATH" = x; then
AM_CONDITIONAL(UCVM, false)
UCVM_CFLAGS=
UCVM_LDFLAGS=
UCVM_HDF5_INC=
UCVM_HDF5_LIB=
UCVM_PROJ_INC=
UCVM_PROJ_LIB=
else
AM_CONDITIONAL(UCVM, true)
UCVM_CFLAGS="-I$UCVM_INSTALL_PATH/include"
UCVM_LDFLAGS="-L$UCVM_INSTALL_PATH/lib -lucvm -dynamic -L$UCVM_INSTALL_PATH/lib/proj/lib -lproj -lpthread -L$UCVM_INSTALL_PATH/lib/hdf5/lib -lhdf5 -lm -ldl"
UCVM_HDF5_INC=$UCVM_INSTALL_PATH/lib/hdf5/include
UCVM_HDF5_LIB=$UCVM_INSTALL_PATH/lib/hdf5/lib
UCVM_PROJ_INC=$UCVM_INSTALL_PATH/lib/proj/include
UCVM_PROJ_LIB=$UCVM_INSTALL_PATH/lib/proj/lib
AC_SUBST(UCVM_PROJ_INC)
AC_SUBST(UCVM_PROJ_LIB)
AC_SUBST(UCVM_HDF5_INC)
AC_SUBST(UCVM_HDF5_LIB)
fi

## dependencies
## 
AC_ARG_ENABLE([geomodelgrids],[install_geomodelgrids=yes])
AM_CONDITIONAL([INSTALL_GEOMODELGRIDS], true)
AC_ARG_WITH([geomodelgrids-incdir],
  [AS_HELP_STRING([--with-geomodelgrids-incdir], [directory containing geomodelgrids header files])],
  [with_geomodelgrids_incdir="$withval"],
  [with_geomodelgrids_incdir="$prefix/include"])
AC_SUBST(with_geomodelgrids_incdir)
AC_ARG_WITH([geomodelgrids-libdir],
  [AS_HELP_STRING([--with-geomodelgrids-libdir], [directory containing geomodelgrids library])],
  [with_geomodelgrids_libdir="$withval"],
  [with_geomodelgrids_libdir="$prefix/lib"])

GEOMODELGRIDS_INCLUDES="-I$with_geomodelgrids_incdir"
GEOMODELGRIDS_LDFLAGS="-L$with_geomodelgrids_libdir -lgeomodelgrids"
AC_SUBST(GEOMODELGRIDS_INCLUDES)
AC_SUBST(GEOMODELGRIDS_LDFLAGS)

# TAR
AC_PATH_PROG(TAR, tar)
if test -z "$TAR" ; then
  AC_MSG_FAILURE([cannot find 'tar' program.])
  TAR=`echo "Error: tar is not installed." ; false`
fi

# CURL (used to download model files)
AC_PATH_PROG(CURL, curl)
if test -z "$CURL" ; then
  AC_MSG_FAILURE([cannot find 'curl' program.])
  CURL=`echo "Error: curl is not installed." ; false`
fi

##check optional large data path
##SFCVM_LARGEDATA_DIR=$CVM_LARGEDATA_DIR/model/sfcvm
if test x"$CVM_LARGEDATA_DIR" != x; then
   # test directory existence
   SFCVM_LARGEDATA_DIR=$CVM_LARGEDATA_DIR/model/sfcvm
# In docker container building.. this is not accessible  yet
   if test x"$CVM_IN_DOCKER" != x; then 
     AM_CONDITIONAL([WITH_SFCVM_LARGEDATA_DIR], true)
     AC_SUBST(SFCVM_LARGEDATA_DIR)
   else
     AC_CHECK_FILE([$SFCVM_LARGEDATA_DIR/USGS_SFCVM_v21-1_detailed.h5],
                 [AM_CONDITIONAL([WITH_SFCVM_LARGEDATA_DIR], true) AC_SUBST(SFCVM_LARGEDATA_DIR)],
                 [AM_CONDITIONAL(WITH_SFCVM_LARGEDATA_DIR, false)])
   fi
else
   AM_CONDITIONAL(WITH_SFCVM_LARGEDATA_DIR, false)
fi


# Set final CFLAGS and LDFLAGS
CFLAGS="$CFLAGS"
LDFLAGS="$LDFLAGS -lm"

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile dependencies/Makefile test/Makefile test_geomodelgrids/Makefile])

AC_OUTPUT


