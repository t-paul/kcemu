## Process this file with automake to produce Makefile.in

bin_PROGRAMS = tdtodump

EXTRA_DIST = extract-system-tracks.sh

noinst_LIBRARIES = libdisk.a

AM_CPPFLAGS = \
	-I../../include \
	-DDEBUG -DLINUX

libdisk_a_SOURCES = \
	disk.c \
	teledisk.c \
	load_dir.c \
	load_cdm.c \
	load_td0.c \
	load_22dsk.c

tdtodump_SOURCES = \
	tdtodump.c

tdtodump_LDADD = \
	libdisk.a
