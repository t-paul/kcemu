## Process this file with automake to produce Makefile.in

SUBDIRS = data

AM_CPPFLAGS = -I../../include $(CPPUNIT_CFLAGS)

LIBS = \
	$(CPPUNIT_LIBS) \
	$(SNDFILE_LIBS) \
	$(AUDIOFILE_LIBS) \
	$(VORBIS_LIBS) \
	$(MP3LAME_LIBS) \
	-lefence

if HAVE_CPPUNIT

TESTS = t_load

check_PROGRAMS = $(TESTS)

t_load_SOURCES = t_load.cc
t_load_LDADD = ../../src/libaudio/libaudio.a

endif
