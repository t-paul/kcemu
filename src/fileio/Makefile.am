## Process this file with automake to produce Makefile.in

bin_PROGRAMS = kc2img kc2tap kc2wav kc2raw

noinst_PROGRAMS = test-load

noinst_LIBRARIES = libfileio.a

INCLUDES = \
	-I../../include

AM_CPPFLAGS = \
	-DVERSION=\"$(VERSION)\" \
	-DFILEIO_DEBUG

libfileio_a_SOURCES = \
	load.c		\
	load_AF.c	\
	load_GPF.c	\
	load_SSS.c	\
	load_BIN.c	\
	load_HS.c	\
	load_TAPE.c	\
	load_BASIC.c	\
	load_BASICODE.c	\
	load_UNKNOWN.c	\
	save_IMG.c	\
	save_TAP.c	\
	save_WAV.c	\
	save_RAW.c

kc2img_SOURCES = kc2img.c
kc2img_LDADD = libfileio.a $(INTLLIBS)

kc2tap_SOURCES = kc2tap.c
kc2tap_LDADD = libfileio.a $(INTLLIBS)

kc2wav_SOURCES = kc2wav.c
kc2wav_LDADD = libfileio.a $(INTLLIBS)

kc2raw_SOURCES = kc2raw.c
kc2raw_LDADD = libfileio.a $(INTLLIBS)

test_load_SOURCES = test_load.c
test_load_LDADD = libfileio.a $(INTLLIBS)
