## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libui_gtk.a

keymapdir = $(pkgdatadir)/keymaps

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
	vcs80_keyboard.xpm \
	a5105.key \
	a5105_keyboard.png \
	a5105_keyboard_pressed.png

keymap_DATA = $(KEYMAPS)

EXTRA_DIST = $(KEYMAPS)

INCLUDES = \
	-I../../../include \
	$(LIBTHEORA_CFLAGS) \
	$(XVIDCORE_CFLAGS) \
	$(LIBDIRAC_CFLAGS) \
	$(LIBSCHROEDINGER_CFLAGS) \
	$(LIBAVFORMAT_CFLAGS) \
	$(VNCSERVER_CFLAGS) \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

AM_CPPFLAGS = \
	$(X11_CFLAGS) \
	$(XMU_CFLAGS)

libui_gtk_a_SOURCES = \
	ui_gtk.cc		\
	ui_gtk_base.cc		\
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
	ui_gtk_c80.cc		\
	cmd.cc			\
	window.cc		\
	main.cc			\
	disk.cc			\
	vdip.cc			\
	tape.cc			\
	tapeadd.cc		\
	info.cc			\
	help.cc			\
	about.cc		\
	thanks.cc		\
	color.cc		\
	module.cc		\
	keyboard.cc		\
	options.cc		\
	screenshot.cc		\
	selector.cc		\
	debug.cc		\
	copying.cc		\
	savemem.cc		\
	fbrowse.cc		\
	dialog.cc		\
	image.cc		\
	wav.cc			\
	plotter.cc		\
	video.cc		\
	theora.cc		\
	xvid.cc			\
	dirac.cc		\
	schro.cc		\
	ffmpeg.cc		\
	vnc.cc			\
	gif.cc			\
	hedit.cc

if TARGET_LINUX
bin_PROGRAMS = kcemu-remote
kcemu_remote_SOURCES = kcemu-remote.c
kcemu_remote_LDADD = $(X11_LIBS) $(XMU_LIBS) $(INTLLIBS)
endif
