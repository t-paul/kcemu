#!/bin/bash

#
#  This script is intended to help cross-compiling kcemu on
#  linux host system. Target is Windows/MinGW.
#
#  Compilation now at least requires Gtk+-2.6. Gtk+-2.4 is not
#  supported anymore.
#
#  Support for Windows 95 is not available anymore. Due to the
#  usage of the cairo library, at least Windows 2000 is required.
#

#BUILD_DIR="/tmp/kcemu.build.$$"
BUILD_DIR="/tmp/kcemu.build"
GTK_DEV_PACKAGES_DIR="/data/download/win32-dev/gtk-win32/gtk+-2.14.5"
GTK_RUNTIME_PACKAGES_BASE="/data/download/win32-dev/gtk-win32"
DEP_PACKAGES_DIR="/data/download/win32-dev/gtk-win32/dependencies"
SDL_PACKAGES_DIR="/data/download/win32-dev/libsdl"
GLADE_PACKAGES_DIR="/data/download/win32-dev/gtk-win32/glade-2.6"
KCEMU_DIR="/home/tp/projects/kcemu"

################################################################

source VERSION

################################################################

DEV_DIR="$BUILD_DIR/dev-2.6"
INSTALL_DIR="$BUILD_DIR/kcemu"

CROSS_PKG_CONFIG="$DEV_DIR/cross-pkg-config.sh"

export CPPFLAGS="-I$DEV_DIR/include"
export LDFLAGS="-L$DEV_DIR/lib"
export PKG_CONFIG="$CROSS_PKG_CONFIG"
export PKG_CONFIG_LIBDIR="$DEV_DIR/lib/pkgconfig"
export SDL_CONFIG="$DEV_DIR"/bin/i386-mingw32msvc-sdl-config

export TARGET="i586-mingw32msvc"

u () {
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
	tar xfz "$1"
}

unpack_dev_libs () {
    DIR="$1"
    (
    	if [ -d "$DIR" ]
	then
		echo "gtk development directory $DIR already exists, unpacking skipped."
	else
		mkdir -p "$DIR" && cd "$DIR" || exit 5

		u "$GTK_DEV_PACKAGES_DIR"/glib-dev_2.18.3-1_win32.zip
		u "$GTK_DEV_PACKAGES_DIR"/gtk+-dev_2.14.5-1_win32.zip
		u "$GTK_DEV_PACKAGES_DIR"/pango-dev_1.22.2-1_win32.zip
		u "$GTK_DEV_PACKAGES_DIR"/atk-dev_1.24.0-1_win32.zip
		u "$GTK_DEV_PACKAGES_DIR"/cairo-dev_1.8.0-1_win32.zip

		u "$DEP_PACKAGES_DIR"/pkg-config-0.23-2.zip
		u "$DEP_PACKAGES_DIR"/libiconv-1.9.1.bin.woe32.zip
		u "$DEP_PACKAGES_DIR"/gettext-runtime-dev-0.17-1.zip
		u "$DEP_PACKAGES_DIR"/libpng-dev_1.2.32-1_win32.zip
		u "$DEP_PACKAGES_DIR"/jpeg-6b-4-lib.zip

		u "$DEP_PACKAGES_DIR"/zlib123-dll.zip
		mv zlib1.dll bin/

		u "$GLADE_PACKAGES_DIR"/libglade-dev-2.6.2.zip
		u "$GLADE_PACKAGES_DIR"/libxml2-dev-2.6.27.zip

		patch_libxml2_pkg_config_file

		x "$SDL_PACKAGES_DIR"/SDL-devel-1.2.9-mingw32.tar.gz
		mv SDL-1.2.9/bin/* bin/
		mv SDL-1.2.9/lib/* lib/
		mv SDL-1.2.9/include/* include
		rm -rf SDL-1.2.9
	fi
    )
}

patch_libxml2_pkg_config_file () {
	echo "*"
	echo "* patching libxml2 pkgconfig file to use -lzdll"
	echo "*"
	sleep 1
	patch -p0 << EOF
--- lib/pkgconfig/libxml-2.0.pc.orig	2006-12-13 08:31:44.000000000 +0100
+++ lib/pkgconfig/libxml-2.0.pc	2008-03-21 16:23:12.000000000 +0100
@@ -7,5 +7,5 @@
 Version: 2.6.27
 Description: libXML library version 2.
 Requires:
-Libs: -L\${libdir} -lxml2 -lz
+Libs: -L\${libdir} -lxml2 -lzdll
 Cflags: -I\${includedir}
EOF
	sleep 2
	echo
}

unpack_cur_libs_common () {
	u "$DEP_PACKAGES_DIR"/libiconv-1.9.1.bin.woe32.zip
	u "$DEP_PACKAGES_DIR"/gettext-runtime-0.17-1.zip
	u "$DEP_PACKAGES_DIR"/libpng_1.2.32-1_win32.zip
	u "$DEP_PACKAGES_DIR"/jpeg-6b-4-bin.zip

	u "$GLADE_PACKAGES_DIR"/libglade-2.6.2.zip
	u "$GLADE_PACKAGES_DIR"/libxml2-2.6.27.zip

	u "$DEP_PACKAGES_DIR"/zlib123-dll.zip
	mv zlib1.dll bin

	mkdir libtiff
	u "$DEP_PACKAGES_DIR"/tiff-3.8.1-bin.zip libtiff
	mv libtiff/bin/libtiff3.dll bin/
	rm -rf libtiff

	x "$SDL_PACKAGES_DIR"/SDL-devel-1.2.9-mingw32.tar.gz
	mv SDL-1.2.9/bin/* bin
	rm -rf SDL-1.2.9
}

unpack_cur_libs_gtk () {
    DIR="$1"
    shift
    (
    	if [ -d "$DIR" ]
	then
		echo "gtk runtime directory $DIR already exists, unpacking skipped."
	else
		mkdir -p "$DIR" && cd "$DIR" || exit 6

		for p in $@
		do
			u "$GTK_RUNTIME_PACKAGES_BASE/$DIR/$p"
		done

		# install gtkrc of wimp theme if available
		GTKRC="share/themes/MS-Windows/gtk-2.0/gtkrc"
		if [ -f "$GTKRC" ]
		then
			cp "$GTKRC" etc/gtk-2.0/
		fi

		unpack_cur_libs_common
	fi
    )
    rm -f dist || exit 7
    ln -s "$DIR" dist || exit 8
}

unpack_cur_libs_gtk_2_6_10 () {
	unpack_cur_libs_gtk gtk+-2.6.10 glib-2.6.6.zip gtk+-2.6.10-20050823.zip pango-1.8.2.zip atk-1.9.0.zip
}

unpack_cur_libs_gtk_2_8_13 () {
	unpack_cur_libs_gtk gtk+-2.8.13 glib-2.8.6.zip gtk+-2.8.13.zip pango-1.10.2.zip atk-1.10.3.zip cairo-1.0.2.zip
}

unpack_cur_libs_gtk_2_8_20 () {
	unpack_cur_libs_gtk gtk+-2.8.20 glib-2.12.1.zip gtk+-2.8.20.zip pango-1.12.3.zip atk-1.10.3.zip cairo-1.2.2.zip
}

unpack_cur_libs_gtk_2_10_7 () {
	unpack_cur_libs_gtk gtk+-2.10.7 glib-2.12.7.zip gtk+-2.10.7.zip pango-1.14.9.zip atk-1.12.3.zip cairo-1.2.6.zip
}

unpack_cur_libs_gtk_2_12_0 () {
	unpack_cur_libs_gtk gtk+-2.12.0 glib-2.14.1.zip gtk+-2.12.0.zip pango-1.18.2.zip atk-1.20.0.zip cairo-1.4.10.zip
}

unpack_cur_libs_gtk_2_12_9 () {
	unpack_cur_libs_gtk gtk+-2.12.9 glib-2.16.3.zip gtk+-2.12.9.zip pango-1.20.2.zip atk-1.22.0.zip cairo-1.6.4-1.zip
}

unpack_cur_libs_gtk_2_14_5 () {
	unpack_cur_libs_gtk gtk+-2.14.5 glib_2.18.3-1_win32.zip gtk+_2.14.5-1_win32.zip pango_1.22.2-1_win32.zip atk_1.24.0-1_win32.zip cairo_1.8.0-1_win32.zip
}

compile_kcemu () {
	export BUILD=`cd "KCemu-${KCEMU_VERSION}" && ./config/config.guess`
	echo "*"
	echo "* Building on '$BUILD' for target '$TARGET'..."
	echo "*"

	(
	  cd "KCemu-${KCEMU_VERSION}" || exit 9
	  ./configure \
	  		--build="$BUILD" \
			--host="$TARGET" \
			--target="$TARGET" \
			--program-prefix="" \
			--prefix="$INSTALL_DIR" \
			--with-debug-level=1 \
			--enable-sound \
		&& make install
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
#  create installer for Win95 version
#
#unpack_cur_libs_gtk_2_6_10
#makensis - < "KCemu-${KCEMU_VERSION}/setup/KCemuSetup_gtk2.6.nsi" || exit 4

#
#  create installer for Win2000 version
#
unpack_cur_libs_gtk_2_14_5
makensis - < "KCemu-${KCEMU_VERSION}/setup/KCemuSetup.nsi" || exit 5

exit 0
