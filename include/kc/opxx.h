/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: opxx.h,v 1.3 2001/04/14 15:14:38 tp Exp $
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

#ifndef __opxx_h
/* #define __opxx_h */

#ifdef GEN_DD
#define CODE_XX dd
#define _p_XXd _p_IXd
#else
#ifdef GEN_FD
#define CODE_XX fd
#define _p_XXd _p_IYd
#else
#error must define GEN_DD or GEN_FD!
#endif
#endif

#define name _name(CODE_XX)
#define _name(x) __name(opcodes_,x)
#define __name(x,y) x##y

static opcodes_t name[256] = {
	/* 0x00 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x10 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x20 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x30 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x40 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x50 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x60 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x70 */
	{"LD",		_p_XXd,		_r_B},
	{"LD",		_p_XXd,		_r_C},
	{"LD",		_p_XXd,		_r_D},
	{"LD",		_p_XXd,		_r_E},
	{"LD",		_p_XXd,		_r_H},
	{"LD",		_p_XXd,		_r_L},
	{"",     	_none,		_none},
	{"LD",		_p_XXd,		_r_A},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x80 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0x90 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0xa0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0xb0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0xc0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0xd0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0xe0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
	/* 0xf0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",        	_none,		_none},
};

#undef GEN_DD
#undef GEN_FD
#undef CODE_XX
#undef _p_XXd

#undef name
#undef _name
#undef __name

#endif /* __opdd_h */
