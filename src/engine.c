/*
 ibus-array - The Array 30 Engine for IBus

 Copyright (c) 2009-2018 Yu-Chun Wang <mainlander1122@gmail.com>
                         Keng-Yu Lin <kengyu@lexical.tw>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <locale.h>
#include <libintl.h>
#include "engine.h"
#include "array.h"
#include "config.h"

#define _(String) gettext(String)
#define ARRAY_SHORT_CODE_EMPTY_STRING "⎔"

typedef struct _IBusArrayEngine IBusArrayEngine;
typedef struct _IBusArrayEngineClass IBusArrayEngineClass;

struct _IBusArrayEngine {
    IBusEngine parent;

    /* members */
    GString *preedit;
    gint cursor_pos;
    guint space_press_count;
    guint wildcard_char_count;

    IBusLookupTable *table;
    IBusPropList *prop_list;
};

struct _IBusArrayEngineClass {
    IBusEngineClass parent;
};

/* functions prototype */
static void ibus_array_engine_class_init (IBusArrayEngineClass *klass);
static void ibus_array_engine_init (IBusArrayEngine *engine);
static void ibus_array_engine_destroy (IBusArrayEngine *engine);

static gboolean ibus_array_engine_process_key_event (IBusEngine *engine, guint keyval, guint keycode, guint modifiers);

static void ibus_array_engine_focus_in (IBusEngine *engine);
static void ibus_array_engine_focus_out (IBusEngine *engine);

static void ibus_array_engine_reset (IBusEngine *engine);
static void ibus_array_engine_enable (IBusEngine *engine);
static void ibus_array_engine_disable (IBusEngine *engine);

static void ibus_engine_set_cursor_location (IBusEngine *engine, gint x, gint y, gint w, gint h);
static void ibus_array_engine_set_capabilities (IBusEngine *engine, guint caps);

static void ibus_array_engine_page_up (IBusEngine *engine);
static void ibus_array_engine_page_down (IBusEngine *engine);
static void ibus_array_engine_cursor_up (IBusEngine *engine);
static void ibus_array_engine_cursor_down (IBusEngine *engine);

static void ibus_array_engine_property_activate (IBusEngine *engine, const gchar *prop_name, guint prop_state);
static void ibus_array_engine_property_show (IBusEngine *engine, const gchar *prop_name);
static void ibus_array_engine_property_hide (IBusEngine *engine, const gchar *prop_name);

static void ibus_array_engine_commit_string (IBusArrayEngine *arrayeng, const gchar *string);

static void ibus_array_engine_update (IBusArrayEngine *arrayeng);
static void ibus_array_engine_update_preedit (IBusArrayEngine *arrayeng);
static void ibus_array_engine_update_lookup_table (IBusArrayEngine *arrayeng);

static gboolean ibus_array_engine_process_candidate_key_event (IBusArrayEngine *arrayeng, guint keyval, guint modifiers);
static gboolean ibus_array_engine_commit_current_candidate (IBusArrayEngine *arrayeng);

static void ibus_array_engine_space_press(IBusArrayEngine *arrayeng);

static gboolean ibus_array_engine_update_symbol_lookup_table (IBusArrayEngine *arrayeng);

static void ibus_array_engine_update_auxiliary_text (IBusArrayEngine *arrayeng, gchar* aux_string);

static void ibus_array_engine_show_special_code(IBusArrayEngine *arrayeng);
static void ibus_array_engine_show_special_code_for_char(IBusArrayEngine *arrayeng, gchar *ch);

static void ibus_config_value_changed_cb (IBusConfig *config, const gchar *section, const gchar *name, GVariant *value, gpointer unused);

static IBusEngineClass *parent_class = NULL;
static IBusConfig *config = NULL;
static gboolean is_special_notify;
static gboolean is_special_only;
static gboolean is_aux_shown = FALSE;
static ArrayContext *array_context = NULL;

GType ibus_array_engine_get_type (void) {
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof (IBusArrayEngineClass),
		(GBaseInitFunc)	NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) ibus_array_engine_class_init,
		NULL,
		NULL,
		sizeof (IBusArrayEngine),
		0,
		(GInstanceInitFunc) ibus_array_engine_init,
	};

	if (type == 0)
		type = g_type_register_static (IBUS_TYPE_ENGINE, "IBusArrayEngine", &type_info, (GTypeFlags) 0);

	return type;
}

void ibus_array_init (IBusBus *bus) {
    gboolean res;

    array_context = array_create_context();

    config = ibus_bus_get_config (bus);
    if (config)
        g_object_ref_sink (config);

    is_special_notify = FALSE;
    is_special_only = FALSE;

    /* load config */
    GVariant* value;

    value = ibus_config_get_value (config, "engine/Array", "SpecialNotify");
    if (value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN)
        is_special_notify = g_variant_get_boolean(value);

    value = ibus_config_get_value (config, "engine/Array", "SpecialOnly");
    if (value && g_variant_classify(value) == G_VARIANT_CLASS_BOOLEAN)
            is_special_only = g_variant_get_boolean(value);

    /* gettext preparation */
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
}

void ibus_array_exit (void) 
{
    array_release_context(array_context);    

    if (g_object_is_floating (config))
        g_object_unref(config);
}

static void ibus_array_engine_class_init (IBusArrayEngineClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

    parent_class = (IBusEngineClass *) g_type_class_peek_parent (klass);
	
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_array_engine_destroy;

    engine_class->process_key_event = ibus_array_engine_process_key_event;
    engine_class->reset = ibus_array_engine_reset;

    engine_class->page_up = ibus_array_engine_page_up;
    engine_class->page_down = ibus_array_engine_page_down;

    engine_class->focus_in = ibus_array_engine_focus_in;
    engine_class->focus_out = ibus_array_engine_focus_out;

    engine_class->property_activate = ibus_array_engine_property_activate;
}

static void ibus_array_engine_init (IBusArrayEngine *arrayeng)
{
    IBusProperty *setup_prop;
    IBusText *setup_label;
    IBusText *setup_tooltip;

    arrayeng->preedit = g_string_new ("");
    arrayeng->cursor_pos = 0;
    arrayeng->space_press_count = 0;
    arrayeng->wildcard_char_count = 0;

    arrayeng->table = ibus_lookup_table_new (10, 0, FALSE, TRUE);
    g_object_ref_sink (arrayeng->table);
    setup_label = ibus_text_new_from_string (_("Setup"));
    setup_tooltip = ibus_text_new_from_string (_("Configure Array 30 engine"));
    setup_prop = ibus_property_new("setup", PROP_TYPE_NORMAL, setup_label, "gtk-preferences", setup_tooltip, TRUE, TRUE, 0, NULL);
    g_object_ref_sink (setup_prop);

    arrayeng->prop_list = ibus_prop_list_new();
    g_object_ref_sink (arrayeng->prop_list);

    ibus_prop_list_append(arrayeng->prop_list, setup_prop);

    g_signal_connect (config, "value-changed", G_CALLBACK(ibus_config_value_changed_cb), NULL);
}

static void ibus_array_engine_destroy (IBusArrayEngine *arrayeng) {
    if (arrayeng->prop_list) {
        g_object_unref(arrayeng->prop_list);
        arrayeng->prop_list = NULL;
    }

    if (arrayeng->preedit) {
        g_string_free (arrayeng->preedit, TRUE);
        arrayeng->preedit = NULL;
    }

    if (arrayeng->table) {
        g_object_unref (arrayeng->table);
        arrayeng->table = NULL;
    }

    IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)arrayeng);
}

static void ibus_array_engine_reset(IBusEngine *engine) {
    IBusArrayEngine *arrayeng = (IBusArrayEngine*)engine;
    g_string_assign (arrayeng->preedit, "");
    arrayeng->cursor_pos = 0;
    arrayeng->space_press_count = 0;
    arrayeng->wildcard_char_count = 0;

    ibus_engine_hide_preedit_text (engine);
    ibus_engine_hide_lookup_table (engine);
    ibus_engine_hide_auxiliary_text (engine);
    parent_class->reset(engine);
}

static void ibus_array_engine_page_up (IBusEngine *engine) {
    parent_class->page_up (engine);
}

static void ibus_array_engine_page_down (IBusEngine *engine) {
    parent_class->page_down (engine);
}

static void ibus_array_engine_focus_in (IBusEngine *engine) {
    IBusArrayEngine *arrayeng = (IBusArrayEngine*)engine;
    ibus_engine_register_properties (engine, arrayeng->prop_list);
    parent_class->focus_in (engine);
}

static void ibus_array_engine_focus_out (IBusEngine *engine) {
    parent_class->focus_out (engine);
}

static void ibus_array_engine_update_lookup_table (IBusArrayEngine *arrayeng) {
    gint i;
    gboolean retval;

    if (arrayeng->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }

    ibus_lookup_table_clear (arrayeng->table);

    GArray *candidates = NULL;

    if (arrayeng->preedit->len <= 2 && arrayeng->space_press_count == 0)
        candidates = array_get_candidates_from_simple(array_context, arrayeng->preedit->str);
    else
        candidates = array_get_candidates_from_main(array_context, arrayeng->preedit->str, arrayeng->wildcard_char_count);
    
    if (candidates == NULL) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }
    else if (candidates->len == 0) {
        array_release_candidates(candidates);
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }

    for (i = 0; i < candidates->len; i++)
        ibus_lookup_table_append_candidate (arrayeng->table, ibus_text_new_from_string (g_array_index(candidates, gchar*, i)));

    array_release_candidates(candidates);

    ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);

}

static void ibus_array_engine_update_preedit (IBusArrayEngine *arrayeng) {
    IBusText *text;
    gint retval;

    GString *array_preedit = array_get_preedit_string(arrayeng->preedit);

    text = ibus_text_new_from_string(array_preedit->str);

    text->attrs = ibus_attr_list_new ();

    ibus_attr_list_append (text->attrs, ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, array_preedit->len));

    if (array_preedit->len > 0) {
        retval = 0;
        if (retval != 0)
            ibus_attr_list_append (text->attrs, ibus_attr_foreground_new (0xff0000, 0, array_preedit->len));
    }
    
    ibus_engine_update_preedit_text ((IBusEngine *)arrayeng, text, array_preedit->len, TRUE);

#if IBUS_CHECK_VERSION (1, 3, 0)
#else
    if (G_IS_OBJECT (text) && g_object_is_floating (text))
    	g_object_unref (text);
#endif

    g_string_free(array_preedit, TRUE);
}

static gboolean ibus_array_engine_update_symbol_lookup_table (IBusArrayEngine *arrayeng) 
{
    gint i;
    gboolean retval;

    if (arrayeng->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return FALSE;
    }

    ibus_lookup_table_clear (arrayeng->table);

    GArray *candidates = NULL;

    candidates = array_get_candidates_from_main(array_context, arrayeng->preedit->str, arrayeng->wildcard_char_count);
    
    if (candidates == NULL) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return FALSE;
    }
    else if (candidates->len == 0) {
        array_release_candidates(candidates);
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return FALSE;
    }

    for (i = 0; i < candidates->len; i++)
        ibus_lookup_table_append_candidate (arrayeng->table, ibus_text_new_from_string (g_array_index(candidates, gchar*, i)));

    array_release_candidates(candidates);

    ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);
    return TRUE;
}

static gboolean ibus_array_engine_commit_current_candidate (IBusArrayEngine *arrayeng) {
    guint cursor_pos;
    const char* value;
    gchar *temptext;
    IBusText* text;
    gboolean check_special = FALSE;

    cursor_pos = ibus_lookup_table_get_cursor_pos (arrayeng->table);
    text = ibus_lookup_table_get_candidate(arrayeng->table, cursor_pos);

    if (g_strcmp0(text->text, ARRAY_SHORT_CODE_EMPTY_STRING) != 0) {
        temptext = g_strdup(text->text);

        if (is_special_only || is_special_notify) {
            check_special = array_input_key_is_not_special (array_context, arrayeng->preedit->str, text->text);

            if (check_special) {
                if (is_special_notify)
                    ibus_array_engine_show_special_code_for_char (arrayeng, text->text);

                if (is_special_only)
                    return FALSE;
            }
        }
        ibus_engine_commit_text((IBusEngine*)arrayeng, text);
        
        ibus_array_engine_reset((IBusEngine*)arrayeng);

        if (is_special_notify && check_special)
            ibus_array_engine_show_special_code_for_char(arrayeng, temptext);

        g_free(temptext);

        return TRUE;
    }

    ibus_engine_hide_lookup_table((IBusEngine*)arrayeng);

    return FALSE;
}

static void ibus_array_engine_commit_string (IBusArrayEngine *arrayeng, const gchar*string) {
    IBusText *text;
    text = ibus_text_new_from_static_string (string);
    ibus_engine_commit_text ((IBusEngine *)arrayeng, text);
#if IBUS_CHECK_VERSION (1, 3, 0)
#else
    if (g_object_is_floating (text))
    	g_object_unref (text);
#endif
}

static void ibus_array_engine_update (IBusArrayEngine *arrayeng) {
    ibus_array_engine_update_preedit (arrayeng);
    ibus_array_engine_update_lookup_table(arrayeng);
    ibus_array_engine_show_special_code(arrayeng);
}

#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z))
#define is_root(c)  ((is_alpha (c)  || (c) == IBUS_period || (c) == IBUS_comma || (c) == IBUS_slash || (c) == IBUS_semicolon))
#define is_wildcard(c) (((c) == IBUS_question))

static gboolean  ibus_array_engine_process_key_event (IBusEngine *engine, guint keyval, guint keycode, guint modifiers) {
    IBusText *text;
    IBusArrayEngine *arrayeng = (IBusArrayEngine *)engine;

    if (g_strcmp0(arrayeng->preedit->str, "w") == 0) {
        ibus_array_engine_update_auxiliary_text(arrayeng, _("1.comma 2.bracket 3.symbol 4.math 5.arrow 6.unit 7.table 8.roman 9.greek 0.bopomo"));
        is_aux_shown = TRUE;
    }
    else if (is_aux_shown) {
        ibus_array_engine_update_auxiliary_text(arrayeng, "");
        ibus_engine_hide_auxiliary_text((IBusEngine*)arrayeng);
        is_aux_shown = FALSE;
    }

    if (modifiers & IBUS_RELEASE_MASK)
        return FALSE;

    if (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R)
        return FALSE;

    if (modifiers & (IBUS_CONTROL_MASK | IBUS_MOD1_MASK))
        return FALSE;


    switch (keyval) {
    case IBUS_space:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        ibus_array_engine_space_press(arrayeng);
        return TRUE;

    case IBUS_Return:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        else
            return TRUE;

    case IBUS_Escape:
        ibus_array_engine_reset((IBusEngine*)arrayeng);
        return TRUE;        

    case IBUS_Left:
    case IBUS_Up:
    case IBUS_Page_Up:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        ibus_lookup_table_page_up (arrayeng->table);
        ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);
        return TRUE;

    case IBUS_Right:
    case IBUS_Down:
    case IBUS_Page_Down:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        ibus_lookup_table_page_down (arrayeng->table);
        ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);
        return TRUE;

    case IBUS_BackSpace:
    case IBUS_Delete:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        if (arrayeng->cursor_pos > 0) {
            arrayeng->cursor_pos-- ;
            if (is_wildcard (arrayeng->preedit->str[arrayeng->cursor_pos]))
		arrayeng->wildcard_char_count --;
            g_string_erase (arrayeng->preedit, arrayeng->cursor_pos, 1);
            ibus_array_engine_update (arrayeng);
        }
        return TRUE; 
    }

    if (arrayeng->preedit->len > 0 && (keyval >= IBUS_0 && keyval <= IBUS_9)) {
        if (g_strcmp0(arrayeng->preedit->str, "w") == 0) {
            g_string_insert_c (arrayeng->preedit,
                               arrayeng->cursor_pos,
                               keyval);
            arrayeng->cursor_pos ++;
            ibus_array_engine_update_symbol_lookup_table (arrayeng);
            return TRUE;
        }
        return ibus_array_engine_process_candidate_key_event(arrayeng, keyval, modifiers);
    }

    if (is_root (keyval) || is_wildcard (keyval)) {
        if (arrayeng->space_press_count == 1)
            if (arrayeng->table->candidates->len > 0) {
                gboolean commit_rev;

                ibus_lookup_table_set_cursor_pos (arrayeng->table, 0);

                commit_rev = ibus_array_engine_commit_current_candidate(arrayeng);
            } else {
		ibus_array_engine_reset((IBusEngine*)arrayeng);
            }

        if (arrayeng->preedit->len >= 5)
                return TRUE;

	if (is_wildcard (keyval))
            arrayeng->wildcard_char_count ++;

        g_string_insert_c (arrayeng->preedit, arrayeng->cursor_pos, keyval);

        arrayeng->cursor_pos ++;
        ibus_array_engine_update (arrayeng);
        
        return TRUE;
    }

    return FALSE;
}

static gboolean ibus_array_engine_process_candidate_key_event (IBusArrayEngine *arrayeng, guint keyval, guint modifiers) {
    if (keyval >= IBUS_0 && keyval <= IBUS_9) {
        guint page_no;
        guint page_size;
        guint cursor_pos;
        gboolean commit_rev;

        page_size = ibus_lookup_table_get_page_size (arrayeng->table);
        cursor_pos = ibus_lookup_table_get_cursor_pos (arrayeng->table);
        page_no = cursor_pos / page_size;

        if (keyval >= IBUS_1)
            cursor_pos = page_no * page_size + (keyval - IBUS_1);
        else
            cursor_pos = page_no * page_size + 9;

        if (cursor_pos >= arrayeng->table->candidates->len) 
            return TRUE;

        ibus_lookup_table_set_cursor_pos (arrayeng->table, cursor_pos);

        commit_rev = ibus_array_engine_commit_current_candidate(arrayeng);
    }

    return TRUE;
}

static void ibus_array_engine_space_press (IBusArrayEngine *arrayeng)
{
    guint page_size;
    page_size = ibus_lookup_table_get_page_size (arrayeng->table);
    if (arrayeng->table->candidates->len > page_size) {
        ibus_lookup_table_page_down (arrayeng->table);
        ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);
        return;
    }
    
    if (arrayeng->space_press_count == 0) {
        arrayeng->space_press_count ++;

        ibus_array_engine_update (arrayeng);

        if (arrayeng->table->candidates->len == 1) {
            gboolean commit_rev;

            ibus_lookup_table_set_cursor_pos(arrayeng->table, 0);
            commit_rev = ibus_array_engine_commit_current_candidate(arrayeng);
        }
    }
    else if (arrayeng->space_press_count == 1) {
        gboolean commit_rev;

        if (arrayeng->table->candidates->len > 0) {
            ibus_lookup_table_set_cursor_pos (arrayeng->table, 0);
            commit_rev = ibus_array_engine_commit_current_candidate(arrayeng);
        }
        else
            ibus_array_engine_reset((IBusEngine*)arrayeng);
    }
}

static void ibus_array_engine_update_auxiliary_text(IBusArrayEngine *arrayeng, gchar* aux_string)
{
    IBusText *text;

    if (aux_string) {
        text = ibus_text_new_from_string(aux_string);
        ibus_engine_update_auxiliary_text((IBusEngine*)arrayeng, text, TRUE);
    }
}

static void ibus_array_engine_show_special_code(IBusArrayEngine *arrayeng)
{
    if (!is_special_notify)
        return;

    if (arrayeng->preedit->len != 2) {
        ibus_engine_hide_auxiliary_text((IBusEngine*)arrayeng);
        return;
    }

    GArray* candidates = array_get_reverted_char_candidates_from_special(array_context, arrayeng->preedit->str);

    if (candidates->len == 1) {
        gchar* sch = g_array_index(candidates, gchar*, 0);
        GString* keystr = array_get_preedit_string(arrayeng->preedit);
        gchar* show_str = g_strdup_printf("%s: %s", sch, keystr->str);

        ibus_array_engine_update_auxiliary_text(arrayeng, show_str);

        g_string_free(keystr, TRUE);
        g_free(show_str);
    }
    else
        ibus_engine_hide_auxiliary_text((IBusEngine*)arrayeng);

    array_release_candidates(candidates);
}

static void ibus_array_engine_show_special_code_for_char (IBusArrayEngine *arrayeng, gchar *ch) {
    if (!is_special_notify)
        return;

    GArray* candidates = array_get_reverted_key_candidates_from_special(array_context, ch);
    if (candidates->len == 1) {
        gchar* rawkeys = g_array_index(candidates, gchar*, 0);
        GString *rawkeystr = g_string_new(rawkeys);
        GString* keystr = array_get_preedit_string(rawkeystr);
        gchar* show_str = g_strdup_printf("%s: %s", ch, keystr->str);

        ibus_array_engine_update_auxiliary_text(arrayeng, show_str);

        g_string_free(rawkeystr, TRUE);
        g_string_free(keystr, TRUE);
        g_free(show_str);
    }
    else
        ibus_engine_hide_auxiliary_text((IBusEngine*)arrayeng);

    array_release_candidates(candidates);
}

static void ibus_array_engine_property_activate (IBusEngine *engine, const gchar *prop_name, guint prop_state) {
    if (g_strcmp0(prop_name, "setup") == 0) {
        GError *error = NULL;
        gchar *argv[2] = { NULL, };
        gchar *path;
        const char* libexecdir;

        libexecdir = g_getenv("LIBEXECDIR");
        if (libexecdir == NULL)
            libexecdir = LIBEXECDIR;

        path = g_build_filename(libexecdir, "ibus-setup-array", NULL);
        argv[0] = path;
        argv[1] = NULL;
        g_spawn_async (NULL, argv, NULL, 0, NULL, NULL, NULL, &error);

        g_free(path);
    }
}

static void ibus_config_value_changed_cb (IBusConfig *config, const gchar *section,  const gchar *name, GVariant *value, gpointer unused) {
    if (g_strcmp0(section, "engine/array") == 0)
        if (g_strcmp0(name, "specialnotify") == 0)
            is_special_notify = g_variant_get_boolean (value);
        else if (g_strcmp0(name, "specialonly") == 0)
            is_special_only = g_variant_get_boolean (value);
}

