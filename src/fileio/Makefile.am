## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = test-load

noinst_LIBRARIES = libfileio.a

INCLUDES = \
	-I../../include

CPPFLAGS = \
	-DVERSION=\"$(VERSION)\" \
	-DFILEIO_DEBUG

libfileio_a_SOURCES = \
	load.c		\
	load_AF.c	\
	load_GPF.c	\
	load_SSS.c	\
	load_TAPE.c	\
	load_BASIC.c	\
	load_UNKNOWN.c

test_load_SOURCES = \
	test_load.c

test_load_LDADD = \
	libfileio.a
