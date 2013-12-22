## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libkcnet.a

AM_CPPFLAGS = \
	-I../../../include \
        -DKCEMU_PACKAGE=\"$(PACKAGE)\" \
        -DKCEMU_DATADIR=\"$(pkgdatadir)\" \
        -DKCEMU_LOCALEDIR=\"$(localedir)\" \
        -DDEBUG

libkcnet_a_SOURCES = \
	kcnet.cc \
	kcnetm.cc \
	socket.cc \
	pio_net.cc \
	ipraw.cc \
	udp.cc \
	tcp.cc \
	tftp.cc
