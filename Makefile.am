## Process this file with automake to produce Makefile.in

docdir = $(datadir)/doc/kcemu

SUBDIRS = include ROM PRG DISK intl src test images doc man debian po

if TARGET_MINGW
e = README.de TODO THANKS COPYING COPYING.de KCemuSetup.iss
else
e = README.de TODO THANKS COPYING COPYING.de
endif

EXTRA_DIST = $(e)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	cp ChangeLog $(DESTDIR)$(docdir)/changelog
