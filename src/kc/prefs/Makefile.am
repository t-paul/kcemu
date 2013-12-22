## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libprefs.a

AM_CPPFLAGS = \
	-I../../../include \
        -DKCEMU_PACKAGE=\"$(PACKAGE)\" \
        -DKCEMU_DATADIR=\"$(pkgdatadir)\" \
        -DKCEMU_LOCALEDIR=\"$(localedir)\" \
        -DDEBUG \
        $(GTK_CFLAGS) \
        $(SDL_CFLAGS)


libprefs_a_SOURCES = \
	prefs.cc \
	parser.cc \
	profile.cc \
	types.cc \
	strlist.cc \
	dirlist.cc
