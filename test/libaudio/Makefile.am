## Process this file with automake to produce Makefile.in

SUBDIRS = data

TESTS = t_load

check_PROGRAMS = $(TESTS)

INCLUDES = -I../../include $(CPPUNIT_CFLAGS)
LIBS = $(CPPUNIT_LIBS) $(SNDFILE_LIBS) $(AUDIOFILE_LIBS) $(MP3LAME_LIBS) -lefence

t_load_SOURCES = t_load.cc
t_load_LDADD = ../../src/libaudio/libaudio.a
