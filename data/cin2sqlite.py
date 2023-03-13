#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# cin2db - Convert cin table to sqlite db
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

REGION_UNIFIED_ExtA  = 1
REGION_ARRAY_SPECIAL = 2
REGION_UNIFIED_ExtB  = 3
REGION_UNIFIED_ExtCD = 4
REGION_UNIFIED_ExtE  = 5
REGION_UNIFIED_ExtF  = 6
REGION_ARRAY_SYMBOL  = 7
REGION_UNIFIED_ExtG  = 8

STR_UNIFIED_ExtA  = "CJK Unified Ideographs + Extension A"
STR_ARRAY_SPECIAL = "Array30 Special Code"
STR_UNIFIED_ExtB  = "CJK Unified Ideographs Extension B"
STR_UNIFIED_ExtCD = "CJK Unified Ideographs Extension C & D"
STR_UNIFIED_ExtE  = "CJK Unified Ideographs Extension E"
STR_UNIFIED_ExtF  = "CJK Unified Ideographs Extension F"
STR_ARRAY_SYMBOL  = "CJK Symbols & Punctuation for Array30 input method (w+0~9)"
STR_UNIFIED_ExtG  = "CJK Unified Ideographs Extension G"

REG_STACK = []


def array_updatedb(table_file):
        con = sqlite.connect("array.db")
        cur = con.cursor()

        # read from the text table
        f = open(table_file, 'r')
        for ln in f.readlines():
            ln = ln.strip()

            if  (ln == "# Begin of " + STR_UNIFIED_ExtA):
                print("Enter: " + STR_UNIFIED_ExtA)
                REG_STACK.append(REGION_UNIFIED_ExtA)

            elif(ln == "# Begin of " + STR_ARRAY_SPECIAL):
                print("Enter: " + STR_ARRAY_SPECIAL)
                REG_STACK.append(REGION_ARRAY_SPECIAL)

            elif(ln == "# Begin of " + STR_UNIFIED_ExtB):
                print("Enter: " + STR_UNIFIED_ExtB)
                REG_STACK.append(REGION_UNIFIED_ExtB)

            elif(ln == "# Begin of " + STR_UNIFIED_ExtCD):
                print("Enter: " + STR_UNIFIED_ExtCD)
                REG_STACK.append(REGION_UNIFIED_ExtCD)

            elif(ln == "# Begin of " + STR_UNIFIED_ExtE):
                print("Enter: " + STR_UNIFIED_ExtE)
                REG_STACK.append(REGION_UNIFIED_ExtE)

            elif(ln == "# Begin of " + STR_UNIFIED_ExtF):
                print("Enter: " + STR_UNIFIED_ExtF)
                REG_STACK.append(REGION_UNIFIED_ExtF)

            elif(ln == "# " + STR_ARRAY_SIMPLE):
                print(STR_ARRAY_SIMPLE)
                REG_STACK.append(REGION_ARRAY_SIMPLE)

            elif(ln == "# End of " + STR_UNIFIED_ExtA):
                print("Exit: " + STR_UNIFIED_ExtA)
                REG_STACK.pop()

            elif(ln == "# End of " + STR_ARRAY_SPECIAL):
                print("Exit: " + STR_ARRAY_SPECIAL)
                REG_STACK.pop()

            elif(ln == "# End of " + STR_UNIFIED_ExtB):
                print("Exit: " + STR_UNIFIED_ExtB)
                REG_STACK.pop()

            elif(ln == "# End of " + STR_UNIFIED_ExtCD):
                print("Exit: " + STR_UNIFIED_ExtCD)
                REG_STACK.pop()

            elif(ln == "# End of " + STR_UNIFIED_ExtE):
                print("Exit: " + STR_UNIFIED_ExtE)
                REG_STACK.pop()

            elif(ln == "# End of " + STR_UNIFIED_ExtF):
                print("Exit: " + STR_UNIFIED_ExtF)
                REG_STACK.pop()

            elif(ln == "# End of " + STR_UNIFIED_ExtG):
                print("Exit: " + STR_UNIFIED_ExtG)
                REG_STACK.pop()

            elif(ln == "%chardef end"):
                print("End of CJK Symbols & Punctuation for Array30 input method (w+0~9)")
                REG_STACK.pop()

            elif(ln == "%chardef end"):
                print("this is final")
                REG_STACK.pop()
                print(REG_STACK)

            else:
                if(len(REG_STACK) == 0 or len(ln.strip()) == 0):
                    pass

                else:
                    # Write data to SQL Database
                    r = (str(REG_STACK[-1]) + "\t" + ln).split()
                    print(r)
                    cur.execute('INSERT INTO main (keys, ch, cat, cnt) VALUES ("' + r[1] + '", "' + r[2] + '", "' + r[0] + '", "0")') 

        f.close()
        con.commit()
        con.close()

# empty tables
con = sqlite.connect("array.db")
cur = con.cursor()
cur.execute('DELETE FROM main;')
con.commit()
con.close()

array_updatedb(argv[1])
