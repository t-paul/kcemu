## Process this file with automake to produce Makefile.in

LIBS =
SUFFIXES = .asm .bin

if COMPILE_Z80_ASM
noinst_PROGRAMS = kc853.bin
endif

kc853_bin_SOURCES = kc853.asm
kc853_bin_LINK = p2bin -r '$$-$$'
kc853_bin_LDADD =

asmdir = $(pkgdatadir)/asm/autostart
asm_DATA = kc853.asm

z80libdir = $(pkgdatadir)/lib/z80
z80lib_DATA = $(noinst_PROGRAMS)

.asm.o:
	asl $< -o $@

EXTRA_DIST = $(asm_DATA) $(noinst_PROGRAMS)
