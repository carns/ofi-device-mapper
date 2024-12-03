#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])

# IMPORTANT: when editing the version  number, edit both the
# MARGO_VERSION_ variables and the argument provided to AC_INIT
AC_INIT([mochi-plumber], [0.0.1], [],[],[])

AC_CONFIG_MACRO_DIR([m4])
LT_INIT

AC_CANONICAL_TARGET
AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])

# we should remove this soon, only needed for automake 1.10 and older
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([mochi-plumber-private.h])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CXXCPP

AC_PROG_MKDIR_P
AC_REQUIRE_CPP

AC_CHECK_SIZEOF([long int])

dnl
dnl Verify pkg-config
dnl
PKG_PROG_PKG_CONFIG
if test "x$PKG_CONFIG" == "x"; then
   AC_MSG_ERROR([Could not find pkg-config utility!])
fi

dnl require libfabric
PKG_CHECK_MODULES([LIBFABRIC],[libfabric],[],
   [AC_MSG_ERROR([Could not find working libfabric installation!])])
LIBS="$LIBFABRIC_LIBS $LIBS"
CPPFLAGS="$LIBFABRIC_CFLAGS $CPPFLAGS"
CFLAGS="$LIBFABRIC_CFLAGS $CFLAGS"

dnl require hwloc
PKG_CHECK_MODULES([HWLOC],[hwloc],[],
   [AC_MSG_ERROR([Could not find working hwloc installation!])])
LIBS="$HWLOC_LIBS $LIBS"
CPPFLAGS="$HWLOC_CFLAGS $CPPFLAGS"
CFLAGS="$HWLOC_CFLAGS $CFLAGS"


AC_ARG_ENABLE(coverage,
              [AS_HELP_STRING([--enable-coverage],[Enable code coverage @<:@default=no@:>@])],
              [case "${enableval}" in
                yes) enable_coverage="yes" ;;
                no) enable_coverage="no" ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-coverage) ;;
               esac],
              [enable_coverage="no"]
)

if test "$enable_coverage" = "yes" ; then
    CPPFLAGS="$CPPFLAGS --coverage -O0"
    LDFLAGS="--coverage -lgcov"
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
