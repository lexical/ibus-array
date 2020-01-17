#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Empty array.db
#
# Copyright (c) 2018 Keng-Yu Lin <kengyu@lexical.tw>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

from pysqlite2 import dbapi2 as sqlite
from sys import argv

# empty tables
con = sqlite.connect(argv[1])
cur = con.cursor()
cur.execute('DELETE FROM main;')
cur.execute('DELETE FROM simple;')
con.commit()
con.close()
