## Process this file with automake to produce Makefile.in

bin_PROGRAMS = kcemu

noinst_PROGRAMS = kcemu-send

INCLUDES = \
	-I../../include

localedir = $(prefix)/@DATADIRNAME@/locale

CPPFLAGS = \
	-DVERSION=\"$(VERSION)\" \
	-DDATADIR=\"$(pkgdatadir)\" \
	-DPACKAGE=\"$(PACKAGE)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDEBUG \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

kcemu_SOURCES = \
	debug.cc	\
	kc.cc		\
	rc.cc		\
	ctc.cc		\
	ctc1.cc		\
	ctc3.cc		\
	keyb1.cc	\
	keyb3.cc	\
	memory.cc	\
	memory1.cc	\
	memory2.cc	\
	memory3.cc	\
	memory4.cc	\
	sh_mem.cc	\
	disk_io.cc	\
	fdc.cc		\
	fdc_cmd.cc	\
	floppy.cc	\
	pio.cc		\
	pio1.cc		\
	pio2.cc		\
	pio3.cc		\
	pio4.cc		\
	ports.cc	\
	ports1.cc	\
	ports3.cc	\
	ports4.cc	\
	module.cc	\
	mod_ram.cc	\
	mod_ram1.cc	\
	mod_64k.cc	\
	mod_rom.cc	\
	mod_disk.cc	\
	mod_v24.cc	\
	mod_list.cc	\
	tape.cc		\
	disk.cc		\
	wav.cc		\
	timer.cc	\
	z80.cc		\
	sound.cc	\
	disass.cc

kcemu_LDADD = \
	../z80core/libz80.a	\
	../z80core2/libz80.a	\
	../fileio/libfileio.a	\
	../libtape/libtape.a	\
	../ui/gtk/libui_gtk.a	\
	../ui/libui.a		\
	../libdbg/libdbg.a	\
	../cmd/*.o		\
	@INTLLIBS@		\
	$(GTK_LIBS)		\
	$(SDL_LIBS)

kcemu_send_SOURCES = \
	kcemu-send.c

kcemu_send_LDADD = \
	-lncurses
