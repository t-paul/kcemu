## Process this file with automake to produce Makefile.in

VPATH = svg

SUBDIRS = original

icondir = $(pkgdatadir)/icons
imagedir = $(pkgdatadir)/images

ICONS = \
	kcemu-bic.xpm \
	kcemu-kc85.xpm \
	kcemu-kc87.xpm \
	kcemu-lc80.xpm \
	kcemu-poly880.xpm \
	kcemu-z1013.xpm

WINICONS = \
	kcemu-bic.ico \
	kcemu-kc85.ico \
	kcemu-kc87.ico \
	kcemu-lc80.ico \
	kcemu-poly880.ico \
	kcemu-z1013.ico

IMAGES = \
	logo-flac.png \
	logo-gtk.png \
	logo-lame.png \
	logo-libsdl.png \
	logo-libsndfile.png \
	logo-openclipartlibrary.png \
	logo-vorbisdotcom.png \
	media-end.png \
	media-play.png \
	media-play.png \
	media-record.png \
	media-record.png \
	media-start.png \
	media-stop.png \
	media-stop.png \
	window-about.png \
	window-audio.png \
	window-copying.png \
	window-disk.png \
	window-help.png \
	window-info.png \
	window-module.png \
	window-question.png \
	window-tape.png \
	window-thanks.png \
	kcemu.png

%.ico : %.xpm
	xpmtoppm --alphaout /tmp/ico.alpha.1.$$$$ $< | ppmquant 256 > /tmp/ico.icon.$$$$ ; \
	ppmtowinicon -andppms /tmp/ico.icon.$$$$ /tmp/ico.alpha.1.$$$$ > $@ ; \
	rm -f /tmp/ico.icon.$$$$ /tmp/ico.alpha.1.$$$$

if TARGET_LINUX
icon_DATA = $(ICONS)
endif

if TARGET_MINGW
icon_DATA = $(WINICONS)
endif

image_DATA = $(IMAGES)

all : $(ICONS) $(WINICONS)

EXTRA_DIST = $(ICONS) $(WINICONS) $(IMAGES)
