## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libdbg.a

noinst_PROGRAMS = dbg-test

INCLUDES = \
	-I../../include

AM_CPPFLAGS = \
	-DVERSION=\"$(VERSION)\"

libdbg_a_SOURCES = \
	dbg.cc

dbg_test_SOURCES = \
	dbg-test.cc

dbg_test_LDADD = \
	libdbg.a
