/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: opxxxx.h,v 1.3 2001/04/14 15:14:39 tp Exp $
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

#ifndef __opxxxx_h
/* #define __opxxxx_h */

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
#define _name(x) __name(opcodes_,x,_cb)
#define __name(x,y,z) x##y##z

static opcodes_t name[256] = {
	/* 0x00 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RLC",     	_p_XXd,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RRC",		_p_XXd,		_none},
	{"",        	_none,		_none},
	/* 0x10 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RL",     	_p_XXd,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RR",		_p_XXd,		_none},
	{"",        	_none,		_none},
	/* 0x20 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SLA",     	_p_XXd,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SRA",		_p_XXd,		_none},
	{"",        	_none,		_none},
	/* 0x30 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SLI",     	_p_XXd,		_none},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SRL",		_p_XXd,		_none},
	{"",        	_none,		_none},
	/* 0x40 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",     	_p_XXd,		_x_0},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",		_p_XXd,		_x_1},
	{"",        	_none,		_none},
	/* 0x50 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",     	_p_XXd,		_x_2},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",		_p_XXd,		_x_3},
	{"",        	_none,		_none},
	/* 0x60 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",     	_p_XXd,		_x_4},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",		_p_XXd,		_x_5},
	{"",        	_none,		_none},
	/* 0x70 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",     	_p_XXd,		_x_6},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"BIT",		_p_XXd,		_x_7},
	{"",        	_none,		_none},
	/* 0x80 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",     	_p_XXd,		_x_0},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",		_p_XXd,		_x_1},
	{"",        	_none,		_none},
	/* 0x90 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",     	_p_XXd,		_x_2},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",		_p_XXd,		_x_3},
	{"",        	_none,		_none},
	/* 0xa0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",     	_p_XXd,		_x_4},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",		_p_XXd,		_x_5},
	{"",        	_none,		_none},
	/* 0xb0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",     	_p_XXd,		_x_6},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"RES",		_p_XXd,		_x_7},
	{"",        	_none,		_none},
	/* 0xc0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",     	_p_XXd,		_x_0},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",		_p_XXd,		_x_1},
	{"",        	_none,		_none},
	/* 0xd0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",     	_p_XXd,		_x_2},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",		_p_XXd,		_x_3},
	{"",        	_none,		_none},
	/* 0xe0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",     	_p_XXd,		_x_4},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",		_p_XXd,		_x_5},
	{"",        	_none,		_none},
	/* 0xf0 */
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",     	_p_XXd,		_x_6},
	{"",		_none,		_none},
	{"",     	_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"",		_none,		_none},
	{"SET",		_p_XXd,		_x_7},
	{"",        	_none,		_none},
};

#undef GEN_DD
#undef GEN_FD
#undef CODE_XX
#undef _p_XXd

#undef name
#undef _name
#undef __name

#endif /* __opxxxx_h */
