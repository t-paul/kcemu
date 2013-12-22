## Process this file with automake to produce Makefile.in

SUBDIRS = prefs

AM_CPPFLAGS = -I../../include $(CPPUNIT_CFLAGS)

if HAVE_CPPUNIT

TESTS = t_memstream

check_PROGRAMS = $(TESTS)

t_memstream_SOURCES = t_memstream.cc
t_memstream_LDADD = $(CPPUNIT_LIBS)

endif
