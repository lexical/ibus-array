#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Wrapper script to generate array.db from source tables.
# Usage: gen_array_db.py <cin_file> <phrase_file> <shortcode_file> <output_db>

import sys
import os
import subprocess
import sqlite3

cin_file       = sys.argv[1]
phrase_file    = sys.argv[2]
shortcode_file = sys.argv[3]
db_file        = sys.argv[4]

data_dir = os.path.dirname(os.path.abspath(__file__))

# Create schema from scratch
if os.path.exists(db_file):
    os.unlink(db_file)

con = sqlite3.connect(db_file)
cur = con.cursor()
cur.executescript("""
CREATE TABLE "main" (
    `keys` VARCHAR(5) NOT NULL,
    `ch`   TEXT NOT NULL,
    `cat`  INTEGER NOT NULL,
    `cnt`  INTEGER
);
CREATE TABLE "simple" (
    `keys` TEXT NOT NULL,
    `ch`   TEXT NOT NULL
);
CREATE TABLE "phrase" (
    `keys` VARCHAR(4) NOT NULL,
    `ph`   TEXT NOT NULL
);
CREATE INDEX "idx_main_keys" ON "main" (`keys`);
CREATE INDEX "idx_main_cat_keys" ON "main" (`cat`, `keys`);
CREATE INDEX "idx_main_cat_ch" ON "main" (`cat`, `ch`);
CREATE INDEX "idx_simple_keys" ON "simple" (`keys`);
CREATE INDEX "idx_phrase_keys" ON "phrase" (`keys`);
""")
con.commit()
con.close()

def run(script, *args):
    subprocess.run(
        [sys.executable, os.path.join(data_dir, script)] + list(args),
        check=True
    )

run('cin2sqlite.py',      cin_file,        db_file)
run('updatePhrase.py',    phrase_file,     db_file)
run('updateShortcode.py', shortcode_file,  db_file)
