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

#ifndef __kc_system_h
#define __kc_system_h

#include "kc/config.h"

#ifdef ENABLE_NLS
# ifdef USE_INCLUDED_LIBINTL
#  include "../../intl/libintl.h"
# else
# include <libintl.h>
# endif
# define _(x) (gettext(x))
# ifdef gettext_noop
#  define N_(x) gettext_noop(x)
# else
#  define N_(x) (x)
# endif
#else
# define _(x) (x)
# define N_(x) x
#endif /* ENABLE_NLS */

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long dword_t;

#endif /* __kc_system_h */
