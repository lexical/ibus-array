#!/usr/bin/python
# -*- coding: utf-8 -*-

from pysqlite2 import dbapi2 as sqlite

def array_updatedb(table_file, table):
	con = sqlite.connect("array.db")
	cur = con.cursor()
	cur.execute('select * from ' + table)
	tbl = cur.fetchall()

	# read from the text table
	f = open(table_file, 'r')
	z = map(lambda x:x.split('\t'), filter(lambda k:(k[0] != '#' and k[0] != '%' and len(k.strip()) != 0), f.readlines()))
	k = map(lambda y:(y[0].lower(), y[1].strip(' \n')), z)
	f.close()

	# update the database
	for i, j in k:
		cur.execute('INSERT INTO ' + table + ' (keys, ch) VALUES ("' + i + '", "' + j + '");')

	con.commit()
	con.close()

# empty tables
con = sqlite.connect("array.db")
cur = con.cursor()
cur.execute('DELETE FROM main;')
cur.execute('DELETE FROM short;')
cur.execute('DELETE FROM special;')
con.commit()
con.close()

array_updatedb('array-special.cin', 'special')
array_updatedb('array-shortcode.cin', 'short')
array_updatedb('array30.cin', 'main')
