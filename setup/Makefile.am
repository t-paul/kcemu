## Process this file with automake to produce Makefile.in

NSI_FILES = KCemuSetup.nsi \
	KCemuSetup_gtk2.6.nsi \
	KCemuSetup_gtk2.8.nsi \
	KCemuSetup_gtk2.10.nsi

pkgdata_DATA = kcemurc

noinst_DATA = $(NSI_FILES)

EXTRA_DIST = $(pkgdata_DATA) $(NSI_FILES)

KCemuSetup.nsi : KCemuSetup.nsi.in
KCemuSetup_gtk2.6.nsi : KCemuSetup_gtk2.6.nsi.in
KCemuSetup_gtk2.8.nsi : KCemuSetup_gtk2.8.nsi.in
KCemuSetup_gtk2.10.nsi : KCemuSetup_gtk2.10.nsi.in
