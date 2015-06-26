#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT(gpart, 0.2.3-dev, https://github.com/baruch/gpart/issues)
AM_INIT_AUTOMAKE([1.11])
AC_CONFIG_SRCDIR([src/gpart.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/ioctl.h sys/mount.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getpagesize memset strchr strdup strerror strtoul])

# Configure system services.
AC_SYS_LARGEFILE

AC_CONFIG_FILES([Makefile
                 man/Makefile
                 src/Makefile
                 man/gpart.8])
AC_OUTPUT
