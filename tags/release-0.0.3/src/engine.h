/* vim:set et sts=4: */
#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_ARRAY_ENGINE	\
	(ibus_array_engine_get_type ())

GType   ibus_array_engine_get_type    (void);

void ibus_array_init (IBusBus *bus);
void ibus_array_exit (void);

#endif
