#!/bin/sh
gcc array.c sqlite3.c test.c -o test_array `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0` -lpthread -ldl
