## Process this file with automake to produce Makefile.in

bin_PROGRAMS = kcemu-remote

noinst_LIBRARIES = libui_gtk.a

INCLUDES = \
	-I../../../include \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

libui_gtk_a_SOURCES = \
	gtkledline.c	\
	ui_gtk.cc	\
	ui_gtk0.cc	\
	ui_gtk1.cc	\
	ui_gtk3.cc	\
	ui_gtk4.cc	\
	ui_gtk8.cc	\
	ui_gtk9.cc	\
	cmd.cc		\
	window.cc	\
	disk.cc		\
	tape.cc		\
	tapeadd.cc	\
	info.cc		\
	about.cc	\
	color.cc	\
	module.cc	\
	keyboard.cc	\
	debug.cc	\
	copying.cc	\
	fbrowse.cc	\
	dialog.cc	\
	image.cc	\
	wav.cc		\
	hedit.cc

kcemu_remote_SOURCES = \
	kcemu-remote.c

kcemu_remote_LDADD = \
	-L/usr/X11R6/lib -lX11 -lXmu $(INTLLIBS)
