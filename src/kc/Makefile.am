## Process this file with automake to produce Makefile.in

SUBDIRS = prefs kcnet kramermc muglerpc vcs80 c80

bin_PROGRAMS = kcemu

if !TARGET_MINGW
noinst_PROGRAMS = kcemu-send
endif

if USE_INCLUDED_GETOPT
GETOPTLIB = ../getopt/libgetopt.a
endif

localedir = $(prefix)/share/locale

AM_CPPFLAGS = \
	-I../../include \
	-DKCEMU_PACKAGE=\"$(PACKAGE)\" \
	-DKCEMU_DATADIR=\"$(pkgdatadir)\" \
	-DKCEMU_LOCALEDIR=\"$(localedir)\" \
	-DDEBUG \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

JOYSTICK_EXTRA_kcemu_SOURCES = mod_js.cc
V24_EXTRA_kcemu_SOURCES = mod_v24.cc
MINGW_EXTRA_kcemu_SOURCES = kcemu.rc

EXTRA_kcemu_SOURCES = 

if HAVE_JOYSTICK
EXTRA_kcemu_SOURCES += $(JOYSTICK_EXTRA_kcemu_SOURCES)
endif

if HAVE_V24
EXTRA_kcemu_SOURCES += $(V24_EXTRA_kcemu_SOURCES)
endif

if TARGET_MINGW
EXTRA_kcemu_SOURCES += $(MINGW_EXTRA_kcemu_SOURCES)
endif

EXTRA_DIST = $(JOYSTICK_EXTRA_kcemu_SOURCES) $(V24_EXTRA_kcemu_SOURCES) $(MINGW_EXTRA_kcemu_SOURCES)

kcemu_SOURCES = \
	kc.cc		\
	ic.cc		\
	ctc.cc		\
	ctc1.cc		\
	ctc3.cc		\
	ctc6.cc		\
	ctc8.cc		\
	ctc9.cc		\
	ctc_fdc.cc	\
	pio_dio.cc	\
	pio_vdip.cc	\
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
	plotter.cc	\
	sh_mem.cc	\
	disk_io.cc	\
	gdc.cc		\
	vis.cc		\
	svg.cc		\
	vdip.cc		\
	vdip_cmd.cc	\
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
	mod_vdip.cc	\
	mod_ram.cc	\
	mod_ram1.cc	\
	mod_r16.cc	\
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
	mod_raf.cc	\
	mod_boot.cc	\
	mod_192k.cc	\
	mod_320k.cc	\
	mod_ramf.cc	\
	mod_fdc.cc	\
	mod_gdc.cc	\
	mod_rtc.cc	\
	mod_disk.cc	\
	mod_4131.cc	\
	mod_m052.cc	\
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
	$(EXTRA_kcemu_SOURCES)


if TARGET_MINGW
SYSLIBS = ../sys/mingw/libsys_mingw.a -lwinmm -lws2_32
else
SYSLIBS = ../sys/unix/libsys_unix.a
endif

kcemu_LDADD = \
	kcnet/libkcnet.a		\
	kramermc/libkramermc.a		\
	muglerpc/libmuglerpc.a		\
	vcs80/libvcs80.a		\
	c80/libc80.a			\
	../fileio/libfileio.a		\
	../libtape/libtape.a		\
	../libdisk/libdisk.a		\
	../libaudio/libaudio.a		\
	../ui/gtk/libui_gtk.a		\
	../ui/generic/libui_generic.a	\
	../ui/libui.a			\
	prefs/libprefs.a		\
	../libgtkex/libgtkex.a		\
	../libdbg/libdbg.a		\
	../cmd/*.o			\
	$(GTK_LIBS)			\
	$(SDL_LIBS)			\
	$(SNDFILE_LIBS)			\
	$(AUDIOFILE_LIBS)		\
	$(MP3LAME_LIBS)			\
	$(VORBIS_LIBS)			\
	$(F_L_A_C_LIBS)			\
	$(LIBTHEORA_LIBS)		\
	$(XVIDCORE_LIBS)		\
	$(LIBDIRAC_LIBS)		\
	$(LIBSCHROEDINGER_LIBS)		\
	$(LIBAVFORMAT_LIBS)		\
	$(VNCSERVER_LIBS)               \
	$(LIBGIF_LIBS)                  \
	$(SYSLIBS)			\
	$(GETOPTLIB)			\
	$(INTLLIBS)			\
	$(Z80EX_LIBS)

kcemu_send_SOURCES = \
	kcemu-send.c

kcemu_send_LDADD = \
	-lncurses -ltinfo $(INTLLIBS)
