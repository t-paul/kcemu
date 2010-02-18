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

int
check_file_type(char *name, char *type, unsigned char *buf)
{
  int a;
  const char *t = NULL;

  if ((buf[1] == 0xd3) && (buf[2] == 0xd3) && (buf[3] == 0xd3))
    {
      t = "BASIC";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[1] == 0xd4) && (buf[2] == 0xd4) && (buf[3] == 0xd4))
    {
      t = "DATA";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[1] == 0xd5) && (buf[2] == 0xd5) && (buf[3] == 0xd5))
    {
      t = "LIST";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[1] == 0xd7) && (buf[2] == 0xd7) && (buf[3] == 0xd7))
    {
      t = "BASIC*";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'C') && (buf[10] == 'O') && (buf[11] == 'M'))
    {
      t = "COM";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'A') && (buf[10] == 'S') && (buf[11] == 'M'))
    {
      t = "ASM";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'T') && (buf[10] == 'X') && (buf[11] == 'T'))
    {
      t = "TXT";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'P') && (buf[10] == 'A') && (buf[11] == 'S'))
    {
      t = "PAS";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'S') && (buf[10] == 'C') && (buf[11] == 'R'))
    {
      t = "SCR";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == '\0') && (buf[10] == '\0') && (buf[11] == '\0'))
    if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", buf[1]) != NULL)
      {
	t = "(none)";
	memcpy(name, &buf[1], 8);
	name[8] = '\0';
      }
  
  if (t == NULL)
    return 0;

  for (a = 7;(a > 0) && name[a] == ' ';a--)
    name[a] = '\0';

  strcpy(type, t);
  return 1;
}
