## Process this file with automake to produce Makefile.in

SUBDIRS = gtk

noinst_LIBRARIES = libui.a

INCLUDES = \
	-I../../include

CPPFLAGS = \
	-DVERSION=\"$(VERSION)\"

libui_a_SOURCES = \
	log.cc		\
	error.cc	\
	status.cc	\
	commands.cc	\
	hsv2rgb.cc
