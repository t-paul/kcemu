## Process this file with automake to produce Makefile.in

SUBDIRS = include ROM PRG DISK intl src test images doc man debian po

pkgdata_DATA =

docdir = $(datadir)/doc/kcemu
doc_DATA = README.de TODO THANKS

EXTRA_DIST = $(pkgdata_DATA) $(doc_DATA)

install-data-local:
	cp ChangeLog $(docdir)/changelog
