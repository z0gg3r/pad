#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([pad], [0.1.1], [zocker@10zen.eu])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([padding], [padding], [has_libpadding=1], [has_libpadding=0])

if test "$has_libpadding" = "0"; then
	AC_MSG_ERROR([You need libpadding from https://github.com/z0gg3r/padding])
fi

# Checks for header files.
AC_CHECK_HEADER([string.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
