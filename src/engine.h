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

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <ibus.h>

#define IBUS_TYPE_ARRAY_ENGINE	\
	(ibus_array_engine_get_type ())

GType   ibus_array_engine_get_type    (void);

void ibus_array_init (IBusBus *bus);
void ibus_array_exit (void);

#endif
