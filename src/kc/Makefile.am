## Process this file with automake to produce Makefile.in

SUBDIRS = kramermc muglerpc vcs80 c80

bin_PROGRAMS = kcemu

if TARGET_LINUX
noinst_PROGRAMS = kcemu-send
endif

if USE_INCLUDED_GETOPT
GETOPTLIB = ../getopt/libgetopt.a
endif

INCLUDES = \
	-I../../include

localedir = $(prefix)/share/locale

AM_CPPFLAGS = \
	-DKCEMU_PACKAGE=\"$(PACKAGE)\" \
	-DKCEMU_DATADIR=\"$(pkgdatadir)\" \
	-DKCEMU_LOCALEDIR=\"$(localedir)\" \
	-DDEBUG \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)


LINUX_EXTRA_kcemu_SOURCES = mod_v24.cc mod_js.cc mod_4131.cc
MINGW_EXTRA_kcemu_SOURCES = kcemu.rc
ALL_EXTRA_kcemu_SOURCES = $(LINUX_EXTRA_kcemu_SOURCES) $(MINGW_EXTRA_kcemu_SOURCES)

if TARGET_LINUX
EXTRA_kcemu_SOURCES = $(LINUX_EXTRA_kcemu_SOURCES)
endif

if TARGET_MINGW
EXTRA_kcemu_SOURCES = $(MINGW_EXTRA_kcemu_SOURCES)
endif

EXTRA_DIST = $(ALL_EXTRA_kcemu_SOURCES)

kcemu_SOURCES = \
	debug.cc	\
	kc.cc		\
	rc.cc		\
	ic.cc		\
	ctc.cc		\
	ctc1.cc		\
	ctc3.cc		\
	ctc6.cc		\
	ctc8.cc		\
	ctc9.cc		\
	ctc_fdc.cc	\
	pio_dio.cc	\
	ctc_dio.cc	\
	ctc_base.cc	\
	cb_list.cc	\
	daisy.cc	\
	keyb0.cc	\
	keyb1.cc	\
	keyb3.cc	\
	keyb6.cc	\
	keyb8.cc	\
	keyb9.cc	\
	memory.cc	\
	memory0.cc	\
	memory1.cc	\
	memory2.cc	\
	memory3.cc	\
	memory4.cc	\
	memory6.cc	\
	memory7.cc	\
	memory8.cc	\
	memory9.cc	\
	sh_mem.cc	\
	disk_io.cc	\
	gdc.cc		\
	vis.cc		\
	svg.cc		\
	gide.cc		\
	fdc.cc		\
	fdc0s.cc	\
	fdc4.cc		\
	fdc7.cc		\
	fdc9.cc		\
	fdc_cmd.cc	\
	floppy.cc	\
	pio.cc		\
	pio0.cc		\
	pio1.cc		\
	pio2.cc		\
	pio3.cc		\
	pio4.cc		\
	pio6.cc		\
	pio8.cc		\
	pio9.cc		\
	ports.cc	\
	ports0.cc	\
	ports4.cc	\
	ports6.cc	\
	module.cc	\
	mod_dio.cc	\
	mod_ram.cc	\
	mod_ram1.cc	\
	mod_r64.cc	\
	mod_cpm.cc	\
	mod_ram8.cc	\
	mod_64k.cc	\
	mod_segm.cc	\
	mod_128k.cc	\
	mod_256k.cc	\
	mod_512k.cc	\
	mod_1m.cc	\
	mod_4m.cc	\
	mod_rom.cc	\
	mod_urom.cc	\
	mod_auto.cc	\
	mod_rom1.cc	\
	mod_romb.cc	\
	mod_boot.cc	\
	mod_192k.cc	\
	mod_320k.cc	\
	mod_ramf.cc	\
	mod_fdc.cc	\
	mod_gdc.cc	\
	mod_rtc.cc	\
	mod_disk.cc	\
	mod_list.cc	\
	tape.cc		\
	disk.cc		\
	timer.cc	\
	timer1.cc	\
	timer3.cc	\
	poly880.cc	\
	z80.cc		\
	z80_fdc.cc	\
	wav.cc		\
	sound.cc	\
	sound3.cc	\
	sound8.cc	\
	disass.cc	\
	$(EXTRA_kcemu_SOURCES)

if TARGET_UI_GTK
UILIBS_GTK = ../ui/gtk/libui_gtk.a ../ui/gtk/glade/libui_glade.a
endif

if TARGET_UI_SDL
UILIBS_SDL = ../ui/sdl/libui_sdl.a ../ui/sdlmtk/libsdlmtk.a -lSDL_ttf
endif

if TARGET_LINUX
UILIBS_LINUX = $(UILIBS_GTK) $(UILIBS_SDL)
SYSLIBS_LINUX = ../sys/linux/libsys_linux.a
endif

if TARGET_MINGW
UILIBS_MINGW = $(UILIBS_GTK) $(UILIBS_SDL)
SYSLIBS_MINGW = ../sys/mingw/libsys_mingw.a -lwinmm
endif

if TARGET_BEOS
UILIBS_BEOS = ../ui/beos/libui_beos.a
SYSLIBS_BEOS = ../sys/beos/libsys_beos.a -lbe
endif

kcemu_LDADD = \
	kramermc/libkramermc.a		\
	muglerpc/libmuglerpc.a		\
	vcs80/libvcs80.a		\
	c80/libc80.a			\
	../z80core/libz80.a		\
	../z80core2/libz80.a		\
	../fileio/libfileio.a		\
	../libtape/libtape.a		\
	../libdisk/libdisk.a		\
	../libaudio/libaudio.a		\
	$(UILIBS_BEOS)			\
	$(UILIBS_MINGW)			\
	$(UILIBS_LINUX)			\
	../ui/generic/libui_generic.a	\
	../ui/libui.a			\
	../libdbg/libdbg.a		\
	../cmd/*.o			\
	$(GTK_LIBS)			\
	$(SDL_LIBS)			\
	$(SNDFILE_LIBS)			\
	$(AUDIOFILE_LIBS)		\
	$(MP3LAME_LIBS)			\
	$(VORBIS_LIBS)			\
	$(F_L_A_C_LIBS)			\
	$(SYSLIBS_BEOS)			\
	$(SYSLIBS_MINGW)		\
	$(SYSLIBS_LINUX)		\
	$(GETOPTLIB)			\
	$(INTLLIBS)

kcemu_send_SOURCES = \
	kcemu-send.c

kcemu_send_LDADD = \
	-lncurses $(INTLLIBS)

#
# Setting icons for Windows programs with gcc
# http://fragglet.livejournal.com/4448.html
#
# I found I needed the windres rule twice, in the two forms
# shown. The first tells automake to recognise .rc files
# (it doesn't understand the second form), while the version
# of make I was using (GNU make) didn't seem to like the
# second form. Weird.
#
.rc.o:
	$(RC) $^ -o $@
%.o : %.rc
	$(RC) $^ -o $@
