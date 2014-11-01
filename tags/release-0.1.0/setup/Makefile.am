# ibus-array - The Array 30 Engine For IBus
#
# Copyright (c) 2009-2014 Choe Hwanjin <choe.hwanjin@gmail.com>
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

setup_hangul_PYTHON = \
	main.py \
	$(NULL)

nodist_setup_hangul_PYTHON = config.py

setup_hanguldir = $(datadir)/ibus-array/setup

libexec_SCRIPTS = ibus-setup-array

CLEANFILES = \
	*.pyc \
	$(NULL)

EXTRA_DIST = \
	ibus-setup-array.in \
	$(NULL)

test:
	$(ENV) DBUS_DEBUG=true \
		   LANG=en_US \
		   PYTHONPATH=$(abs_top_srcdir):$(pyexecdir) \
		   $(PYTHON) $(srcdir)/main.py

