## Process this file with automake to produce Makefile.in

MTK_FONTFILE = FreeSans.ttf

pkgdata_DATA = \
	$(MTK_FONTFILE)

EXTRA_DIST = $(pkgdata_DATA)

noinst_LIBRARIES = libsdlmtk.a

noinst_PROGRAMS = mtk-test

INCLUDES = \
	-DMTK_FONTFILE=\"$(pkgdatadir)/$(MTK_FONTFILE)\" \
	-I../../../include \
	-I../../../include/ui/sdlmtk \
	$(SDL_CFLAGS)

libsdlmtk_a_SOURCES = \
	mtk_button.cc \
	mtk_togglebutton.cc \
	mtk_menubutton.cc \
	mtk_colors.cc \
	mtk_container.cc \
	mtk_font.cc \
	mtk_screen.cc \
	mtk_hbox.cc \
	mtk_label.cc \
	mtk_vbox.cc \
	mtk_widget.cc

mtk_test_SOURCES = \
	mtk-test.cc

mtk_test_LDADD = \
	 libsdlmtk.a $(SDL_LIBS) -lSDL_ttf
