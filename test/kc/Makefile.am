## Process this file with automake to produce Makefile.in

TESTS = t_memstream #t_cb_list t_ctc

check_PROGRAMS = $(TESTS)

INCLUDES = -I../../include $(CPPUNIT_CFLAGS)

#t_cb_list_SOURCES = t_cb_list.cc
#t_cb_list_LDADD = ../../src/kc/cb_list.o $(CPPUNIT_LIBS)

#t_ctc_SOURCES = t_ctc.cc
#t_ctc_LDADD = ../../src/kc/ctc.o ../../src/libdbg/libdbg.a $(CPPUNIT_LIBS)

t_memstream_SOURCES = t_memstream.cc
t_memstream_LDADD = $(CPPUNIT_LIBS)
