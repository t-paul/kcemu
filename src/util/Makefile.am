## Process this file with automake to produce Makefile.in

# bin_PROGRAMS = wav2kc

noinst_PROGRAMS = send_file show-wav ft disp-wav wavtokc # audio2kc

LIBS=

CPPFLAGS = \
	-DDEBUG

#wav2kc_SOURCES = \
#	wav2kc.c

wavtokc_SOURCES = \
	wavtokc2.c wav2kc.c

wavtokc_LDADD = \
	-lm

#audio2kc_SOURCES = \
#	audio2kc.c
#
#audio2kc_LDADD = \
#	-laudiofile

send_file_SOURCES = \
	send_file.c

INCLUDES = `gtk-config --cflags`

show_wav_SOURCES = \
	show-wav.c
show_wav_LDFLAGS = \
	`gtk-config --libs`

ft_SOURCES = \
	ft.c
ft_LDADD = \
	-lm

disp_wav_SOURCES = \
	disp-wav.c
disp_wav_LDADD = \
	-lncurses
