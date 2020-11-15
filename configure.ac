# vim:set et ts=4:
#
# ibus-tmpl - The Input Bus template project
#
# Copyright (c) 2007-2020 Huang Peng  <shawn.p.huang@gmail.com>
# Copyright (c)           Anthony Fok <foka@debian.org>
# Copyright (c)           Keng-Yu Lin <kengyu@debian.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# if not 1, append datestamp to the version number.
m4_define([package_name], [ibus-array])
m4_define([ibus_array_released], [0])
m4_define([ibus_array_major_version], [0])
m4_define([ibus_array_minor_version], [2])
m4_define([ibus_array_micro_version], [2])

m4_define([ibus_array_version],
    ibus_array_major_version.ibus_array_minor_version.ibus_array_micro_version)

AC_INIT([package_name], [ibus_array_version], [https://github.com/lexical/ibus-array/issues], [package_name])
AM_INIT_AUTOMAKE([1.10])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_SANITY_CHECK
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

# checks for pthreads
AC_SEARCH_LIBS(pthread_create, pthread)

# check ibus
PKG_CHECK_MODULES(IBUS, [
    ibus-1.0
])

#check python
AM_PATH_PYTHON([3.0])

# check sqlite3
PKG_CHECK_MODULES(SQLITE3, [
    sqlite3
])
AC_SUBST(SQLITE3_CFLAGS)
AC_SUBST(SQLITE3_LIBS)

# define GETTEXT_* variables
GETTEXT_PACKAGE="$PACKAGE_NAME"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)


# OUTPUT files
AC_CONFIG_FILES(po/Makefile.in
Makefile
ibus-array.spec
src/Makefile
src/array.inputMethod.xml.in
icons/Makefile
setup/Makefile
setup/ibus-setup-array
setup/config.py
data/Makefile
m4/Makefile
)

AC_OUTPUT
