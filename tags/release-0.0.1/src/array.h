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
GArray* array_get_candidates_from_short(ArrayContext *context, gchar *keys);
GArray* array_get_candidates_from_special(ArrayContext *context, gchar *keys);
GArray* array_get_reverted_candidates_from_special(ArrayContext *context, gchar *ch);

#endif
