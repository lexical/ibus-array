/* vim:set et sts=4: */

#include "engine.h"
#include "array.h"

#define ARRAY_SHORT_CODE_EMPTY_STRING "⎔"

typedef struct _IBusArrayEngine IBusArrayEngine;
typedef struct _IBusArrayEngineClass IBusArrayEngineClass;

struct _IBusArrayEngine {
	IBusEngine parent;

    /* members */
    GString *preedit;
    gint cursor_pos;
    guint space_press_count;

    IBusLookupTable *table;
};

struct _IBusArrayEngineClass {
	IBusEngineClass parent;
};

/* functions prototype */
static void	ibus_array_engine_class_init	(IBusArrayEngineClass	*klass);
static void	ibus_array_engine_init		(IBusArrayEngine		*engine);
static void	ibus_array_engine_destroy		(IBusArrayEngine		*engine);
static gboolean 
			ibus_array_engine_process_key_event
                                            (IBusEngine             *engine,
                                             guint               	 keyval,
                                             guint                   keycode,
                                             guint               	 modifiers);
static void ibus_array_engine_focus_in    (IBusEngine             *engine);
static void ibus_array_engine_focus_out   (IBusEngine             *engine);
static void ibus_array_engine_reset       (IBusEngine             *engine);
static void ibus_array_engine_enable      (IBusEngine             *engine);
static void ibus_array_engine_disable     (IBusEngine             *engine);
static void ibus_engine_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_array_engine_set_capabilities
                                            (IBusEngine             *engine,
                                             guint                   caps);
static void ibus_array_engine_page_up     (IBusEngine             *engine);
static void ibus_array_engine_page_down   (IBusEngine             *engine);
static void ibus_array_engine_cursor_up   (IBusEngine             *engine);
static void ibus_array_engine_cursor_down (IBusEngine             *engine);
static void ibus_array_property_activate  (IBusEngine             *engine,
                                             const gchar            *prop_name,
                                             gint                    prop_state);
static void ibus_array_engine_property_show
											(IBusEngine             *engine,
                                             const gchar            *prop_name);
static void ibus_array_engine_property_hide
											(IBusEngine             *engine,
                                             const gchar            *prop_name);

static void ibus_array_engine_commit_string
                                            (IBusArrayEngine      *arrayeng,
                                             const gchar            *string);
static void ibus_array_engine_update      (IBusArrayEngine      *arrayeng);
static void ibus_array_engine_update_preedit (IBusArrayEngine *arrayeng);
static void ibus_array_engine_update_lookup_table (IBusArrayEngine *arrayeng);
static gboolean ibus_array_engine_process_candidate_key_event (IBusArrayEngine *arrayeng, guint keyval, guint modifiers);
static gboolean ibus_array_engine_commit_current_candidate (IBusArrayEngine *arrayeng);
static void ibus_array_engine_space_press(IBusArrayEngine *arrayeng);
static gboolean ibus_array_engine_update_symbol_lookup_table (IBusArrayEngine *arrayeng);

static IBusEngineClass *parent_class = NULL;

static ArrayContext *array_context = NULL;

GType
ibus_array_engine_get_type (void)
{
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof (IBusArrayEngineClass),
		(GBaseInitFunc)		NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc)	ibus_array_engine_class_init,
		NULL,
		NULL,
		sizeof (IBusArrayEngine),
		0,
		(GInstanceInitFunc)	ibus_array_engine_init,
	};

	if (type == 0) {
		type = g_type_register_static (IBUS_TYPE_ENGINE,
									   "IBusArrayEngine",
									   &type_info,
									   (GTypeFlags) 0);
	}

	return type;
}

void 
ibus_array_init (IBusBus *bus) 
{
    array_context = array_create_context();
}

void 
ibus_array_exit (void) 
{
    array_release_context(array_context);    
}

static void
ibus_array_engine_class_init (IBusArrayEngineClass *klass)
{
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

	parent_class = (IBusEngineClass *) g_type_class_peek_parent (klass);
	
	ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_array_engine_destroy;

    engine_class->process_key_event = ibus_array_engine_process_key_event;
    engine_class->reset = ibus_array_engine_reset;

    engine_class->page_up = ibus_array_engine_page_up;
    engine_class->page_down = ibus_array_engine_page_down;
}

static void
ibus_array_engine_init (IBusArrayEngine *arrayeng)
{
    arrayeng->preedit = g_string_new ("");
    arrayeng->cursor_pos = 0;
    arrayeng->space_press_count = 0;

    arrayeng->table = ibus_lookup_table_new (10, 0, FALSE, TRUE);
}

static void
ibus_array_engine_destroy (IBusArrayEngine *arrayeng)
{
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
    ibus_array_engine_update_preedit (arrayeng);
    ibus_engine_hide_lookup_table (engine);

    parent_class->reset(engine);
}

static void
ibus_array_engine_page_up (IBusEngine *engine)
{
        parent_class->page_up (engine);
}

static void
ibus_array_engine_page_down (IBusEngine *engine)
{
        parent_class->page_down (engine);
}



static void
ibus_array_engine_update_lookup_table (IBusArrayEngine *arrayeng)
{
    gint i;
    gboolean retval;

    if (arrayeng->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }

    ibus_lookup_table_clear (arrayeng->table);

    GArray *candidates = NULL;

    if (arrayeng->preedit->len <= 2 && arrayeng->space_press_count == 0) {
        candidates = array_get_candidates_from_short(array_context, arrayeng->preedit->str);
    }
    else {
        candidates = array_get_candidates_from_main(array_context, arrayeng->preedit->str);
    }
    
    if (candidates == NULL) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }
    else if (candidates->len == 0) {
        array_release_candidates(candidates);
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }

    for (i = 0; i < candidates->len; i++) {
        ibus_lookup_table_append_candidate (arrayeng->table, ibus_text_new_from_string (g_array_index(candidates, gchar*, i)));
    }

    array_release_candidates(candidates);

    ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);

}

static void
ibus_array_engine_update_preedit (IBusArrayEngine *arrayeng)
{
    IBusText *text;
    gint retval;

    GString *array_preedit = array_get_preedit_string(arrayeng->preedit);

    text = ibus_text_new_from_string(array_preedit->str);

    //text = ibus_text_new_from_static_string (arrayeng->preedit->str);
    text->attrs = ibus_attr_list_new ();

    ibus_attr_list_append (text->attrs,
                           ibus_attr_underline_new (IBUS_ATTR_UNDERLINE_SINGLE, 0, array_preedit->len));

    if (array_preedit->len > 0) {
        retval = 0;
        if (retval != 0) {
            ibus_attr_list_append (text->attrs,
                               ibus_attr_foreground_new (0xff0000, 0, array_preedit->len));
        }
    }
    
    ibus_engine_update_preedit_text ((IBusEngine *)arrayeng,
                                     text,
                                     array_preedit->len, //arrayeng->cursor_pos,
                                     TRUE);
    g_object_unref (text);

    g_string_free(array_preedit, TRUE);
}

static gboolean 
ibus_array_engine_update_symbol_lookup_table (IBusArrayEngine *arrayeng) 
{
    gint i;
    gboolean retval;

    if (arrayeng->preedit->len == 0) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }

    ibus_lookup_table_clear (arrayeng->table);

    GArray *candidates = NULL;

    candidates = array_get_candidates_from_main(array_context, arrayeng->preedit->str);
    
    if (candidates == NULL) {
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }
    else if (candidates->len == 0) {
        array_release_candidates(candidates);
        ibus_engine_hide_lookup_table ((IBusEngine *) arrayeng);
        return;
    }

    for (i = 0; i < candidates->len; i++) {
        ibus_lookup_table_append_candidate (arrayeng->table, ibus_text_new_from_string (g_array_index(candidates, gchar*, i)));
    }

    array_release_candidates(candidates);

    ibus_engine_update_lookup_table ((IBusEngine *) arrayeng, arrayeng->table, TRUE);
}

static gboolean  
ibus_array_engine_commit_current_candidate (IBusArrayEngine *arrayeng) {
    guint cursor_pos;
    const char* value;
    IBusText* text;

    cursor_pos = ibus_lookup_table_get_cursor_pos (arrayeng->table);
    text = ibus_lookup_table_get_candidate(arrayeng->table, cursor_pos);

    if (g_strcmp0(text->text, ARRAY_SHORT_CODE_EMPTY_STRING) != 0) {
        ibus_engine_commit_text((IBusEngine*)arrayeng, text);
        return TRUE;
    }

    return FALSE;
}

static void
ibus_array_engine_commit_string (IBusArrayEngine *arrayeng,
                                   const gchar       *string)
{
    IBusText *text;
    text = ibus_text_new_from_static_string (string);
    ibus_engine_commit_text ((IBusEngine *)arrayeng, text);
    g_object_unref (text);
}

static void
ibus_array_engine_update (IBusArrayEngine *arrayeng)
{
    ibus_array_engine_update_preedit (arrayeng);
    ibus_array_engine_update_lookup_table(arrayeng);
}

//#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z) || ((c) >= IBUS_A && (c) <= IBUS_Z))
#define is_alpha(c) (((c) >= IBUS_a && (c) <= IBUS_z))

static gboolean 
ibus_array_engine_process_key_event (IBusEngine *engine,
                                       guint       keyval,
                                       guint       keycode,
                                       guint       modifiers)
{
    IBusText *text;
    IBusArrayEngine *arrayeng = (IBusArrayEngine *)engine;

    if (modifiers & IBUS_RELEASE_MASK)
        return FALSE;

    //modifiers &= (IBUS_CONTROL_MASK | IBUS_MOD1_MASK);

    /*if (modifiers == IBUS_CONTROL_MASK && keyval == IBUS_s) {
        ibus_array_engine_update_lookup_table (arrayeng);
        return TRUE;
    }*/

    /*if (modifiers != 0) {
        if (arrayeng->preedit->len == 0)
            return FALSE;
        else
            return TRUE;
    }*/

    if (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R)
        return FALSE;

    if (modifiers & (IBUS_CONTROL_MASK | IBUS_MOD1_MASK))
        return FALSE;


    switch (keyval) {
    case IBUS_space:
        //g_string_append (arrayeng->preedit, " ");
        if (arrayeng->preedit->len == 0)
            return FALSE;
        ibus_array_engine_space_press(arrayeng);
        return TRUE;
        //return ibus_array_engine_commit_preedit (arrayeng);
    case IBUS_Return:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        else
            return TRUE;
        //return ibus_array_engine_commit_preedit (arrayeng);

    case IBUS_Escape:
        if (arrayeng->preedit->len == 0)
            return FALSE;
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
        /*if (arrayeng->cursor_pos < arrayeng->preedit->len) {
            arrayeng->cursor_pos ++;
            ibus_array_engine_update (arrayeng);
        }
        return TRUE;*/
    

    case IBUS_BackSpace:
    case IBUS_Delete:
        if (arrayeng->preedit->len == 0)
            return FALSE;
        if (arrayeng->cursor_pos > 0) {
            arrayeng->cursor_pos --;
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

    if (arrayeng->preedit->len >= 5) {
        return TRUE;
    }

    if (is_alpha (keyval) || keyval == IBUS_period || keyval == IBUS_comma || keyval == IBUS_slash || keyval == IBUS_semicolon) {

        if (arrayeng->space_press_count == 1) {
            if (arrayeng->table->candidates->len > 0) {
                gboolean commit_rev;

                ibus_lookup_table_set_cursor_pos (arrayeng->table, 0);

                commit_rev = ibus_array_engine_commit_current_candidate(arrayeng);

                if (commit_rev)
                    ibus_array_engine_reset((IBusEngine*)arrayeng);
            }
        }

        g_string_insert_c (arrayeng->preedit,
                           arrayeng->cursor_pos,
                           keyval);

        arrayeng->cursor_pos ++;
        ibus_array_engine_update (arrayeng);
        
        return TRUE;
    }

    return FALSE;
}

static gboolean
ibus_array_engine_process_candidate_key_event (IBusArrayEngine *arrayeng, 
                                                guint keyval, 
                                                guint modifiers) {
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

        if (commit_rev)
            ibus_array_engine_reset((IBusEngine*)arrayeng);
    }

    return TRUE;
}

static void ibus_array_engine_space_press(IBusArrayEngine *arrayeng) {
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

            if (commit_rev) {
                ibus_array_engine_reset((IBusEngine*)arrayeng);
            }
            else {
                ibus_engine_hide_lookup_table((IBusEngine*)arrayeng);
            }
        }
    }
    else if (arrayeng->space_press_count == 1) {
        gboolean commit_rev;

        if (arrayeng->table->candidates->len > 0) {

            ibus_lookup_table_set_cursor_pos (arrayeng->table, 0);

            commit_rev = ibus_array_engine_commit_current_candidate(arrayeng);

            if (commit_rev)
                ibus_array_engine_reset((IBusEngine*)arrayeng);
            else {
                ibus_engine_hide_lookup_table((IBusEngine*)arrayeng);
             }
        }
        else {
            ibus_engine_hide_lookup_table((IBusEngine*)arrayeng);
        }
    }
}
