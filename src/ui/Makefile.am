## Process this file with automake to produce Makefile.in

if TARGET_UI_GTK
SUBDIRS = generic gtk
endif

if TARGET_UI_SDL
SUBDIRS = generic sdl sdlmtk
endif

if TARGET_BEOS
SUBDIRS = generic beos
endif

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
