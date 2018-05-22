/*
 ibus-array - The Array 30 Engine for IBus

 Copyright (c) 2009-2014 Yu-Chun Wang <mainlander1122@gmail.com>
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

#include "array.h"

#include <stdlib.h>

#define ARRAY_DB_FILE_PATH (PKGDATADIR "/tables/array.db")

static gchar* valid_key_map[] = {
    "1-", //a
    "5v", //b
    "3v", //c
    "3-", //d
    "3^", //e
    "4-", //f
    "5-", //g
    "6-", //h
    "8^", //i
    "7-", //j
    "8-", //k
    "9-", //l
    "7v", //m
    "6v", //n
    "9^", //o
    "0^", //p
    "1^", //q
    "4^", //r
    "2-", //s
    "5^", //t
    "7^", //u
    "4v", //v
    "2^", //w
    "2v", //x
    "6^", //y
    "1v", //z
    "8v", //,
    "9v", //.
    "0v", ///
    "0-"  //;
};

ArrayContext* array_create_context() {
    ArrayContext *context = (ArrayContext*)g_malloc(sizeof(ArrayContext));

    if (sqlite3_open(ARRAY_DB_FILE_PATH, &(context->conn)) != SQLITE_OK) {
        context->conn = NULL;
    }
    return context;
}

void array_release_context(ArrayContext *context) {
    if (context-> conn != NULL) {
        sqlite3_close(context->conn);
    }

    g_free(context);
}

GString* array_get_preedit_string(GString *preedit) {
    GString *result = g_string_new("");

    int i;
    for (i = 0; i < preedit->len; i++) {
        gchar c = preedit->str[i];
        int index = -1;

        if (c >= 'a' && c <= 'z') {
            index = c - 'a';
        }
        else if (c == ',') {
            index = 26;
        }
        else if (c == '.') {
            index = 27;
        }
        else if (c == '/') {
            index = 28;
        }
        else if (c == ';') {
            index = 29;
        }

        if (index >= 0) {
            g_string_append(result, valid_key_map[index]);
        }
    }

    return result;
}

void array_release_candidates(GArray *candidates) {
    int i;
    for (i = 0; i < candidates->len; i++) {
        gchar *s = g_array_index(candidates, gchar*, i);
        g_free(s);
    }
    g_array_free(candidates, TRUE);
}

GArray* array_get_candidates_from_main(ArrayContext *context, gchar *keys) {
    GArray *result;
    result = (GArray*)g_array_new(FALSE, FALSE, sizeof(gchar*));

    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(context->conn, "SELECT ch FROM main WHERE keys=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys, -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            gchar *ch = (gchar*)sqlite3_column_text(stmt, 0);
            gchar *chstr = g_strdup(ch);
            g_array_append_val(result, chstr);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

GArray* array_get_candidates_from_simple(ArrayContext *context, gchar *keys) {
    GArray *result;
    result = (GArray*)g_array_new(FALSE, FALSE, sizeof(gchar*));

    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(context->conn, "SELECT ch FROM simple WHERE keys=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys, -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            gchar *ch = (gchar*)sqlite3_column_text(stmt, 0);
            gchar *chstr = g_strdup(ch);
            g_array_append_val(result, chstr);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

GArray* array_get_candidates_from_special(ArrayContext *context, gchar *keys) {
    GArray *result;
    result = (GArray*)g_array_new(FALSE, FALSE, sizeof(gchar*));

    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(context->conn, "SELECT ch FROM main WHERE cat='2' AND keys=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys, -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            gchar *ch = (gchar*)sqlite3_column_text(stmt, 0);
            gchar *chstr = g_strdup(ch);
            g_array_append_val(result, chstr);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

GArray* array_get_reverted_key_candidates_from_special(ArrayContext *context, gchar *ch) {
    GArray *result;
    result = (GArray*)g_array_new(FALSE, FALSE, sizeof(gchar*));

    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(context->conn, "SELECT keys FROM main WHERE cat='2' AND ch=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, ch, -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            gchar *keys = (gchar*)sqlite3_column_text(stmt, 0);
            gchar *keysstr = g_strdup(keys);
            g_array_append_val(result, keysstr);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

GArray* array_get_reverted_char_candidates_from_special(ArrayContext *context, gchar *keys) {
    GArray *result;
    result = (GArray*)g_array_new(FALSE, FALSE, sizeof(gchar*));

    sqlite3_stmt *stmt;

    int retcode;
    retcode = sqlite3_prepare_v2(context->conn, "SELECT ch FROM main WHERE cat='2' AND keys=?", -1, &stmt, NULL);
    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, keys, -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            gchar *ch = (gchar*)sqlite3_column_text(stmt, 0);
            gchar *chstr = g_strdup(ch);
            g_array_append_val(result, chstr);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    return result;
}

gboolean 
array_input_key_is_not_special(ArrayContext* context, 
                                const gchar* keys, const gchar* ch)
{
    sqlite3_stmt *stmt;

    int retcode;
    gboolean result = FALSE;
    gchar *special_keys = NULL;

    retcode = sqlite3_prepare_v2(context->conn, 
            "SELECT keys FROM special WHERE ch=?", -1, &stmt, NULL);

    if (retcode == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, ch, -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            gchar *ch = (gchar*)sqlite3_column_text(stmt, 0);
            special_keys = g_strdup(ch);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);

    if (special_keys == NULL)
        return FALSE;

    if (g_strcmp0(special_keys, keys) != 0) {
        result = TRUE;
    }

    g_free(special_keys);

    return result;
}
