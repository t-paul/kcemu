## Process this file with automake to produce Makefile.in

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

%.ico : %.xpm
	xpmtoppm --alphaout /tmp/ico.alpha.1.$$$$ $< | ppmquant 256 > /tmp/ico.icon.$$$$ ; \
	pnminvert /tmp/ico.alpha.1.$$$$ > /tmp/ico.alpha.2.$$$$ ; \
	ppmtowinicon -andppms /tmp/ico.icon.$$$$ /tmp/ico.alpha.2.$$$$ > $@ ; \
	rm -f /tmp/ico.icon.$$$$ /tmp/ico.alpha.1.$$$$ /tmp/ico.alpha.2.$$$$

if TARGET_LINUX
pkgdata_DATA = $(ICONS)
endif

if TARGET_MINGW
pkgdata_DATA = $(WINICONS)
endif

all : $(ICONS) $(WINICONS)

EXTRA_DIST = $(ICONS) $(WINICONS)
