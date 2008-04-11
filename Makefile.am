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
	conf \
	setup \
	glade \
	po 

EXTRA_DIST = VERSION README.de TODO THANKS COPYING COPYING.de

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	cp ChangeLog $(DESTDIR)$(docdir)/changelog

CONFIG_STATUS_DEPENDENCIES = VERSION
