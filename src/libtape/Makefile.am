## Process this file with automake to produce Makefile.in

if USE_INCLUDED_GETOPT
GETOPTLIB = ../getopt/libgetopt.a
endif

TESTS = test test.sh

bin_PROGRAMS = kctape

check_PROGRAMS = test

noinst_PROGRAMS = show_tap kcread2tap

noinst_LIBRARIES = libtape.a

CPPFLAGS = \
	-DDEBUG -DLINUX

INCLUDES = \
	-I../../include

libtape_a_SOURCES = \
	kct.cc

kctape_SOURCES = \
	kctape.cc

show_tap_SOURCES = \
	show_tap.c

kcread2tap_SOURCES = \
	kcread2tap.c

kctape_LDADD = \
	libtape.a ../fileio/libfileio.a ../libdbg/libdbg.a $(GETOPTLIB) $(INTLLIBS)

check:
	rm -f test.kct testfile.1 testfile.2
	./test
	./kctape test.kct -c
	./kctape test.kct --add testfile.1 "Test 1.1"
	./kctape test.kct --add-raw testfile.1 "Test 1.2" "TEST" 0x0200
	./kctape test.kct --add testfile.2 "Test 2.1"
	./kctape test.kct --add-raw testfile.2 "Test 2.2" "TEST" 0x0200
