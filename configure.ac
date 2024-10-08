dnl ----------------------------------------------------------------
dnl  $Id: configure.in,v 1.19 2002/10/31 16:39:53 torsten_paul Exp $
dnl
dnl  Process this file with autoconf to produce a configure script.
dnl ----------------------------------------------------------------

AC_INIT([KCemu],[m4_esyscmd(sh ./VERSION -v)])

KCEMU_SET_VERSION
VERSION=$KCEMU_VERSION

AC_CONFIG_SRCDIR([include/kc/kc.h])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIRS([config])
AC_CANONICAL_TARGET
AC_CANONICAL_HOST
AC_USE_SYSTEM_EXTENSIONS

dnl -----------------------
dnl  declare extra options
dnl -----------------------

AC_ARG_ENABLE(static-z80ex, [  --enable-static-z80ex   link Z80 emulation statically [default=no]],,enable_static_z80ex=no)
AC_ARG_ENABLE(sound,        [  --enable-sound          enable sound using libSDL (broken!) [default=no]],,enable_sound=no)
AC_ARG_ENABLE(libaudiofile, [  --enable-libaudiofile   enable usage of libaudiofile if present [default=yes]],,enable_libaudiofile=yes)
AC_ARG_ENABLE(libsndfile,   [  --enable-libsndfile     enable usage of libsndfile if present [default=yes]],,enable_libsndfile=yes)
AC_ARG_ENABLE(libflac,      [  --enable-libflac        enable usage of libFLAC 1.0.x if present [default=yes]],,enable_libflac=yes)
AC_ARG_ENABLE(libvorbis,    [  --enable-libvorbis      enable usage of libvorbis if present [default=yes]],,enable_libvorbis=yes)
AC_ARG_ENABLE(libtheora,    [  --enable-libtheora      enable usage of libtheora if present [default=yes]],,enable_libtheora=yes)
AC_ARG_ENABLE(libxvidcore,  [  --enable-libxvidcore    enable usage of libxvidcore if present [default=yes]],,enable_libxvidcore=yes)
AC_ARG_ENABLE(libdirac,     [  --enable-libdirac       enable usage of libdirac if present [default=yes]],,enable_libdirac=yes)
AC_ARG_ENABLE(libschroedinger, [  --enable-libschroedinger enable usage of libschroedinger if present [default=yes]],,enable_libschroedinger=yes)
AC_ARG_ENABLE(libavformat,  [  --enable-libavformat    enable usage of libavformat if present [default=yes]],,enable_libavformat=yes)
AC_ARG_ENABLE(libgif,       [  --enable-libgif         enable usage of libgif if present [default=yes]],,enable_libgif=yes)
AC_ARG_ENABLE(libvncserver, [  --enable-libvncserver   enable usage of libvncserver if present [default=yes]],,enable_libvncserver=yes)
AC_ARG_ENABLE(libmp3lame,   [  --enable-libmp3lame     enable usage of libmp3lame if present [default=yes]],,enable_libmp3lame=yes)
AC_ARG_ENABLE(ms-bitfields, [  --enable-ms-bitfields   enable usage of gcc -mms-bitfields on MinGW [default=yes]],,enable_ms_bitfields=yes)
AC_ARG_WITH(ui,             [  --with-ui=UI            use User Interface UI (gtk or sdl) [default=auto]],,with_ui=auto)
AC_ARG_WITH(debug_level,    [  --with-debug-level=L    set debug level to L (0..3) [default=1]])

dnl ---------------------
dnl  set package version
dnl ---------------------

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", "package name")
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", "package version")

AC_DEFINE_UNQUOTED(KCEMU_MAJOR_VERSION, $KCEMU_MAJOR_VERSION, "KCemu major version")
AC_DEFINE_UNQUOTED(KCEMU_MINOR_VERSION, $KCEMU_MINOR_VERSION, "KCemu minor version")
AC_DEFINE_UNQUOTED(KCEMU_MICRO_VERSION, $KCEMU_MICRO_VERSION, "KCemu micro version")
AC_DEFINE_UNQUOTED(KCEMU_EXTRA_VERSION, $KCEMU_EXTRA_VERSION, "KCemu extra version")
AC_DEFINE_UNQUOTED(KCEMU_EXTRA_VERSION_TEXT, "$KCEMU_EXTRA_VERSION_TEXT", "KCemu extra version text")
AC_DEFINE_UNQUOTED(KCEMU_VERSION, "$KCEMU_VERSION", "KCemu version")
AC_DEFINE_UNQUOTED(KCEMU_BUILD_DATE, "$KCEMU_BUILD_DATE", "KCemu build date (or source date epoch if available)")
AC_DEFINE_UNQUOTED(KCEMU_BUILD_TIME, "$KCEMU_BUILD_TIME", "KCemu build time (or source date epoch if available)")

AC_DEFINE(PACKAGE_URL, "$PACKAGE_URL", "KCemu Homepage")
AC_DEFINE(PACKAGE_NAME, "$PACKAGE-$KCEMU_VERSION")
AC_DEFINE(PACKAGE_STRING, "$PACKAGE-$KCEMU_VERSION")
AC_DEFINE(PACKAGE_VERSION, "$KCEMU_VERSION")
AC_DEFINE(PACKAGE_TARNAME, "$PACKAGE-${KCEMU_VERSION}.tar.gz")
AC_DEFINE(PACKAGE_BUGREPORT, "$PACKAGE_BUGREPORT")

AC_SUBST(PACKAGE)
AC_SUBST(PACKAGE_URL)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_STRING)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(PACKAGE_TARNAME)
AC_SUBST(PACKAGE_BUGREPORT)

AC_SUBST(VERSION)
AC_SUBST(KCEMU_MAJOR_VERSION)
AC_SUBST(KCEMU_MINOR_VERSION)
AC_SUBST(KCEMU_MICRO_VERSION)
AC_SUBST(KCEMU_EXTRA_VERSION)
AC_SUBST(KCEMU_EXTRA_VERSION_TEXT)

dnl ---------------------
dnl  initialize automake
dnl ---------------------

AM_CONFIG_HEADER(include/kc/config.h)
AM_INIT_AUTOMAKE([no-define])


dnl ---------------------
dnl  checks for programs
dnl ---------------------

AC_PROG_CC

if test x$enable_ms_bitfields = xyes; then
	dnl
	dnl  check for compiler-flag -mms-bitfields needed for mingw/cygwin
	dnl  it defines the bit-field layout to be MS Visual C/C++ compatible
	dnl
	ac_save_CFLAGS=$CFLAGS
	CFLAGS="-mms-bitfields"
	AC_CACHE_CHECK(whether $CC accepts $CFLAGS, ac_cv_prog_cc_mms_bitfields,
		  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM()], [ac_cv_prog_cc_mms_bitfields=yes],
	                                                  [ac_cv_prog_cc_mms_bitfields=no])])
	if test $ac_cv_prog_cc_mms_bitfields = yes; then
		use_ms_bitfields="$CFLAGS"
	else
		use_ms_bitfields="$CFLAGS"
	fi
	CFLAGS="$ac_save_CFLAGS"
fi

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_CHECK_TOOL(RC, windres, :)

AC_PROG_MAKE_SET

dnl -------------------
dnl  check debug level
dnl -------------------

AC_MSG_CHECKING(which debug level is requested)
DBG_LEVEL=1
if test "x$with_debug_level" = "x0"; then
  DBG_LEVEL=0
elif test "x$with_debug_level" = "x1"; then
  DBG_LEVEL=1
elif test "x$with_debug_level" = "x2"; then
  DBG_LEVEL=2
elif test "x$with_debug_level" = "x3"; then
  DBG_LEVEL=3
fi
AC_MSG_RESULT($DBG_LEVEL)
AC_DEFINE_UNQUOTED(DBG_LEVEL, $DBG_LEVEL, "debug level")

dnl ----------------------------
dnl  check for arnold assembler
dnl ----------------------------

AC_CHECK_PROG(HAVE_ASL, asl, "yes", "no")
AC_CHECK_PROG(HAVE_P2BIN, p2bin, "yes", "no")

AM_CONDITIONAL(COMPILE_Z80_ASM, false)
if test "x$HAVE_ASL" = "xyes"; then
  if test "x$HAVE_P2BIN" = "xyes"; then
    AM_CONDITIONAL(COMPILE_Z80_ASM, true)
  fi
fi
    
dnl -----------------------------------
dnl  check on which system we will run
dnl -----------------------------------

AM_CONDITIONAL(TARGET_MINGW, false)

case "$host_os" in
	beos)
		AC_MSG_ERROR([
***
*** BeOS is not supported anymore.
***])
		;;
		
	mingw32*)
		HOST_OS="MinGW"
		AM_CONDITIONAL(TARGET_MINGW, true)
		CFLAGS="$CFLAGS $use_ms_bitfields"
		CXXFLAGS="$CXXFLAGS $use_ms_bitfields"
		LDFLAGS="$LDFLAGS $use_ms_bitfields -mwindows"
		;;

	*) 
		HOST_OS="Unix compatible"
		;;
esac

AC_DEFINE_UNQUOTED(HOST_OS, "$HOST_OS", "host os")

dnl ---------------------------------------------------------------
dnl  checks for typedefs, structures, and compiler characteristics
dnl ---------------------------------------------------------------

AC_C_CONST
AC_C_INLINE
AC_CHECK_HEADERS_ONCE([sys/time.h])

dnl -------------------------------
dnl  check for common header files
dnl -------------------------------

AC_PATH_X
AC_CHECK_HEADERS(mcheck.h)

dnl ----------------------------------
dnl  check for common C++ header files
dnl ----------------------------------

AC_LANG_PUSH(C++)
AC_LANG_POP(C++)

dnl --------------------------------------
dnl  checks for various library functions
dnl --------------------------------------
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    ('int' or 'void').])

AC_CHECK_FUNCS(gettimeofday strstr strchr strrchr strtol strcasecmp strncasecmp mtrace)
AC_CHECK_FUNC(localtime_r, AC_DEFINE(HAVE_LOCALTIME_R, 1, "localtime_r"))
AC_CHECK_FUNC(localtime, AC_DEFINE(HAVE_LOCALTIME, 1, "localtime"))
AC_CHECK_FUNC(getopt,
	      have_getopt=yes
	      AC_DEFINE(HAVE_GETOPT, 1, "getopt() available"))
AC_CHECK_FUNC(getopt_long,
	      have_getopt_long=yes
	      AC_DEFINE(HAVE_GETOPT_LONG, 1, "getopt_long() available"))

AC_MSG_CHECKING(whether to use the included getopt library)
if test x$have_getopt = xyes -o x$have_getopt_long = xyes; then
	use_included_getopt=no
	AM_CONDITIONAL(USE_INCLUDED_GETOPT, false)
else
	use_included_getopt=yes
	AM_CONDITIONAL(USE_INCLUDED_GETOPT, true)
	AC_DEFINE(USE_INCLUDED_GETOPT, 1, "use included getopt")
        AC_DEFINE(HAVE_GETOPT, 1, "getopt() available")
        AC_DEFINE(HAVE_GETOPT_LONG, 1, "getopt_long() available")
fi
AC_MSG_RESULT($use_included_getopt)

dnl -------------------
dnl  configure gettext
dnl -------------------

ALL_LINGUAS="en de"
AM_PO_SUBDIRS
AM_GNU_GETTEXT([no-libtool])
dnl MinGW hack, libintl is there but configure failes with
dnl error undefined reference to `__nl_expand_alias'
if test "x$HOST_OS" = "xMinGW"; then
	LTLIBINTL=""
	LIBINTL=""
	INTLLIBS="-lintl"
	INTLOBJS=""
	USE_INCLUDED_LIBINTL=no
	BUILD_INCLUDED_LIBINTL=no
	AC_MSG_WARN([Disable building libintl for MinGW compilation])
fi
if test "x$USE_INCLUDED_LIBINTL" = "xyes"; then
	AC_DEFINE(USE_INCLUDED_LIBINTL, 1, "use included libintl")
fi

dnl -----------------------------
dnl  check for common libraries
dnl -----------------------------
dnl
dnl  on mingw the libz dll has to be linked
dnl  with -lzdll instead of -lz
dnl
AC_CHECK_LIB(z, crc32, have_libz=-lz, have_libz=no)
if test x$have_libz = xno; then
   AC_CHECK_LIB(zdll, crc32, have_libz=-lzdll, have_libz=no)
fi
if test x$have_libz = xno; then
   AC_MSG_ERROR(Cannot link libz -- check CFLAGS/LDFLAGS)
else
   LIBS="$LIBS $have_libz"
fi

AC_CHECK_LIB(m, pow,,AC_MSG_WARN(Cannot link libm -- check CFLAGS/LDFLAGS))

AM_CONDITIONAL(HAVE_CPPUNIT, false)

dnl -----------------------
dnl  initialize pkg-config
dnl -----------------------
PKG_PROG_PKG_CONFIG(0.20)
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.16.1)

_sep=""
_gtk_libs_temp=""
for lib in $GTK_LIBS
do
	if test "x-lfreetype" = "x$lib"; then
		echo "removing 'freetype' from library list GTK_LIBS"
		continue
	elif test "x-lfontconfig" = "x$lib"; then
		echo "removing 'fontconfig' from library list GTK_LIBS"
		continue
	elif test "x-latk-1.0" = "x$lib"; then
		echo "removing 'atk-1.0' from library list GTK_LIBS"
		continue
	fi

	_gtk_libs_temp="$_gtk_libs_temp$_sep$lib"
	_sep=" "
done
GTK_LIBS="$_gtk_libs_temp"

if test x$cross_compiling = xno; then
	PKG_CHECK_MODULES(X11, x11 >= 1.1)
	PKG_CHECK_MODULES(XMU, xmu >= 1.0)
fi

dnl -----------------------
dnl  check for z80ex
dnl -----------------------

req_z80ex_api=1
min_z80ex_version_major=1
min_z80ex_version_minor=18

min_z80ex_version="${min_z80ex_version_major}.${min_z80ex_version_minor}"
AC_CHECK_LIB(z80ex, z80ex_create, have_libz80ex=yes, have_libz80ex=no)
if test x$have_libz80ex = xyes; then
	Z80EX_LIBS="-lz80ex -lz80ex_dasm"
	AC_MSG_CHECKING(for z80ex - api = $req_z80ex_api - version >= $min_z80ex_version)
	if test x$cross_compiling = xno; then
		ac_save_CFLAGS="$CFLAGS"
		ac_save_LIBS="$LIBS"
		CFLAGS="$CFLAGS $Z80EX_CFLAGS"
		LIBS="$Z80EX_LIBS $LIBS"
		AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <stdio.h>
#include <z80ex/z80ex.h>

int
main()
{
  Z80EX_VERSION *v = z80ex_get_version();
  
  if (v->API_revision != $req_z80ex_api)
    return 1;
  if (v->major < $min_z80ex_version_major)
    return 2;
  if ((v->major == $min_z80ex_version_major) && (v->minor < $min_z80ex_version_minor))
    return 3;

  return 0;
}
]])],[AC_MSG_RESULT(yes)],[have_libz80ex=no],[])
		CFLAGS="$ac_save_CFLAGS"
		LIBS="$ac_save_LIBS"
		AC_SUBST(Z80EX_LIBS)
                z80_emulation_type="dynamic"
	else
		AC_MSG_RESULT([cross compiling; assumed OK...])
	fi
fi
if test x$have_libz80ex = xno; then
	AC_MSG_ERROR([
***
*** Can not find required library z80ex.
***])
fi

AC_MSG_CHECKING(if static linking of z80ex is requested)
if test "x$enable_static_z80ex" = "xyes"; then
	AC_MSG_RESULT(yes)
	AC_MSG_CHECKING(for path of static z80ex libraries)
	z80ex_so=`gcc $LDFLAGS -lz80ex -Wl,--trace /dev/null 2>/dev/null | grep -- '-lz80ex' | sed -e 's/.*(//; s/).*//;'`
	z80ex_path=`dirname "$z80ex_so"`
	z80ex_lib="$z80ex_path/libz80ex.a"
	z80ex_dasm_lib="$z80ex_path/libz80ex_dasm.a"
	if test -f "$z80ex_lib" -a -f "$z80ex_dasm_lib"; then
		AC_MSG_RESULT([$z80ex_path])
		Z80EX_LIBS="$z80ex_lib $z80ex_dasm_lib"
                z80_emulation_type="static"
	else
		AC_MSG_ERROR([
***
*** Can not find path for static z80ex library.
***])
	fi
else
	AC_MSG_RESULT(no)
fi

dnl ---------------------------------
dnl  check for theoraenc library
dnl ---------------------------------
have_theora=no
min_theora="1.0"
min_theoraenc="1.1.1"
if test "x$enable_libtheora" = xyes; then
	PKG_CHECK_MODULES(LIBTHEORA11, theoraenc >= $min_theoraenc,
	[
                have_theora=yes
                have_theora_version=" (version 1.1)"
                AC_DEFINE(HAVE_LIBTHEORA, 1, "have libtheora")
                AC_DEFINE(HAVE_LIBTHEORA11, 1, "have libtheora version 1.1")
                LIBTHEORA_CFLAGS="$LIBTHEORA11_CFLAGS"
                LIBTHEORA_LIBS="$LIBTHEORA11_LIBS"
		AC_SUBST(LIBTHEORA_CFLAGS)
		AC_SUBST(LIBTHEORA_LIBS)
	],[
                PKG_CHECK_MODULES(LIBTHEORA10, theoraenc >= $min_theora theoradec >= $min_theora,
                [
                        have_theora=yes
                        have_theora_version=" (version 1.0)"
                        AC_DEFINE(HAVE_LIBTHEORA, 1, "have libtheora")
                        AC_DEFINE(HAVE_LIBTHEORA10, 1, "have libtheora version 1.0")
                        LIBTHEORA_CFLAGS="$LIBTHEORA10_CFLAGS"
                        LIBTHEORA_LIBS="$LIBTHEORA10_LIBS"
                        AC_SUBST(LIBTHEORA_CFLAGS)
                        AC_SUBST(LIBTHEORA_LIBS)
                ],[
                        AC_MSG_WARN([
***
*** Unable to find theora library (with version number >= $min_theora)!
***
*** This is NO ERROR. It only means you can't capture video in
*** theora/ogg format.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libtheora
***])
                ])
	])
fi

dnl ---------------------------------
dnl  check for libavformat library
dnl ---------------------------------
have_avformat=no
min_libavformat="52.41.0"
if test "x$enable_libavformat" = xyes; then
	PKG_CHECK_MODULES(LIBAVFORMAT, libavformat >= $min_libavformat libavcodec libavutil,
	[
                have_avformat=yes
                AC_DEFINE(HAVE_LIBAVFORMAT, 1, "have libavformat")
		AC_SUBST(LIBAVFORMAT_CFLAGS)
		AC_SUBST(LIBAVFORMAT_LIBS)
	],[
		AC_MSG_WARN([
***
*** Unable to find avformat library (with version number >= $min_libavformat)!
***
*** This is NO ERROR. It only means you can't capture video in
*** avi format.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libavformat
***])
	])
fi

dnl ---------------------------------
dnl  check for libdirac
dnl ---------------------------------
have_dirac=no
min_libdirac="1.0.2"
if test "x$enable_libdirac" = xyes; then
	PKG_CHECK_MODULES(LIBDIRAC, dirac >= $min_libdirac,
	[
                have_dirac=yes
                AC_DEFINE(HAVE_LIBDIRAC, 1, "have libdirac")
		AC_SUBST(LIBDIRAC_CFLAGS)
		AC_SUBST(LIBDIRAC_LIBS)
	],[
		AC_MSG_WARN([
***
*** Unable to find dirac library (with version number >= $min_libdirac)!
***
*** This is NO ERROR. It only means you can't capture video in
*** dirac format.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libdirac
***])
	])
fi

dnl ---------------------------------
dnl  check for libschroedinger
dnl ---------------------------------
have_libschroedinger=no
min_libschroedinger="1.0.8"
if test "x$enable_libschroedinger" = xyes; then
	PKG_CHECK_MODULES(LIBSCHROEDINGER, schroedinger-1.0 >= $min_libschroedinger,
	[
                have_libschroedinger=yes
                AC_DEFINE(HAVE_LIBSCHROEDINGER, 1, "have libschroedinger")
		AC_SUBST(LIBSCHROEDINGER_CFLAGS)
		AC_SUBST(LIBSCHROEDINGER_LIBS)
	],[
		AC_MSG_WARN([
***
*** Unable to find schroedinger library (with version number >= $min_libschroedinger)!
***
*** This is NO ERROR. It only means you can't capture video in
*** schroedinger format.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libschroedinger
***])
	])
fi

dnl ---------------------------------
dnl  check for libxvidcore
dnl ---------------------------------
have_libxvidcore=no
min_libxvidcore="1.2.2"
if test "x$enable_libxvidcore" = xyes; then
        AC_CHECK_LIB(xvidcore,
                     xvid_encore,
                     have_libxvidcore=yes,
                     have_libxvidcore=no)
        if test x$have_libxvidcore = xyes; then
                XVIDCORE_LIBS="-lxvidcore"
                AC_SUBST(XVIDCORE_LIBS)
                AC_DEFINE(HAVE_LIBXVIDCORE, 1, "have libxvidcore")
        else
		AC_MSG_WARN([
***
*** Unable to find libxvidcore (with version number >= $min_libxvidcore)!
***
*** This is NO ERROR. It only means you can't capture video in
*** mpeg4 format.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libxvidcore
***])
        fi
fi

dnl ---------------------------------
dnl  check for libgif
dnl ---------------------------------
if test "x$enable_libgif" = xyes; then
        AC_CHECK_LIB(gif,
                     EGifSetGifVersion,
                     have_libgif=yes,
                     have_libgif=no)
        if test x$have_libgif = xyes; then
                LIBGIF_LIBS="-lgif"
                AC_SUBST(LIBGIF_LIBS)
                AC_DEFINE(HAVE_LIBGIF, 1, "have libgif")
        else
		AC_MSG_WARN([
***
*** Unable to find libgif!
***
*** This is NO ERROR. It only means you can't capture video in
*** animated GIF format.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libgif
***])
        fi
fi

dnl ---------------------------------
dnl  check for libvncserver
dnl ---------------------------------
have_libvncserver=no
min_libvncserver="0.9"
if test "x$enable_libvncserver" = xyes; then
        AC_CHECK_LIB(vncserver,
                     rfbGetScreen,
                     have_libvncserver=yes,
                     have_libvncserver=no)
        if test x$have_libvncserver = xyes; then
                VNCSERVER_LIBS="-lvncserver -lvncclient -ljpeg"
                AC_SUBST(VNCSERVER_LIBS)
                AC_DEFINE(HAVE_LIBVNCSERVER, 1, "have libvncserver")
        else
		AC_MSG_WARN([
***
*** Unable to find libvncserver (with version number >= $min_libvncserver)!
***
*** This is NO ERROR. It only means you can't connect to KCemu
*** using a VNC client.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libvncserver
***])
        fi
fi

_sep=""
_vncserver_libs_temp=""
for lib in $VNCSERVER_LIBS
do
	if test "x-ljpeg" = "x$lib"; then
		echo "removing 'jpeg' from library list VNCSERVER_LIBS"
		continue
	fi

	_vncserver_libs_temp="$_vncserver_libs_temp$_sep$lib"
	_sep=" "
done
VNCSERVER_LIBS="$_vncserver_libs_temp"

dnl ---------------------
dnl  check for libsndfile
dnl ---------------------
have_libsndfile=no
if test x$enable_libsndfile = xyes; then
	AC_CHECK_LIB(sndfile,
		     sf_open_read,
		     have_libsndfile=yes,
		     have_libsndfile=no)
	if test x$have_libsndfile = xyes; then
		SNDFILE_LIBS=-lsndfile
		AC_SUBST(SNDFILE_LIBS)
		AC_DEFINE(HAVE_LIBSNDFILE, 1, "have libsndfile")
		AC_DEFINE(HAVE_LIBSNDFILE_V0, 1, "have libsndfile version 0.0.x")
	else
		AC_CHECK_LIB(sndfile,
			     sf_open,
			     have_libsndfile=yes,
			     have_libsndfile=no)
		if test x$have_libsndfile = xyes; then
			SNDFILE_LIBS=-lsndfile
			AC_SUBST(SNDFILE_LIBS)
			AC_DEFINE(HAVE_LIBSNDFILE, 1, "have libsndfile")
			AC_DEFINE(HAVE_LIBSNDFILE_V1, 1, "have libnsndfile version 1.0.x")
		else
			AC_MSG_WARN([
***
*** Unable to find sndfile library! The audio loader module which
*** requires this library will be disabled.
***
*** This is NO ERROR. It only means you can't playback certain
*** audio file types (e.g. .WAV, .PCM, .AU, .SND) in the emulated
*** tape recorder.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libsndfile
***])
		fi
	fi
fi

dnl -----------------------
dnl  check for libaudiofile
dnl -----------------------
have_libaudiofile=no
if test x$enable_libaudiofile = xyes; then
	AC_CHECK_LIB(audiofile, afOpenFile, have_libaudiofile=yes, have_libaudiofile=no)
	if test x$have_libaudiofile = xyes; then
		AUDIOFILE_LIBS=-laudiofile
		AC_SUBST(AUDIOFILE_LIBS)
		AC_DEFINE(HAVE_LIBAUDIOFILE, 1, "have libaudiofile")
	else
		AC_MSG_WARN([
***
*** Unable to find audiofile library! The audio loader module which
*** requires this library will be disabled.
***
*** This is NO ERROR. It only means you can't playback certain
*** audio file types (e.g. .WAV, .AIFF, .AU) in the emulated
*** tape recorder.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libaudiofile
***])
	fi
fi

dnl ---------------------
dnl  check for libmp3lame
dnl ---------------------
have_libmp3lame=no
if test x$enable_libmp3lame = xyes; then
	AC_CHECK_LIB(mp3lame, lame_decode1_headers, have_libmp3lame=yes, have_libmp3lame=no)
	if test x$have_libmp3lame = xyes; then
		MP3LAME_LIBS=-lmp3lame
		AC_SUBST(MP3LAME_LIBS)
		AC_DEFINE(HAVE_LIBMP3LAME, 1, "have libmp3lame")
	else
		AC_MSG_WARN([
***
*** Unable to find libmp3lame (part of MP3 encoder LAME)! The audio
*** loader module which requires this library will be disabled.
***
*** This is NO ERROR. It only means you can't playback certain
*** audio file types (e.g. .MP3) in the emulated tape recorder.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libmp3lame
***])
	fi
fi

dnl --------------------
dnl  check for libvorbis
dnl --------------------
have_libvorbis=no
if test x$enable_libvorbis = xyes; then
	AC_CHECK_LIB(vorbisfile, ov_open, have_libvorbis=yes, have_libvorbis=no, -lvorbis)
	if test x$have_libvorbis = xyes; then
		VORBIS_LIBS="-lvorbisfile -lvorbis"
		AC_SUBST(VORBIS_LIBS)
		AC_DEFINE(HAVE_LIBVORBIS, 1, "have libvorbis")
	else
		AC_MSG_WARN([
***
*** Unable to find libvorbis library! The audio loader module which
*** requires this library will be disabled.
***
*** This is NO ERROR. It only means you can't playback certain
*** audio file types (e.g. .OGG) in the emulated tape recorder.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libvorbis
***])
	fi
fi

dnl ------------------
dnl  check for libflac
dnl ------------------
have_libflac=no
if test x$enable_libflac = xyes; then
	AC_CHECK_LIB(FLAC, FLAC__file_decoder_process_metadata, have_libflac=yes, have_libflac=no)
	if test x$have_libflac = xyes; then
		F_L_A_C_LIBS=-lFLAC
		AC_SUBST(F_L_A_C_LIBS)
		AC_DEFINE(HAVE_LIBFLAC, 1, "have libFLAC")
	else
		AC_MSG_WARN([
***
*** Unable to find libFLAC library! The audio loader module which
*** requires this library will be disabled.
***
*** Currently only libFLAC 1.0.x is supported!
***
*** This is NO ERROR. It only means you can't playback certain
*** audio file types (e.g. .FLAC) in the emulated tape recorder.
***
*** If you are still annoyed by this warning you can tell ./configure
*** not to check for this library by using --disable-libflac
***])
	fi
fi

dnl -------------------------------------
dnl  check for system specific libraries
dnl -------------------------------------

dnl --------------
dnl  check for OSS
dnl --------------

AC_CHECK_HEADER(sys/soundcard.h,
  [AC_DEFINE(HAVE_OSS, 1, "have oss")
   AC_DEFINE(OSS_HEADER, [<sys/soundcard.h>], "soundcard.h path")],
  [AC_CHECK_HEADER(machine/soundcard.h,
     [AC_DEFINE(HAVE_OSS, 1, "have oss")
      AC_DEFINE(OSS_HEADER, [<machine/soundcard.h>], "soundcard.h path")],
	AC_MSG_WARN([
***
*** Unable to find OSS header file soundcard.h! The audio loader
*** module which requires OSS will be disabled.
***
*** This is NO ERROR. It only means you can't directly load programs
*** by reading directly from the soundcard line input.
***]))])

dnl --------------------------------------
dnl  check for libSDL if sound is enabled
dnl --------------------------------------

if test x$enable_sound = xyes; then
	AC_DEFINE(ENABLE_SOUND, 1, "enable sound")
	AM_PATH_SDL([1.2.0], have_libsdl=yes, have_libsdl=no)
fi

if test x$have_libsdl = xyes; then
	AC_DEFINE(HAVE_LIBSDL, 1, "have libSDL")
else
	if test x$enable_sound = xyes; then
		AC_MSG_WARN([
***
*** Unable to find SDL library (see http://www.libsdl.org/)
*** Sound is disabled!
***])
		enable_sound=no
	fi
fi

dnl ----------------------------------------
dnl  check for the Linux Joystick interface
dnl ----------------------------------------

AC_CHECK_HEADER(linux/joystick.h,
  [AM_CONDITIONAL(HAVE_JOYSTICK, true)
   AC_DEFINE(HAVE_JOYSTICK, 1, "Supported joystick interface present - Currently linux only")],
  [AM_CONDITIONAL(HAVE_JOYSTICK, false)])

dnl -----------------------------------------------
dnl  check for the Unix facilities needed for V.24
dnl -----------------------------------------------

dnl The serial port code also uses unix IPC and forks of an helper process
dnl This code checks for the availability of
dnl  - termios
dnl  - socketpair (communication to helper)
dnl  - fork (starting of helper)
dnl  - fcntl (needed to activate signal raising for sockets)
dnl  - sigaction (for registering the signal handler)

AC_MSG_CHECKING([Unix serial port facilities])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
  [[#include <termios.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <signal.h>]],
  [[struct termios tio;
    int fds[2];
    struct sigaction sa;
    tcgetattr(0, &tio);
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    fork();
    fcntl(fds[0], F_SETOWN, getpid());
    fcntl(fds[0], F_SETFL, FASYNC);
    sigemptyset(&sa.sa_mask);
    sigaction(SIGIO, &sa, NULL); ]]) ],
   [AC_MSG_RESULT(yes)
    AM_CONDITIONAL(HAVE_V24, true)
    AC_DEFINE(HAVE_V24, 1, "Supported serial port interface present")],
   [AC_MSG_RESULT(no)])

AC_CONFIG_FILES([
	Makefile
	include/Makefile
	include/kc/Makefile
	include/kc/prefs/Makefile
	include/kc/kcnet/Makefile
	include/kc/kramermc/Makefile
	include/kc/muglerpc/Makefile
	include/kc/vcs80/Makefile
	include/kc/c80/Makefile
	include/sys/Makefile
	include/getopt/Makefile
	include/cmd/Makefile
	include/ui/Makefile
	include/ui/gtk/Makefile
	include/ui/generic/Makefile
	include/fileio/Makefile
	include/libdbg/Makefile
	include/libtape/Makefile
	include/libdisk/Makefile
	include/libaudio/Makefile
	include/libgtkex/Makefile
	asm/Makefile
	asm/autostart/Makefile
	src/Makefile
	src/kc/kcemu.rc
	src/kc/Makefile
	src/kc/prefs/Makefile
	src/kc/kcnet/Makefile
	src/kc/kramermc/Makefile
	src/kc/muglerpc/Makefile
	src/kc/vcs80/Makefile
	src/kc/c80/Makefile
	src/sys/Makefile
	src/sys/mingw/Makefile
	src/sys/unix/Makefile
	src/getopt/Makefile
	src/cmd/Makefile
	src/ui/Makefile
	src/ui/gtk/Makefile
	src/ui/generic/Makefile
	src/fileio/Makefile
	src/libdbg/Makefile
	src/libtape/Makefile
	src/libdisk/Makefile
	src/libaudio/Makefile
	src/libgtkex/Makefile
	src/util/Makefile
	test/Makefile
	test/kc/Makefile
	test/kc/prefs/Makefile
	test/libaudio/Makefile
	test/libaudio/data/Makefile
	test/libgtkex/Makefile
	roms/Makefile
	roms/z1013/Makefile
	roms/z9001/Makefile
	roms/kc85/Makefile
	roms/poly880/Makefile
	roms/lc80/Makefile
	roms/a5105/Makefile
	roms/kramermc/Makefile
	roms/muglerpc/Makefile
	roms/vcs80/Makefile
	roms/c80/Makefile
	tapes/Makefile
        disks/Makefile
	images/Makefile
	images/original/Makefile
	doc/Makefile
	doc/kcemu-help.xml
	intl/Makefile
	man/Makefile
	man/man1/Makefile
	debian/Makefile
	rpm/Makefile
	rpm/KCemu.spec
	po/Makefile.in
	conf/Makefile
	setup/Makefile
	setup/KCemuSetup.nsi])
AC_OUTPUT




echo ""
echo ""
echo "KCemu configuration summary:"
echo "----------------------------"
echo ""
echo "- version        : KCemu $KCEMU_VERSION"
echo ""
echo "- architecture   : $HOST_OS"
echo "- host system    : $host"
echo "- install prefix : $prefix"
echo ""
echo "- z80 emulation  : $z80_emulation_type"
echo "- sound          : $enable_sound"
echo "- debug level    : $DBG_LEVEL"
echo ""
echo "- audio:"
echo ""
echo "  * sndfile      : $have_libsndfile"
echo "  * audiofile    : $have_libaudiofile"
echo "  * vorbis       : $have_libvorbis"
echo "  * mp3lame      : $have_libmp3lame"
echo "  * FLAC         : $have_libflac"
echo ""
echo "- video:"
echo ""
echo "  * theora       : $have_theora$have_theora_version"
echo "  * avformat     : $have_avformat"
echo "  * dirac        : $have_dirac"
echo "  * schroedinger : $have_libschroedinger"
echo "  * xvid         : $have_libxvidcore"
echo "  * vncserver    : $have_libvncserver"
echo ""
echo ""
