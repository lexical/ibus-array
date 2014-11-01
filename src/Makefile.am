# ibus-array - The Array 30 engine for IBus
#
# Copyright (c) 2007-2014 Huang Peng <shawn.p.huang@gmail.com>
#                         Yu-Chun Wang <mainlander1122@gmail.com>
#                         Keng-Yu Lin <kengyu@lexical.tw>
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
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


libexec_PROGRAMS = ibus-engine-array

ibus_engine_array_SOURCES = \
	main.c \
	engine.c \
	engine.h \
	array.c \
	array.h \
	$(NULL)
ibus_engine_array_CFLAGS = \
	@IBUS_CFLAGS@ \
	@SQLITE3_CFLAGS@ \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	$(NULL)
ibus_engine_array_LDFLAGS = \
	@IBUS_LIBS@ \
	@SQLITE3_LIBS@ \
	@LIBINTL@ \
	$(NULL)

component_DATA = \
	array.xml \
	$(NULL)
componentdir = @datadir@/ibus/component

EXTRA_DIST = \
	array.xml.in \
	$(NULL)

CLEANFILES = \
	array.xml \
	$(NULL)

array.xml: array.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

test: ibus-engine-array
	$(builddir)/ibus-engine-array
