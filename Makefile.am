## Process this file with automake to produce Makefile.in

SUBDIRS = include ROM PRG DISK intl src test images doc man debian po

MINGW_EXTRA_DOCS=KCemuSetup.iss

docdir = $(datadir)/doc/kcemu
if TARGET_MINGW
doc_DATA = README.de TODO THANKS COPYING COPYING.de $(MINGW_EXTRA_DOCS)
else
doc_DATA = README.de TODO THANKS COPYING COPYING.de
endif

EXTRA_DIST = $(doc_DATA) $(MINGW_EXTRA_DOCS)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(docdir)
	cp ChangeLog $(DESTDIR)$(docdir)/changelog
