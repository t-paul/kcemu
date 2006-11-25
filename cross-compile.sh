#!/bin/bash

#BUILD_DIR="/tmp/kcemu.build.$$"
BUILD_DIR="/tmp/kcemu.build"
GTK_DEV_PACKAGES_DIR="/a/download/win32-dev/gtk-win32/gtk-2.4.14"
GTK_CUR_PACKAGES_DIR="/a/download/win32-dev/gtk-win32/gtk-2.8.20"
DEP_PACKAGES_DIR="/a/download/win32-dev/gtk-win32/dependencies"
SDL_PACKAGES_DIR="/a/download/win32-dev/libsdl"
KCEMU_DIR="/home/tp/projects/kcemu"

source VERSION

################################################################

if test $KCEMU_MICRO_VERSION -eq 0
then
	KCEMU_VERSION=${KCEMU_MAJOR_VERSION}.${KCEMU_MINOR_VERSION}
else
	KCEMU_VERSION=${KCEMU_MAJOR_VERSION}.${KCEMU_MINOR_VERSION}.${KCEMU_MICRO_VERSION}
fi

if test x"$KCEMU_EXTRA_VERSION" != x
then
        KCEMU_VERSION=${KCEMU_VERSION}${KCEMU_EXTRA_VERSION}
fi

################################################################

DEV_DIR="$BUILD_DIR/dev-2.4"
CUR_DIR="$BUILD_DIR/current"
INSTALL_DIR="$BUILD_DIR/kcemu"

CROSS_PKG_CONFIG="$DEV_DIR/cross-pkg-config.sh"

export CPPFLAGS="-I$DEV_DIR/include"
export LDFLAGS="-L$DEV_DIR/lib"
export PKG_CONFIG="$CROSS_PKG_CONFIG"
export PKG_CONFIG_LIBDIR="$DEV_DIR/lib/pkgconfig"
export SDL_CONFIG="$DEV_DIR"/bin/i386-mingw32msvc-sdl-config

export TARGET="i586-mingw32msvc"

u () {
	echo "unpacking $1..."
	unzip -q -o "$1"
}

x () {
	echo "unpacking $1..."
	tar xfz "$1"
}

unpack_dev_libs () {
    (
	cd "$1" || exit 4

	u "$GTK_DEV_PACKAGES_DIR"/glib-2.4.7.zip
	u "$GTK_DEV_PACKAGES_DIR"/glib-dev-2.4.7.zip
	u "$GTK_DEV_PACKAGES_DIR"/gtk+-2.4.14.zip
	u "$GTK_DEV_PACKAGES_DIR"/gtk+-dev-2.4.14.zip
	u "$GTK_DEV_PACKAGES_DIR"/pango-1.4.1.zip
	u "$GTK_DEV_PACKAGES_DIR"/pango-dev-1.4.1.zip
	u "$GTK_DEV_PACKAGES_DIR"/atk-1.8.0.zip
	u "$GTK_DEV_PACKAGES_DIR"/atk-dev-1.8.0.zip

	u "$DEP_PACKAGES_DIR"/pkg-config-0.20.zip
	u "$DEP_PACKAGES_DIR"/libiconv-1.9.1.bin.woe32.zip
	u "$DEP_PACKAGES_DIR"/gettext-0.14.5.zip
	u "$DEP_PACKAGES_DIR"/gettext-dev-0.14.5.zip
	u "$DEP_PACKAGES_DIR"/libpng-1.2.7-bin.zip
	u "$DEP_PACKAGES_DIR"/libpng-1.2.7-lib.zip
	u "$DEP_PACKAGES_DIR"/jpeg-6b-4-bin.zip
	u "$DEP_PACKAGES_DIR"/jpeg-6b-4-lib.zip

	u "$DEP_PACKAGES_DIR"/zlib123-dll.zip
	mv zlib1.dll bin

	x "$SDL_PACKAGES_DIR"/SDL-devel-1.2.9-mingw32.tar.gz
	mv SDL-1.2.9/bin/* bin
	mv SDL-1.2.9/lib/* lib
	mv SDL-1.2.9/include/* include
	rm -rf SDL-1.2.9
    )
}

unpack_cur_libs_common () {
	u "$DEP_PACKAGES_DIR"/libiconv-1.9.1.bin.woe32.zip
	u "$DEP_PACKAGES_DIR"/gettext-0.14.5.zip
	u "$DEP_PACKAGES_DIR"/libpng-1.2.8-bin.zip
	u "$DEP_PACKAGES_DIR"/jpeg-6b-4-bin.zip

	u "$DEP_PACKAGES_DIR"/zlib123-dll.zip
	mv zlib1.dll bin

	x "$SDL_PACKAGES_DIR"/SDL-devel-1.2.9-mingw32.tar.gz
	mv SDL-1.2.9/bin/* bin
	rm -rf SDL-1.2.9
}

unpack_cur_libs_gtk_2_8_13 () {
    (
        cd "$1" || exit 6

	u "$GTK_CUR_PACKAGES_DIR"/glib-2.8.6.zip
	u "$GTK_CUR_PACKAGES_DIR"/gtk+-2.8.13.zip
	u "$GTK_CUR_PACKAGES_DIR"/pango-1.10.2.zip
	u "$GTK_CUR_PACKAGES_DIR"/atk-1.10.3.zip
	u "$GTK_CUR_PACKAGES_DIR"/cairo-1.0.2.zip

	unpack_cur_libs_common
    )
}

unpack_cur_libs_gtk_2_8_20 () {
    (
        cd "$1" || exit 6

	u "$GTK_CUR_PACKAGES_DIR"/glib-2.12.1.zip
	u "$GTK_CUR_PACKAGES_DIR"/gtk+-2.8.20.zip
	u "$GTK_CUR_PACKAGES_DIR"/pango-1.12.3.zip
	u "$GTK_CUR_PACKAGES_DIR"/atk-1.10.3.zip
	u "$GTK_CUR_PACKAGES_DIR"/cairo-1.2.2.zip

	unpack_cur_libs_common
    )
}

compile_kcemu () {
	export BUILD=`cd "KCemu-${KCEMU_VERSION}" && ./config/config.guess`
	echo "*"
	echo "* Building on '$BUILD' for target '$TARGET'..."
	echo "*"

	(
	  cd "KCemu-${KCEMU_VERSION}" || exit 5
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

#mkdir "$BUILD_DIR" || exit 1
mkdir "$BUILD_DIR"
mkdir -p "$DEV_DIR" || exit 2
mkdir -p "$CUR_DIR" || exit 3
cd "$BUILD_DIR" || exit 4

#
#  show banner
#
echo "*"
echo "*  cross compiling KCemu version $KCEMU_VERSION"
echo "*"

#
#  prepare pkg-config to use the mingw lib not the system one
#
cat > "$CROSS_PKG_CONFIG" <<EOF
exec /usr/bin/pkg-config "\$@" --define-variable=prefix="$DEV_DIR"
EOF
chmod 755 "$CROSS_PKG_CONFIG"

#
#  unpack emulator source
#
x "${KCEMU_DIR}/KCemu-${KCEMU_VERSION}.tar.gz"

#
#  unpack development libraries
#
unpack_dev_libs "$DEV_DIR"

#
#  configure and compile
#
compile_kcemu

#
#  create installer for Win95 version
#
rm -f dist || exit 7
ln -s "$DEV_DIR" dist || exit 8
makensis - < "KCemu-${KCEMU_VERSION}/setup/KCemuSetup_gtk2.4.nsi" || exit 9
rm dist || exit 10

#
#  unpack runtime libraries
#
#unpack_cur_libs_gtk_2_8_13 "$CUR_DIR"
unpack_cur_libs_gtk_2_8_20 "$CUR_DIR"

#
#  create installer for Win2000 version
#
ln -s "$CUR_DIR" dist || exit 11
makensis - < "KCemu-${KCEMU_VERSION}/setup/KCemuSetup.nsi" || exit 12
rm -f dist

exit 0
