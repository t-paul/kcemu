## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libcmd.a

AM_CPPFLAGS = \
	$(CPPFLAGS) \
	-I../../include \
	-DVERSION=\"$(VERSION)\"

libcmd_a_SOURCES = \
	cmd.cc		\
	cmdargs.cc	\
	quit.cc		\
	reset.cc	\
	image.cc	\
	load.cc		\
	dbg.cc
