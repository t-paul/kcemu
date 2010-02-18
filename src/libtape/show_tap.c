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
#include <string.h>
#include <stdlib.h>

static int nr = 0;

void
list_file(FILE *f, const char *name)
{
	FILE *of;
	char oname[100];
	const char *fname;
	int block, nr_blocks;
	unsigned char buf[129], kcname[12];
	unsigned short addr, start, autostart;
  
	fname = strrchr(name, '/');
	if (fname)
		fname++;
	else
		fname = name;
	
	nr_blocks = 0;
	while (242) {
		if (fread(buf, 1, 129, f) != 129) {
			if (nr_blocks > 0) {
				printf("read error\n");
			}
			return;
		}
		nr_blocks++;
		block = buf[0];
		if (block == 1) {
			addr = buf[18] | (buf[19] << 8);
			start = buf[22] | (buf[23] << 8);
			autostart = (buf[17] > 2);
			memcpy(kcname, &buf[1], 11);
			kcname[11] = '\0';

			printf("%-40s ", fname);
			printf("%-13s ", kcname);
			printf("%04x  ", addr);
			if (!autostart)
				printf("%04x ", start);
			else
				printf("-    ");
			sprintf(oname, "out.%04d.img", nr++);
			of = fopen(oname, "wb");
		}
		if (of) {
			fwrite(&buf[1], 1, 128, of);
		}
		if (block == 0xff) {
			printf("%d blocks\n", nr_blocks);
			nr_blocks = 0;
			if (of) {
				fclose(of);
				of = NULL;
			}
			if (fread(buf, 1, 16, f) != 16) {
				return;
			}
		}
	}
}

int
main(int argc, char **argv)
{
	int a;
	FILE *f;
	char buf[16];
	
	printf("FILENAME                                 KC-NAME       "
	       "ADDR START LENGTH\n");
	for (a = 1;a < argc;a++) {
		f = fopen(argv[a], "rb");
		if (!f)
			continue;
		if (fread(buf, 1, 16, f) != 16) {
			fclose(f);
			exit(2);
		}
		if (strncmp((const char *)&buf[1], "KC-TAPE by AF", 13) != 0) {
			fclose(f);
			exit(3);
		}
		list_file(f, argv[a]);
		fclose(f);
	}
	
	return 0;
}
