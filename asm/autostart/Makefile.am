## Process this file with automake to produce Makefile.in

LIBS =
SUFFIXES = .asm .bin

Z80FILES = kc853.bin

if COMPILE_Z80_ASM
noinst_PROGRAMS = $(Z80FILES)
kc853_bin_SOURCES = kc853.asm
kc853_bin_LINK = p2bin -r '$$-$$'
kc853_bin_LDADD =
endif

asmdir = $(pkgdatadir)/asm/autostart
asm_DATA = kc853.asm

z80libdir = $(pkgdatadir)/lib/z80
z80lib_DATA = $(Z80FILES)

.asm.o:
	asl $< -o $@

EXTRA_DIST = $(asm_DATA) $(Z80FILES)
