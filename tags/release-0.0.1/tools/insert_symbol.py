#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sqlite3
import codecs

def create_dict(filename, sqlfilename):
    kdict = {}
    f = codecs.open(filename, "r", "UTF-8")
    for line in f:
        ss = line.strip().split('\t')
        keys = ss[0].lower()
        ch = ss[1]
        if keys in kdict:
            li = kdict[keys]
            li.append(ch)
        else:
            kdict[keys] = [ ch ]
    return kdict

if __name__ == '__main__':
    conn = sqlite3.connect('array.db')
    c = conn.cursor()

    di = create_dict("array_symbol.txt", "array.db")
    dkeys = di.keys()
    dkeys.sort()
    for key in dkeys:
        li = di[key]
        for t in li:
            c.execute("INSERT INTO main (keys,ch) VALUES (?,?)", (key, t))
    conn.commit()
    c.close()
    conn.close()
