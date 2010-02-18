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

#ifndef __disass_h
#define __disass_h

typedef enum {
	_none,
	_r_A, _r_B, _r_C, _r_D, _r_E, _r_H, _r_L,
	_r_BC, _r_DE, _r_HL, _r_SP, _r_AF,
	_p_BC, _p_DE, _p_HL, _p_SP,
        _p_IXd, _p_IYd, _p_nn,
	_i_n, _i_nn, _i_e,
	_x_0, _x_1, _x_2, _x_3, _x_4, _x_5, _x_6, _x_7,
        _x_00h, _x_08h,_x_10h, _x_18h, _x_20h, _x_28h, _x_30h, _x_38h
} arg_t;

typedef struct
{
	char *op;
	arg_t dest;
	arg_t src;
} opcodes_t;

typedef int (*opcode_func_t)(char *, int, int);

#endif /* __disass_h */
