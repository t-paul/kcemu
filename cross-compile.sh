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

#
#  This script is intended to help cross-compiling kcemu on
#  linux host system. Target is Windows/MinGW.
#
#  Compilation now at least requires Gtk+ 2.16.
#
#  Support for Windows 95 is not available anymore. Due to the
#  usage of the cairo library, at least Windows 2000 is required.
#

#BUILD_DIR="/tmp/kcemu.build.$$"
BUILD_DIR="/tmp/kcemu.build"
GTK_DEV_PACKAGES_DIR="/data/download/win32-dev/gtk-win32/2010-01-04"
DEP_PACKAGES_DIR="/data/download/win32-dev/gtk-win32/dependencies"
SDL_PACKAGES_DIR="/data/download/win32-dev/libsdl"
KCEMU_DIR="/home/tp/projects/kcemu"

################################################################

source ./VERSION

################################################################

DEV_DIR="$BUILD_DIR/dev"
DIST_DIR="$BUILD_DIR/dist"
INSTALL_DIR="$BUILD_DIR/kcemu"

CROSS_PKG_CONFIG="$DEV_DIR/cross-pkg-config.sh"

export CPPFLAGS="-I$DEV_DIR/include"
export LDFLAGS="-L$DEV_DIR/lib"
export PKG_CONFIG="$CROSS_PKG_CONFIG"
export PKG_CONFIG_LIBDIR="$DEV_DIR/lib/pkgconfig"
export SDL_CONFIG="$DEV_DIR"/bin/i386-mingw32msvc-sdl-config

export TARGET="i586-mingw32msvc"

u () {
	if [ ! -f "$1" ]
	then
		echo "can't find file '$1'."
		exit 1
	fi
	if [ "x$2" = "x" ]
	then
		echo "unpacking $1..."
		unzip -q -o "$1"
	else
		echo "unpacking $1... (to $2)"
		unzip -q -d "$2" -o "$1"
	fi
}

x () {
	echo "unpacking $1..."
	tar xf "$1"
}

unpack_dev_libs () {
    DIR="$1"
    (
    	if [ -d "$DIR" ]
	then
            echo "gtk development directory $DIR already exists, unpacking skipped."
	else
            mkdir -p "$DIR" && cd "$DIR" || exit 5

            u "$GTK_DEV_PACKAGES_DIR"/glib-dev_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/gtk+-dev_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/pango-dev_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/atk-dev_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/cairo-dev_*_win32.zip

            u "$DEP_PACKAGES_DIR"/pkg-config_0.23-3_win32.zip
            u "$DEP_PACKAGES_DIR"/libiconv-1.9.1.bin.woe32.zip
            u "$DEP_PACKAGES_DIR"/gettext-runtime-dev-0.17-1.zip
            u "$DEP_PACKAGES_DIR"/libpng-dev_1.2.40-1_win32.zip
            u "$DEP_PACKAGES_DIR"/jpeg_7-1_win32.zip
            u "$DEP_PACKAGES_DIR"/freetype-dev_2.3.11-1_win32.zip
            u "$DEP_PACKAGES_DIR"/fontconfig-dev_2.8.0-1_win32.zip
            u "$DEP_PACKAGES_DIR"/z80ex-1.1.18_win32.zip

            x "$DEP_PACKAGES_DIR"/xvidcore-mingw32-1.2.2-gcc42.tar.bz2
            x "$DEP_PACKAGES_DIR"/libtheora-mingw32-1.1.1-gcc42.tar.bz2
            x "$DEP_PACKAGES_DIR"/libogg-mingw32-1.1.4-gcc42.tar.bz2

            u "$DEP_PACKAGES_DIR"/giflib-4.1.4-1-lib.zip

            u "$DEP_PACKAGES_DIR"/zlib123-dll.zip
            mv zlib1.dll bin/

            x "$SDL_PACKAGES_DIR"/SDL-devel-1.2.9-mingw32.tar.gz
            mv SDL-1.2.9/bin/* bin/
            mv SDL-1.2.9/lib/* lib/
            mv SDL-1.2.9/include/* include
            rm -rf SDL-1.2.9
	fi
    )
}

unpack_dist_libs () {
    DIR="$1"
    (
    	if [ -d "$DIR" ]
	then
            echo "gtk runtime directory $DIR already exists, unpacking skipped."
	else
            mkdir -p "$DIR" && cd "$DIR" || exit 6

            u "$GTK_DEV_PACKAGES_DIR"/glib_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/gtk+_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/pango_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/atk_*_win32.zip
            u "$GTK_DEV_PACKAGES_DIR"/cairo_*_win32.zip

            u "$DEP_PACKAGES_DIR"/libiconv-1.9.1.bin.woe32.zip
            u "$DEP_PACKAGES_DIR"/gettext-runtime-0.17-1.zip
            u "$DEP_PACKAGES_DIR"/libpng_1.2.40-1_win32.zip
            u "$DEP_PACKAGES_DIR"/jpeg_7-1_win32.zip
            u "$DEP_PACKAGES_DIR"/libtiff_3.9.1-1_win32.zip
            u "$DEP_PACKAGES_DIR"/freetype_2.3.11-1_win32.zip
            u "$DEP_PACKAGES_DIR"/fontconfig_2.8.0-1_win32.zip
            u "$DEP_PACKAGES_DIR"/expat_2.0.1-1_win32.zip

            x "$DEP_PACKAGES_DIR"/xvidcore-mingw32-1.2.2-gcc42.tar.bz2
            x "$DEP_PACKAGES_DIR"/libtheora-mingw32-1.1.1-gcc42.tar.bz2
            x "$DEP_PACKAGES_DIR"/libogg-mingw32-1.1.4-gcc42.tar.bz2

            u "$DEP_PACKAGES_DIR"/giflib-4.1.4-1-bin.zip

            u "$DEP_PACKAGES_DIR"/zlib123-dll.zip
            mv zlib1.dll bin

            x "$SDL_PACKAGES_DIR"/SDL-devel-1.2.9-mingw32.tar.gz
            mv SDL-1.2.9/bin/* bin
            rm -rf SDL-1.2.9

            # install gtkrc of wimp theme if available
            GTKRC="share/themes/MS-Windows/gtk-2.0/gtkrc"
            if [ -f "$GTKRC" ]
            then
                    cp "$GTKRC" etc/gtk-2.0/
            fi
        fi
    )
}

compile_kcemu () {
	export BUILD=`cd "KCemu-${KCEMU_VERSION}" && ./config/config.guess`
	echo "*"
	echo "* Building on '$BUILD' for target '$TARGET'..."
	echo "*"

	(
	  cd "KCemu-${KCEMU_VERSION}" || exit 9
	  if [ -f config.status ]
	  then
	    echo "File config.status already exists, skipping ./configure run"
	    make install
          else
	    ./configure \
	  		--build="$BUILD" \
			--host="$TARGET" \
			--target="$TARGET" \
			--program-prefix="" \
			--prefix="$INSTALL_DIR" \
			--with-debug-level=1 \
			--enable-sound \
		&& make install
	  fi
	)
}

#
#  show banner
#
echo "*"
echo "*  cross compiling KCemu version $KCEMU_VERSION"
echo "*"

if [ -d "$BUILD_DIR" ]
then
	echo "build directory $BUILD_DIR already exists, creation skipped."
else
	mkdir "$BUILD_DIR" || exit 1
fi

cd "$BUILD_DIR" || exit 2

#
#  unpack emulator source
#
x "${KCEMU_DIR}/KCemu-${KCEMU_VERSION}.tar.gz"

#
#  unpack development libraries
#
unpack_dev_libs "$DEV_DIR"

#
#  unpack runtime libraries
#
unpack_dist_libs "$DIST_DIR"

#
#  prepare pkg-config to use the mingw lib not the system one
#
cat > "$CROSS_PKG_CONFIG" <<EOF
exec /usr/bin/pkg-config "\$@" --define-variable=prefix="$DEV_DIR"
EOF
chmod 755 "$CROSS_PKG_CONFIG"

#
#  configure and compile
#
compile_kcemu || exit 3

#
#  create installer
#
makensis - < "KCemu-${KCEMU_VERSION}/setup/KCemuSetup.nsi" || exit 5

exit 0
