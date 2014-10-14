## Process this file with automake to produce Makefile.in

TESTS = t_prefs

check_PROGRAMS = $(TESTS)

AM_CPPFLAGS = -I../../../include $(CPPUNIT_CFLAGS)
LIBS = $(GTK_LIBS)

if TARGET_MINGW
SYSLIB=../../../src/sys/mingw/libsys_mingw.a
else
SYSLIB=../../../src/sys/unix/libsys_unix.a
endif

t_prefs_SOURCES = t_prefs.cc
t_prefs_LDADD = $(CPPUNIT_LIBS) ../../../src/libdbg/libdbg.a ../../../src/kc/prefs/libprefs.a $(SYSLIB)
