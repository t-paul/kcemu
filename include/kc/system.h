/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: system.h,v 1.8 2002/06/09 14:24:32 torsten_paul Exp $
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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
