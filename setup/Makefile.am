## Process this file with automake to produce Makefile.in

NSI_FILES = KCemuSetup.nsi KCemuSetup_gtk2.4.nsi

pkgdata_DATA = kcemurc

noinst_DATA = $(NSI_FILES)

EXTRA_DIST = $(pkgdata_DATA) $(NSI_FILES)

KCemuSetup.nsi : KCemuSetup.nsi.in
KCemuSetup_gtk2.4.nsi : KCemuSetup_gtk2.4.nsi.in
