AC_COPYRIGHT([Copyright (C) 2010-2011 Santtu Lakkala])
AC_INIT([statzni], [0.0.1], [inz@inz.fi])
AC_PREREQ(2.60)

AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_AUX_DIR(admin)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADER(config.h)
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])


AC_PROG_CC

AC_USE_SYSTEM_EXTENSIONS
AC_C_RESTRICT
CFLAGS="${CFLAGS} -Wall -Wextra"
CFLAGS="${CFLAGS} -Wundef -Wnested-externs"
CFLAGS="${CFLAGS} -Wwrite-strings"
CFLAGS="${CFLAGS} -Wpointer-arith -Wbad-function-cast"
CFLAGS="${CFLAGS} -Wmissing-declarations -Wmissing-prototypes"
CFLAGS="${CFLAGS} -Wstrict-prototypes -Wredundant-decls"
CFLAGS="${CFLAGS} -Wunused-parameter -Wold-style-definition"
CFLAGS="${CFLAGS} -Winline"

AC_DEFINE(_FORTIFY_SOURCE, 2, [Define to 2 to get glibc warnings.])

AC_HEADER_ASSERT

AM_INIT_AUTOMAKE([1.11 color-tests])
AM_SILENT_RULES([yes])

AM_PROG_CC_C_O
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

PKG_CHECK_MODULES(GLIB, [glib-2.0])
PKG_CHECK_MODULES(ZLIB, [zlib], [AC_DEFINE([HAVE_ZLIB],1,[Have zlib])])
PKG_CHECK_MODULES(LZMA, [liblzma], [AC_DEFINE([HAVE_LZMA],1,[Have liblzma])])

AC_CHECK_LIB([bz2],[BZ2_bzopen],[AC_CHECK_HEADERS([bzlib.h],[
	      BZ_LIBS=-lbz2
	      AC_SUBST(BZ_LIBS)
	      AC_DEFINE([HAVE_BZLIB],1,[Have bzlib])])])

AC_OUTPUT
