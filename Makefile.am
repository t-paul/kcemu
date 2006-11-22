## Process this file with automake to produce Makefile.in

docdir = $(datadir)/doc/kcemu

ACLOCAL_AMFLAGS = -I config -I config/m4

SUBDIRS = \
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
	setup \
	glade \
	po

if TARGET_MINGW
e = README.de TODO THANKS COPYING COPYING.de KCemuSetup.iss
else
e = README.de TODO THANKS COPYING COPYING.de
endif

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	cp ChangeLog $(DESTDIR)$(docdir)/changelog
