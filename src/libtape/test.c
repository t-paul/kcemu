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
	int a;
	FILE *f;

	f = fopen("testfile.1", "wb");
	if (f) {
		for (a = 0;a < 512;a++) {
			fputc(a & 0xff, f);
		}
		fclose(f);
	}
	f = fopen("testfile.2", "wb");
	if (f) {
		for (a = 0;a < 512;a++) {
			fputc(((a >> 8) & 0xff) * 64 + 64, f);
		}
		fclose(f);
	}
	return 0;
}
