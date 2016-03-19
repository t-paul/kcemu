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

#include <stdio.h>

int
main(void)
{
	int c;

	while (242) {
		c = fgetc(stdin);
		if (c == EOF)
			break;
		if (c == 0x1a)
			break;

		c = c & 0x7f;

		switch (c) {
			case 0x02: continue;
			case 0x05: continue;
			case 0x11: continue;
			case 0x12: continue;
			case 0x19: continue;
			case 0x1e: continue;

			case 0x0f: c = '-';  break;
			case 0x1f: c = '-';  break;

                        // encoding: latin1
			case 0x5d: c = 0xc4; break; // [ -> Ä
			case 0x5e: c = 0xd6; break; // \ -> Ö
			case 0x5f: c = 0xdc; break; // ] -> Ü
			case 0x7b: c = 0xe4; break; // { -> ä
			case 0x7c: c = 0xf6; break; // | -> ö
			case 0x7d: c = 0xfc; break; // } -> ü
			case 0x7e: c = 0xdf; break; // ~ -> ß
		}
		fputc(c, stdout);
	}

	return 0;
}
