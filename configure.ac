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

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for libraries.

dnl OPENSSL
AC_ARG_ENABLE([openssl],
    [AC_HELP_STRING([--enable-openssl],
        [install openSSL  @<:@default=yes@:>@])],
        [if test "$enableval" = "yes" ; then install_openssl=yes; else install_openssl=no; fi],
        [install_openssl=yes])
AM_CONDITIONAL([INSTALL_OPENSSL], [test "$install_openssl" = yes])
AC_ARG_WITH([openssl-incdir],
    [AC_HELP_STRING([--with-openssl-incdir], [directory containing openSSL header files @<:@default=no@:>@])],
        [with_openssl_incdir="$withval"],
        [with_openssl_incdir=no])
AC_SUBST(with_openssl_incdir)
AC_ARG_WITH([openssl-libdir],
    [AC_HELP_STRING([--with-openssl-libdir], [directory containing openSSL library @<:@default=no@:>@])],
        [with_opensssl_libdir="$withval"],
        [with_openssl_libdir=no])
AC_SUBST(with_openssl_libdir)


dnl HDF5
AC_ARG_ENABLE([hdf5],
    [AC_HELP_STRING([--enable-hdf5],
        [install HDF5  @<:@default=yes@:>@])],
        [if test "$enableval" = "yes" ; then install_hdf5=yes; else install_hdf5=no; fi],
        [install_hdf5=yes])
AM_CONDITIONAL([INSTALL_OPENSSL], [test "$install_hdf5" = yes])
AC_ARG_WITH([hdf5-incdir],
    [AC_HELP_STRING([--with-hdf5-incdir], [directory containing HDF5 header files @<:@default=no@:>@])],
        [with_hdf5_incdir="$withval"],
        [with_hdf5_incdir=no])
AC_SUBST(with_hdf5_incdir)
AC_ARG_WITH([hdf5-libdir],
    [AC_HELP_STRING([--with-hdf5-libdir], [directory containing openSSL library @<:@default=no@:>@])],
        [with_hdf5_libdir="$withval"],
        [with_hdf5_libdir=no])
AC_SUBST(with_hdf5_libdir)

dnl PROJ
AC_ARG_ENABLE([proj],
    [AC_HELP_STRING([--enable-proj],
        [install Proj @<:@default=yes@:>@])],
        [if test "$enableval" = "yes" ; then install_proj=yes; else install_proj=no; fi],
        [install_proj=yes])
AM_CONDITIONAL([INSTALL_PROJ], [test "$install_proj" = yes])
AC_ARG_WITH([proj-incdir],
    [AC_HELP_STRING([--with-proj-incdir], [directory containing Proj header files @<:@default=no@:>@])],
        [with_proj_incdir="$withval"],
        [with_proj_incdir=no])
AC_SUBST(with_proj_incdir)
AC_ARG_WITH([proj-libdir],
    [AC_HELP_STRING([--with-proj-libdir], [directory containing Proj library @<:@default=no@:>@])],
        [with_proj_libdir="$withval"],
        [with_proj_libdir=no])
AC_SUBST(with_proj_libdir)


dnl
dnl Setup environment so dependencies are used in build
dnl

CFLAGS="$CFLAGS"
CPPFLAGS="-I$prefix/include $CPPFLAGS"
LDFLAGS="-L$prefix/lib -L$prefix/lib64 $LDFLAGS"

dnl HDF5 (required)
if test "$install_hdf5" = yes ; then
  dnl proj (required by hdf5)
  if test "$install_proj" = yes ; then
    PROJ_INCLUDES="-I$prefix/include"
    PROJ_LDFLAGS="-L$prefix/lib -lproj -lpthread"
    PROJ_INC="$prefix/include"
    PROJ_LIB="$prefix/lib"
  else
    PROJ_INCLUDES=
    PROJ_INC=
    PROJ_LDFLAGS=
    PROJ_LIB=
  fi

  if test "$with_proj_incdir" != no; then
    PROJ_INCLUDES="-I$with_proj_incdir"
    PROJ_INC="$with_proj_incdir"
  fi
  if test "$with_proj_libdir" != no; then
    PROJ_LDFLAGS="-L$with_proj_libdir -lproj -lpthread"
    PROJ_LIB="$with_proj_libdir"
  fi
  AC_SUBST(PROJ_INCLUDES)
  AC_SUBST(PROJ_INC)
  AC_SUBST(PROJ_LDFLAGS)
  AC_SUBST(PROJ_LIB)

  HDF5_INCLUDES="-I$prefix/include"
  HDF5_INC="$prefix/include"
  HDF5_LDFLAGS="-L$prefix/lib -lhdf5"

  HDF5_LIB="$prefix/lib"
else
  HDF5_INCLUDES=
  HDF5_INC=
  HDF5_LDFLAGS=
  HDF5_LIB=
fi

if test "$with_hdf5_incdir" != no ; then
  HDF5_INCLUDES="-I$with_hdf5_incdir"
  HDF5_INC="$with_hdf5_incdir"
fi
if test "$with_hdf5_libdir" != no ; then
  HDF5_LDFLAGS="-L$with_hdf5_libdir -lhdf5"
  HDF5_LIB="$with_hdf5_libdir"
fi
AC_SUBST(HDF5_INCLUDES)
AC_SUBST(HDF5_INC)
AC_SUBST(HDF5_LDFLAGS)
AC_SUBST(HDF5_LIB)


if test "$install_openssl" = yes ; then
  OPENSSL_LDFLAGS="-L$prefix/lib -lssl -lcrytpo"
else
  OPENSSL_LDFLAGS=
fi

if test "$with_openssl_incdir" != no ; then
  OPENSSL_INCLUDES="-I$with_openssl_incdir"
  OPENSSL_INC="$with_openssl_incdir"
fi
if test "$with_openssl_libdir" != no ; then
  OPENSSL_LDFLAGS="-L$with_openssl_libdir -lssl -lcrypto"
  OPENSSL_LIB="$with_openssl_libdir"
fi

AC_SUBST(OPENSSL_INCLUDES)
AC_SUBST(OPENSSL_INC)
AC_SUBST(OPENSSL_LDFLAGS)
AC_SUBST(OPENSSL_LIB)

if test x"$UCVM_INSTALL_PATH" = x; then
AM_CONDITIONAL(UCVM, false)
UCVM_HDF5_INC=
UCVM_HDF5_LIB=
UCVM_PROJ_INC=
UCVM_PROJ_LIB=
UCVM_OPENSSL_INC=
UCVM_OPENSSL_LIB=
else
AM_CONDITIONAL(UCVM, true)
UCVM_HDF5_INC=$HDF5_INC
UCVM_HDF5_LIB=$HDF5_LIB
UCVM_PROJ_INC=$PROJ_INC
UCVM_PROJ_LIB=$PROJ_LIB
UCVM_OPENSSL_INC=$OPENSSL_INC
UCVM_OPENSSL_LIB=$OPENSSL_LIB
AC_SUBST(UCVM_HDF5_INC)
AC_SUBST(UCVM_HDF5_LIB)
AC_SUBST(UCVM_PROJ_INC)
AC_SUBST(UCVM_PROJ_LIB)
AC_SUBST(UCVM_OPENSSL_INC)
AC_SUBST(UCVM_OPENSSL_LIB)
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
     AC_CHECK_FILE([$SFCVM_LARGEDATA_DIR/USGS_SFCVM_v26-0_detailed.h5],
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


