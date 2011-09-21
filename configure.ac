# configure.ac
#
#	Copyright: Francis BACQUE <idroxid01@yahoo.fr>
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(bbdock, 0.2.10, Markus Fisch)
AC_CONFIG_SRCDIR([src/Dock.cc])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

CPPFLAGS="$CPPFLAGS -I$prefix/include"
LDFLAGS="$LDFLAGS -L$prefix/lib"
if test "x$prefix" != "x/usr/local"; then
  CPPFLAGS="$CPPFLAGS -I/usr/local/include"
  LDFLAGS="$LDFLAGS -L/usr/local/lib"
fi

# Checks for libraries.
# Check for X headers and libraries
AC_PATH_X

if test "x$no_x" = "xyes"; then
  AC_MSG_ERROR([bbdock requires the X Window System libraries and headers.])
fi

if test "x$x_includes" != "x"; then
  CPPFLAGS="$CPPFLAGS -I$x_includes"
  CXXFLAGS="$CXXFLAGS -I$x_includes"
fi
if test "x$x_libraries" != "x"; then
  LIBS="$LIBS -L$x_libraries"
fi

# Check for required functions in -lX11
XLIB=
AC_CHECK_LIB([X11], [XOpenDisplay], [XLIB=yes], [XLIB=no])
test "x$XLIB" = "xno" && AC_MSG_ERROR([standard X11 libraries not found])

AC_CHECK_HEADERS([X11/Xlib.h], [XLIB=yes], [XLIB=no])
test "x$XLIB" = "xno" && AC_MSG_ERROR([standard X11 headers not found])

AC_CHECK_HEADERS([X11/Xatom.h X11/Xutil.h], [XLIB=yes], [XLIB=no],
[
#include <X11/Xlib.h>
])
test "x$XLIB" = "xno" && AC_MSG_ERROR([standard X11 headers not found])

LIBS="$LIBS -lX11"
############################################

AC_CHECK_LIB([png], [png_create_read_struct], , [AC_MSG_ERROR([libpng not found])],)

AC_CHECK_LIB([z], [zlibVersion], , [AC_MSG_ERROR([zlib not found])],)


# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([bzero select strcasecmp strchr strcspn strstr])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
