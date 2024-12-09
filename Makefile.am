AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS =
bin_SCRIPTS =

noinst_PROGRAMS =
noinst_HEADERS =
TESTS =
XFAIL_TESTS =
SKIPPED_TESTS =
check_PROGRAMS =
EXTRA_PROGRAMS =
CLEANFILES = $(bin_SCRIPTS)
DISTCLEANFILES =
MAINTAINERCLEANFILES =
EXTRA_DIST =
BUILT_SOURCES =
include_HEADERS = include/mochi-plumber.h

TESTS_ENVIRONMENT =

EXTRA_DIST += \
 prepare.sh

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include

AM_CFLAGS =

AM_LIBS =

AM_CXXFLAGS = $(AM_CFLAGS)

lib_LTLIBRARIES = src/libmochi-plumber.la
src_libmochi_plumber_la_SOURCES =

LDADD = src/libmochi-plumber.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = maint/mochi-plumber.pc

include Make.rules

include $(top_srcdir)/src/Makefile.subdir
