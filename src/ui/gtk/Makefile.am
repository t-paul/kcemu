## Process this file with automake to produce Makefile.in

SUBDIRS = glade

keymapdir = $(pkgdatadir)/keymaps

noinst_LIBRARIES = libui_gtk.a

KEYMAPS = \
	kc851.key \
	kc851_keyboard.xpm \
	kc851_keyboard_pressed.xpm \
	kc854.key \
	kc854_keyboard.xpm \
	kc854_keyboard_pressed.xpm \
	poly880.key \
	poly880_keyboard.xpm \
	poly880_keyboard_pressed.xpm \
	lc80.key \
	lc80_keyboard.xpm \
	lc80_keyboard_pressed.xpm \
	vcs80.key \
	vcs80_keyboard_pressed.xpm \
	vcs80_keyboard.xpm

keymap_DATA = $(KEYMAPS)

EXTRA_DIST = $(KEYMAPS)

INCLUDES = \
	-I../../../include \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

libui_gtk_a_SOURCES = \
	gtkledline.c		\
	ui_gtk.cc		\
	ui_gtk0.cc		\
	ui_gtk1.cc		\
	ui_gtk3.cc		\
	ui_gtk4.cc		\
	ui_gtk6.cc		\
	ui_gtk8.cc		\
	ui_gtk9.cc		\
	ui_gtk_kramermc.cc	\
	ui_gtk_muglerpc.cc	\
	ui_gtk_vcs80.cc		\
	cmd.cc			\
	window.cc		\
	disk.cc			\
	tape.cc			\
	tapeadd.cc		\
	info.cc			\
	help.cc			\
	about.cc		\
	thanks.cc		\
	color.cc		\
	module.cc		\
	keyboard.cc		\
	debug.cc		\
	copying.cc		\
	fbrowse.cc		\
	dialog.cc		\
	image.cc		\
	wav.cc			\
	hedit.cc

if TARGET_LINUX
bin_PROGRAMS = kcemu-remote
kcemu_remote_SOURCES = kcemu-remote.c
kcemu_remote_LDADD = -L/usr/X11R6/lib -lX11 -lXmu $(INTLLIBS)
endif
