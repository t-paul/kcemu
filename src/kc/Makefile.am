## Process this file with automake to produce Makefile.in

bin_PROGRAMS = kcemu

if TARGET_LINUX
noinst_PROGRAMS = kcemu-send
endif

if USE_INCLUDED_GETOPT
GETOPTLIB = ../getopt/libgetopt.a
endif

INCLUDES = \
	-I../../include

localedir = $(prefix)/@DATADIRNAME@/locale

CPPFLAGS = \
	-DKCEMU_PACKAGE=\"$(PACKAGE)\" \
	-DKCEMU_DATADIR=\"$(pkgdatadir)\" \
	-DKCEMU_LOCALEDIR=\"$(localedir)\" \
	-DDEBUG \
	$(GTK_CFLAGS) \
	$(SDL_CFLAGS)

if TARGET_LINUX
EXTRA_kcemu_SOURCES = mod_v24.cc mod_js.cc mod_4131.cc
endif

kcemu_SOURCES = \
	debug.cc	\
	kc.cc		\
	rc.cc		\
	ic.cc		\
	ctc.cc		\
	ctc1.cc		\
	ctc3.cc		\
	ctc8.cc		\
	ctc9.cc		\
	ctc_fdc.cc	\
	ctc_base.cc	\
	cb_list.cc	\
	keyb0.cc	\
	keyb1.cc	\
	keyb3.cc	\
	keyb8.cc	\
	keyb9.cc	\
	memory.cc	\
	memory0.cc	\
	memory1.cc	\
	memory2.cc	\
	memory3.cc	\
	memory4.cc	\
	memory7.cc	\
	memory8.cc	\
	memory9.cc	\
	sh_mem.cc	\
	disk_io.cc	\
	gdc.cc		\
	vis.cc		\
	svg.cc		\
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
	pio8.cc		\
	pio9.cc		\
	ports.cc	\
	ports0.cc	\
	ports4.cc	\
	module.cc	\
	mod_ram.cc	\
	mod_ram1.cc	\
	mod_r64.cc	\
	mod_cpm.cc	\
	mod_ram8.cc	\
	mod_64k.cc	\
	mod_rom.cc	\
	mod_rom1.cc	\
	mod_192k.cc	\
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
	z80.cc		\
	z80_fdc.cc	\
	wav.cc		\
	sound.cc	\
	sound3.cc	\
	sound8.cc	\
	disass.cc	\
	$(EXTRA_kcemu_SOURCES)

if TARGET_UI_GTK
UILIBS_GTK = ../ui/gtk/libui_gtk.a
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
SYSLIBS_MINGW = ../sys/mingw/libsys_mingw.a
endif

if TARGET_BEOS
UILIBS_BEOS = ../ui/beos/libui_beos.a
SYSLIBS_BEOS = ../sys/beos/libsys_beos.a -lbe
endif

kcemu_LDADD = \
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
