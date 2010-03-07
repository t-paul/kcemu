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

			case 0x0f: c = '-'; break;
			case 0x1f: c = '-'; break;
			case '{':  c = '�'; break;
			case '|':  c = '�'; break;
			case '}':  c = '�'; break;
			case ']':  c = '�'; break;
			case '~':  c = '�'; break;
		}
		fputc(c, stdout);
	}

	return 0;
}
