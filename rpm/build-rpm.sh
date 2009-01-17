#!/bin/bash

KCEMUHOME="../../kcemu"
RPMDIR="/usr/src/packages"

if [ ! -f "$KCEMUHOME/src/kc/kc.cc" ]
then
	echo "KCemu home directory ($KCEMUHOME) not found."
	exit 1
fi

if [ ! -d "$RPMDIR/SRPMS" ]
then
	echo "RPMDIR ($RPMDIR) not found."
	exit 1
fi

source "$KCEMUHOME/VERSION"

rm -rf "$RPMDIR/BUILD/$KCEMUHOME/$PACKAGE-$KCEMU_VERSION"
cp "$KCEMUHOME/$PACKAGE-$KCEMU_VERSION.tar.gz" "$RPMDIR/SOURCES/" || exit
cp "$KCEMUHOME/rpm/KCemu.spec" "$RPMDIR/SPECS" || exit

cd "$RPMDIR/SPECS" || exit

trap 'rm -rf "$BUILDDIR"' EXIT

BUILDDIR=$(mktemp /tmp/kcemu-rpm.XXXXXXXXXX) || exit

rpmbuild -v -bb --buildroot "$BUILDDIR" KCemu.spec
