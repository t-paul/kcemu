## Process this file with automake to produce Makefile.in

noinst_LIBRARIES = libz80.a

INCLUDES = \
	-I../../include

#### Makefile for Z80Em
# Following #defines can be used:
# DEBUG                 Compile with builtin debugger
# LSB_FIRST             Compile for a low-endian machine
# X86_ASM               Use inline 80x86 assembly (GCC only)
#
AM_CPPFLAGS = \
	-DLSB_FIRST -DDEBUG

libz80_a_SOURCES = \
	Z80.c		\
	Z80Debug.c

EXTRA_DIST = \
	README.txt	\
	Z80.h		\
	Z80CDx86.h	\
	Z80Codes.h	\
	Z80DAA.h	\
	Z80IO.h		\
	Z80Dasm.c	\
	Z80Dasm.h
