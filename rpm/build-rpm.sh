#!/bin/bash
#
#  KCemu -- The emulator for the KC85 homecomputer series and much more.
#  Copyright (C) 1997-2010 Torsten Paul
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

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
