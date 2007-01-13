## Process this file with automake to produce Makefile.in

# bin_PROGRAMS = wav2kc

noinst_PROGRAMS = disk-tool tape-tool tape-check tape-check-bc tape-merge tape-test tape-basicode tape-split tape-rename basic-decode make-font tp-type dump2txt dump2img

# send_file show-wav ft disp-wav wavtokc audio2kc

LIBS=

AM_CPPFLAGS = \
	-DDEBUG

INCLUDES = \
	-I../../include
	`gtk-config --cflags`

#wav2kc_SOURCES = \
#	wav2kc.c

#wavtokc_SOURCES = \
#	wavtokc2.c wav2kc.c

#wavtokc_LDADD = \
#	-lm

#audio2kc_SOURCES = \
#	audio2kc.c
#
#audio2kc_LDADD = \
#	-laudiofile

#send_file_SOURCES = \
#	send_file.c

#show_wav_SOURCES = \
#	show-wav.c
#show_wav_LDFLAGS = \
#	`gtk-config --libs`

#ft_SOURCES = \
#	ft.c
#ft_LDADD = \
#	-lm

#disp_wav_SOURCES = \
#	disp-wav.c
#disp_wav_LDADD = \
#	-lncurses

disk_tool_SOURCES = \
	disk-tool.c

tape_tool_SOURCES = \
	tape-tool.c tape-type.c

tape_tool_LDADD = \
	../libaudio/libaudio.a		\
        $(SNDFILE_LIBS)                 \
        $(AUDIOFILE_LIBS)               \
        $(MP3LAME_LIBS)                 \
        $(VORBIS_LIBS)                  \
        $(F_L_A_C_LIBS)

tape_test_SOURCES = \
	tape-test.c

tape_test_LDADD = \
	../libaudio/libaudio.a		\
        $(SNDFILE_LIBS)                 \
        $(AUDIOFILE_LIBS)               \
        $(MP3LAME_LIBS)                 \
        $(VORBIS_LIBS)                  \
        $(F_L_A_C_LIBS)

tape_basicode_SOURCES = \
	tape-basicode.c

tape_basicode_LDADD = \
	../libaudio/libaudio.a		\
        $(SNDFILE_LIBS)                 \
        $(AUDIOFILE_LIBS)               \
        $(MP3LAME_LIBS)                 \
        $(VORBIS_LIBS)                  \
        $(F_L_A_C_LIBS)

tape_split_SOURCES = \
	tape-split.c

tape_check_SOURCES = \
	tape-check.c tape-type.c

tape_check_bc_SOURCES = \
	tape-check-bc.c

tape_merge_SOURCES = \
	tape-merge.c

basic_decode_SOURCES = \
	basic-decode.c

basic_decode_LDADD = \
	../fileio/libfileio.a

make_font_SOURCES = \
	make-font.c

make_font_LDADD = \
        $(INTLLIBS)

tape_rename_SOURCES = \
	tape-rename.c tape-type.c

tp_type_SOURCES = \
	tp-type.c

