# -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([SFCVM],[1.0],[software@scec.org])
AC_CONFIG_AUX_DIR([./aux-config])
AC_CONFIG_SRCDIR([src])
AM_INIT_AUTOMAKE([foreign no-exeext])
AC_PROG_RANLIB

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

if test x"$UCVM_INSTALL_PATH" = x; then
AM_CONDITIONAL(UCVM, false)
UCVM_CFLAGS=
UCVM_LDFLAGS=
else
AM_CONDITIONAL(UCVM, true)
UCVM_CFLAGS="-I$UCVM_INSTALL_PATH/include"
UCVM_LDFLAGS="-L$UCVM_INSTALL_PATH/lib -lucvm -dynamic -L/Users/mei/SCEC/TARGET_UCVM/ucvm_install/lib/proj/lib -lproj -lpthread -lm -ldl"
fi

# Checks for library functions.

# Set final CFLAGS and LDFLAGS
CFLAGS="$CFLAGS $UCVM_LDFAGS"
LDFLAGS="$LDFLAGS $UCVM_LDFAGS"

AC_CONFIG_FILES([Makefile data/Makefile src/Makefile dependencies/Makefile])

AC_OUTPUT


