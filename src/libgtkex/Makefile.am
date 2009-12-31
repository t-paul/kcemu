## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libgtkex.a

noinst_PROGRAMS = libgtkex-test

INCLUDES = \
	-I../../include

AM_CPPFLAGS = \
        $(GTK_CFLAGS)

libgtkex_a_SOURCES = \
	gtkfilebutton.c \
	gtkledline.c

libgtkex_test_SOURCES = \
	libgtkex-test.c

libgtkex_test_LDADD = \
	libgtkex.a \
	$(GTK_LIBS)
