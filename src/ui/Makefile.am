## Process this file with automake to produce Makefile.in

SUBDIRS = generic gtk

noinst_LIBRARIES = libui.a

INCLUDES = \
	-I../../include

AM_CPPFLAGS = \
	-DVERSION=\"$(VERSION)\"

libui_a_SOURCES = \
	log.cc		\
	error.cc	\
	status.cc	\
	commands.cc	\
	hsv2rgb.cc
