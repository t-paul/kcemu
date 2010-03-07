## Process this file with automake to produce Makefile.in

docdir = $(datadir)/doc/kcemu

ACLOCAL_AMFLAGS = -I config -I config/m4

SUBDIRS = \
	intl \
	include \
	src \
	test \
	asm \
	roms \
	tapes \
	disks \
	images \
	doc \
	man \
	debian \
	rpm \
	conf \
	setup \
	po 

EXTRA_DIST = VERSION README.de TODO THANKS COPYING COPYING.de .pc/.[qv]*

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	cp ChangeLog $(DESTDIR)$(docdir)/changelog

deb:
	unset CFLAGS CXXFLAGS FFLAGS CPPFLAGS LDFLAGS; \
	dpkg-buildpackage -rfakeroot -b -us -uc

CONFIG_STATUS_DEPENDENCIES = VERSION
