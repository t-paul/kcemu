/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "kc/config.h"
#include "sys/sysdep.h"

static char *
_sys_gethome(void) {
    char *buf;
    const char *ptr, *ptr2;

    ptr = getenv("APPDATA");
    if (ptr != NULL) {
        buf = malloc(strlen(ptr) + 7);
	strcpy(buf, ptr);
	strcat(buf, "\\KCemu");
        return strdup(ptr);
    }

    ptr = getenv("USERPROFILE");
    if (ptr != NULL)
        return strdup(ptr);

    ptr = getenv("HOMEDRIVE");
    if (ptr != NULL) {
        ptr2 = getenv("HOMEPATH");
	if (ptr2 != NULL) {
	    buf = malloc(strlen(ptr) + strlen(ptr2) + 1);
	    strcpy(buf, ptr);
	    strcat(buf, ptr2);
            return buf;
	}
    }
    
    ptr = getenv("HOME");
    if (ptr != NULL)
        return strdup(ptr);
    
    return NULL;
}

char *
sys_gethome(void) {
    int a;
    char *ptr;

    ptr = _sys_gethome();
    if (ptr == NULL)
        return NULL;

    for (a = 0;ptr[a] != 0;a++)
        if (ptr[a] == '\\')
            ptr[a] = '/';

    return ptr;
}
