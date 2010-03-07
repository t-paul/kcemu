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

void
dump_sector(int c, int h, int s)
{
	int a;

	putchar(c); // acyl
	putchar(h); // asid
	putchar(c); // lcyl
	putchar(h); // lsid
	putchar(s); // lsec
	putchar(3); // llen
	putchar(0); // count low
	putchar(4); // count high

	for (a = 0;a < 1024;a++)
		putchar(0xe5);
}

int
main(void)
{
	int c, h, s;

	for (c = 0;c < 80;c++)
		for (h = 0;h < 2;h++)
			for (s = 1;s < 6;s++)
				dump_sector(c, h, s);

	return 0;
}
