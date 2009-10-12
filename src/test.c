#include "array.h"

#include <stdio.h>

int main() {
    ArrayContext *context;

    context = array_create_context();

    if (context->conn == NULL)
        printf("Load database failed.\n");

    GArray *chs = array_get_candidates_from_main(context, "pzax");
    
    int i;
    for (i = 0; i < chs->len; i++) {
        gchar *s = g_array_index(chs, gchar*, i);
        printf("%s\n", s);
    }

    array_release_candidates(chs);

    chs = array_get_candidates_from_short(context, "vi");
    for (i = 0; i < chs->len; i++) {
        gchar *s = g_array_index(chs, gchar*, i);
        printf("%s\n", s);
    }
    array_release_candidates(chs);


    chs = array_get_candidates_from_special(context, "lo");
    for (i = 0; i < chs->len; i++) {
        gchar *s = g_array_index(chs, gchar*, i);
        printf("%s\n", s);
    }
    array_release_candidates(chs);

    chs = array_get_reverted_candidates_from_special(context, "是");
    for (i = 0; i < chs->len; i++) {
        gchar *s = g_array_index(chs, gchar*, i);
        printf("%s\n", s);
    }
    array_release_candidates(chs);

    GString *preedit = g_string_new(",./;");
    GString *kp = array_get_preedit_string(preedit);
    printf("%s\n", kp->str);
    g_string_free(kp, TRUE);
    g_string_free(preedit, TRUE);

    array_release_context(context);

    return 0;
}
