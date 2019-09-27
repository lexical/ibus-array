/*
 ibus-array - The Array 30 Engine for IBus

 Copyright (c) 2009-2019 Yu-Chun Wang <mainlander1122@gmail.com>
                         Keng-Yu Lin <kengyu@lexical.tw>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _ARRAY_HEADER_
#define _ARRAY_HEADER_

#include "sqlite3.h"
#include <glib.h>

typedef struct _ArrayContext {
    sqlite3 *conn;
} ArrayContext;

ArrayContext* array_create_context();
void array_release_context(ArrayContext *context);
GString* array_get_preedit_string(GString *preedit);
GArray* array_get_candidates_from_main(ArrayContext *context, gchar *keys);
GArray* array_get_candidates_from_simple(ArrayContext *context, gchar *keys);
GArray* array_get_candidates_from_special(ArrayContext *context, gchar *keys);
GArray* array_get_reverted_key_candidates_from_special(ArrayContext *context, gchar *ch);
GArray* array_get_reverted_char_candidates_from_special(ArrayContext *context, gchar *keys);
void array_release_candidates(GArray *candidates);

gboolean array_input_key_is_not_special(ArrayContext* content, const gchar* keys, const gchar* ch);

#endif
