## Process this file with automake to produce Makefile.in

GTK_CFLAGS = @GTK_CFLAGS@
SDL_CFLAGS = @SDL_CFLAGS@

bin_PROGRAMS = kcemu-remote

noinst_LIBRARIES = libui_gtk.a

INCLUDES = \
	-I../../../include \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

CPPFLAGS = \
	-DVERSION=\"$(VERSION)\" \
        -DDATADIR=\"$(pkgdatadir)\"

libui_gtk_a_SOURCES = \
	gtkledline.c	\
	ui_gtk.cc	\
	ui_gtk1.cc	\
	ui_gtk3.cc	\
	ui_gtk4.cc	\
	window.cc	\
	disk.cc		\
	tape.cc		\
	tapeadd.cc	\
	info.cc		\
	about.cc	\
	color.cc	\
	module.cc	\
	debug.cc	\
	copying.cc	\
	fbrowse.cc	\
	dialog.cc	\
	image.cc	\
	hedit.cc

kcemu_remote_SOURCES = \
	kcemu-remote.c

kcemu_remote_LDADD = \
	-L/usr/X11R6/lib -lX11 -lXmu
