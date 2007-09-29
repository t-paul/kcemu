/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2002 Torsten Paul
 *
 *  $Id$
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libdisk/teledisk.h"

static int
decode_sector_copy(teledisk_prop_t *prop, int track_idx, unsigned char *buf)
{
  int a, c, size;

  size = prop->offset[track_idx].l;

  for (a = 0;a < size;a++)
    {
      c = fgetc(prop->f);
      if (c == EOF)
	return -1;

      buf[a] = c;
    }

  return 0;
}

static int
decode_sector_mult(teledisk_prop_t *prop, int track_idx, unsigned char *buf)
{
  int a, b1, b2, idx, len;

  len = fgetc(prop->f);
  len = len + 256 * fgetc(prop->f);

  b1 = fgetc(prop->f);
  b2 = fgetc(prop->f);

  idx = 0;
  for (a = 0;a < len;a++)
    {
      buf[idx++] = b1;
      buf[idx++] = b2;
    }

  return 0;
}

static int
decode_sector_rle(teledisk_prop_t *prop, int track_idx, unsigned char *buf)
{
  int a, c, code, b1, b2, len, idx;
  
  idx = 0;
  while (idx < prop->offset[track_idx].l)
    {
      code = fgetc(prop->f);
      switch (code)
	{
	case 0:
	  len = fgetc(prop->f);
	  
	  for (a = 0;a < len;a++)
	    {
	      c = fgetc(prop->f);
	      buf[idx++] = c;
	    }
	  break;
	case 1:
	  len = fgetc(prop->f);
	  b1 = fgetc(prop->f);
	  b2 = fgetc(prop->f);
	  for (a = 0;a < len;a++)
	    {
	      buf[idx++] = b1;
	      buf[idx++] = b2;

	    }
	  break;
	default:
	  return -1;
	}
    }

  return 0;
}

static int
decode_sector(teledisk_prop_t *prop, int track_idx, unsigned char *buf)
{
  int type;

  type = fgetc(prop->f);

  printf("track: %d, type = %d\n", track_idx, type);

  switch (type)
    {
    case 0x00:
      return decode_sector_copy(prop, track_idx, buf);
    case 0x01:
      return decode_sector_mult(prop, track_idx, buf);
    case 0x02:
      return decode_sector_rle(prop, track_idx, buf);
    default:
      break;
    }

  return -1;
}

int
read_sectors(teledisk_prop_t *prop)
{
  int o, t, s1, s2, s3, s4, s5, s6, s7, s8;

  o = ftell(prop->f);
  
  s1 = fgetc(prop->f);
  s2 = fgetc(prop->f);
  s3 = fgetc(prop->f);
  s4 = fgetc(prop->f);
  s5 = fgetc(prop->f);
  s6 = fgetc(prop->f);
  s7 = fgetc(prop->f);
  s8 = fgetc(prop->f);

  //printf("SECTOR: %02x %02x %02x %02x %02x %02x %02x %02x\n", s1, s2, s3, s4, s5, s6, s7, s8);

  t = prop->tracks;
  prop->tracks++;

  prop->offset[t].c = s1;
  prop->offset[t].h = s2;
  prop->offset[t].s = s3;
  switch (s4)
    {
    case 2:
      prop->offset[t].l = 512;
      break;
    case 3:
      prop->offset[t].l = 1024;
      break;
    default:
      return -1;
    }

  prop->offset[t].o = o;
  prop->offset[prop->tracks].o = 0;

  return decode_sector(prop, t, prop->buf);
}

static int
read_tracks(teledisk_prop_t *prop)
{
  int a, t1, t2, t3, t4;

  t1 = fgetc(prop->f);
  t2 = fgetc(prop->f);
  t3 = fgetc(prop->f);
  t4 = fgetc(prop->f);

  if (t1 == 0xff) // end marker?
    return 1;

  //printf("TRACK: %02x %02x %02x %02x\n", t1, t2, t3, t4);

  for (a = 0;a < t1;a++)
    if (read_sectors(prop) < 0)
      return -1;

  return 0;
}

static char *
read_header(FILE *f)
{
  char *buf;
  char *density;
  char *drive;
  int c, cnt, idx, len;
  struct {
    unsigned char  file_id[2];
    unsigned char  volume_sequence;
    unsigned char  check_signature;
    unsigned char  version_number;
    unsigned char  source_density;
    unsigned char  drive_type;
    unsigned char  track_density;
    unsigned char  dos_mode;
    unsigned char  sides;
    unsigned short crc;
  } h;

  struct {
    unsigned short crc;
    unsigned short len;
    unsigned char  year;
    unsigned char  month;
    unsigned char  day;
    unsigned char  hour;
    unsigned char  minute;
    unsigned char  second;
  } comment;

  const char *month[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  if (fread(&h, sizeof(h), 1, f) != 1)
    return NULL;

  /*
   *  check file identifier
   */
  if ((h.file_id[0] != 'T') && (h.file_id[0] != 't'))
    return NULL;

  if ((h.file_id[1] != 'D') && (h.file_id[1] != 'd'))
    return NULL;

  if (h.volume_sequence != 0)
    return NULL;

  // check for comment
  if ((h.track_density & 0x80) == 0)
    {
      buf = malloc(1);
      buf[0] = 0;
      return buf;
    }

  if (fread(&comment, sizeof(comment), 1, f) != 1)
    return NULL;

//printf("00: %02x, %3d\n", h.unknown00, h.unknown00);
//printf("01: %02x, %3d\n", h.unknown01, h.unknown01);
//printf("02: %02x, %3d\n", h.unknown02, h.unknown02);
//printf("03: %02x, %3d\n", h.unknown03, h.unknown03);
//printf("04: %02x, %3d\n", h.unknown04, h.unknown04);
//printf("05: %02x, %3d\n", h.unknown05, h.unknown05);
//printf("07: %02x, %3d\n", h.unknown07, h.unknown07);
//printf("08: %02x, %3d\n", h.unknown08, h.unknown08);
//printf("09: %02x, %3d\n", h.unknown09, h.unknown09);
//printf("10: %02x, %3d\n", h.unknown10, h.unknown10);
//printf("11: %02x, %3d\n", h.unknown11, h.unknown11);
//printf("12: %02x, %3d\n", h.unknown12, h.unknown12);
//

  switch (h.source_density)
    {
    case 0:
      density = "250K bps MFM";
      break;
    case 1:
      density = "300K bps MFM";
      break;
    case 2:
      density = "500K bps MFM";
      break;
    case 128:
      density = "250K bps FM";
      break;
    case 129:
      density = "300K bps FM";
      break;
    case 130:
      density = "500K bps FM";
      break;
    default:
      return NULL;
    }

  switch (h.drive_type)
    {
    case 1:
      drive = "360k";
      break;
    case 2:
      drive = "1.2M";
      break;
    case 3:
      drive = "720k";
      break;
    case 4:
      drive = "1.44k";
      break;
    default:
      return NULL;
    }

  printf("file_id: %c%c (vol %d, check %d)\n", h.file_id[0], h.file_id[1], h.volume_sequence, h.check_signature);
  printf("version_number: %d.%d\n", h.version_number / 16, h.version_number & 15);
  printf("density: %s\n", density);
  printf("drive_type: %s\n", drive);
  printf("sides: %d\n", h.sides);
  printf("dos_mode: %d\n", h.dos_mode);
  printf("comment length: %d\n", comment.len);
  printf("date: %02d. %s %04d, %02d:%02d:%02d\n",
	 comment.day, month[comment.month], comment.year + 1900,
	 comment.hour, comment.minute, comment.second);

  /*
   *  read comment
   */
  

  cnt = 0;
  idx = 0;
  len = 128;
  buf = (char *)malloc(comment.len);
  if (fread(buf, 1, comment.len, f) != comment.len)
    {
      free(buf);
      return NULL;
    }

  printf("comment: %s\n", buf);

  return buf;
}

teledisk_prop_t *
teledisk_open(const char *filename)
{
  int a;
  int ret;
  FILE *f;
  char *comment;
  teledisk_prop_t *prop;

  f = fopen(filename, "rb");
  if (f == NULL)
    return NULL;

  comment = read_header(f);
  if (comment == NULL)
    return NULL;

  prop = malloc(sizeof(teledisk_prop_t));
  if (prop == NULL)
    {
      free(comment);
      return NULL;
    }

  prop->f = f;
  prop->tracks = 0;
  prop->comment = comment;
  prop->filename = strdup(filename);

  for (a = 0;a < 160;a++)
    {
      ret = read_tracks(prop);
      if (ret > 0)
	break;
      if (ret < 0)
	{
	  teledisk_close(prop);
	  return NULL;
	}
    }

  return prop;
}

void
teledisk_close(teledisk_prop_t *prop)
{
  if (prop == NULL)
    return;

  free(prop->filename);
  if (prop->comment != NULL)
    free(prop->comment);

  free(prop);
  return;
}

int
teledisk_read_sector(teledisk_prop_t *prop, int c, int h, int s)
{
  int a;

  a = -1;
  while (242)
    {
      a++;
      if (prop->offset[a].o == 0)
	return -1;
      if (prop->offset[a].c != c)
	continue;
      if (prop->offset[a].h != h)
	continue;
      if (prop->offset[a].s != s)
	continue;
      break;
    }


  if (fseek(prop->f, prop->offset[a].o + 8, SEEK_SET) < 0)
    return -1;

  if (decode_sector(prop, a, prop->buf) < 0)
    return -1;

  return 0;
}
