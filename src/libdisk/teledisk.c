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
  char *name;

  type = fgetc(prop->f);

  switch (type)
    {
    case 0x00: name = "copy"; break;
    case 0x01: name = "mult"; break;
    case 0x02: name = "rle"; break;
    default: name = "unknown"; break;
    }
      
  //printf("track: %d, type = %d (%s)\n", track_idx, type, name);

  memset(prop->buf, 0, sizeof(prop->buf));
  
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

static int
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

  t = prop->sectors_total;
  prop->sectors_total++;

  prop->offset[t].c = s1;
  prop->offset[t].h = s2;
  prop->offset[t].s = s3;
  switch (s4)
    {
    case 0:
      prop->offset[t].l = 128;
      break;
    case 1:
      prop->offset[t].l = 256;
      break;
    case 2:
      prop->offset[t].l = 512;
      break;
    case 3:
      prop->offset[t].l = 1024;
      break;
    default:
      return -1;
    }

  if (prop->sector_size == 0)
    prop->sector_size = prop->offset[t].l;
  else if (prop->sector_size != prop->offset[t].l)
    prop->sector_size = -1;

  if (prop->cylinders < (s1 + 1))
    prop->cylinders = s1 + 1;

  if (prop->sectors < s3)
    prop->sectors = s3;

  prop->offset[t].o = o;
  prop->offset[prop->sectors_total].o = 0;

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

  //printf("TRACK: %02x %02x %02x %02x\n", t1, t2, t3, t4);

  if (t1 == 0xff) // end marker?
    {
      //printf("TRACK END MARKER?\n");
      return 1;
    }

  for (a = 0;a < t1;a++)
    if (read_sectors(prop) < 0)
      return -1;

  return 0;
}

static int
set_density(teledisk_prop_t *prop, int density)
{
  switch (density)
    {
    case 0:
      prop->density = "250K bps MFM";
      break;
    case 1:
      prop->density = "300K bps MFM";
      break;
    case 2:
      prop->density = "500K bps MFM";
      break;
    case 128:
      prop->density = "250K bps FM";
      break;
    case 129:
      prop->density = "300K bps FM";
      break;
    case 130:
      prop->density = "500K bps FM";
      break;
    default:
      return 0;
    }

  return 1;
}

static int
set_drive_type(teledisk_prop_t *prop, int drive_type)
{
  switch (drive_type)
    {
    case 1:
      prop->drive_type = "360k";
      break;
    case 2:
      prop->drive_type = "1.2M";
      break;
    case 3:
      prop->drive_type = "720k";
      break;
    case 4:
      prop->drive_type = "1.44k";
      break;
    default:
      return 0;
    }

  return 1;
}

static int
read_header(teledisk_prop_t *prop)
{
  char buf[2048];
  struct {
    unsigned char  file_id[2];
    unsigned char  volume_sequence;
    unsigned char  check_signature;
    unsigned char  version_number;
    unsigned char  source_density;
    unsigned char  drive_type;
    unsigned char  track_density;
    unsigned char  dos_mode;
    unsigned char  heads;
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

  if (fread(&h, sizeof(h), 1, prop->f) != 1)
    return 0;

  /*
   *  check file identifier
   */
  if ((h.file_id[0] != 'T') && (h.file_id[0] != 't'))
    return 0;

  if ((h.file_id[1] != 'D') && (h.file_id[1] != 'd'))
    return 0;

  if (h.volume_sequence != 0)
    return 0;

  prop->id[0] = h.file_id[0];
  prop->id[1] = h.file_id[1];
  prop->id[2] = 0;
  prop->volume = h.volume_sequence;
  prop->signature = h.check_signature;
  prop->version_major = h.version_number / 16;
  prop->version_minor = h.version_number & 15;
  prop->heads = h.heads;
  prop->dos_mode = h.dos_mode;

  if (!set_density(prop, h.source_density))
    return 0;

  if (!set_drive_type(prop, h.drive_type))
    return 0;

  // check for comment
  if ((h.track_density & 0x80) != 0)
    {
      // comment header
      if (fread(&comment, sizeof(comment), 1, prop->f) != 1)
        return 0;

      snprintf(buf, sizeof(buf), "%02d. %s %04d, %02d:%02d:%02d",
             comment.day, month[comment.month], comment.year + 1900,
             comment.hour, comment.minute, comment.second);
      prop->comment_date = strdup(buf);

      if (comment.len >= (sizeof(buf) - 1))
        return 0;
      
      /*
       *  read comment
       */
      memset(buf, 0, sizeof(buf));
      if (fread(buf, 1, comment.len, prop->f) != comment.len)
        return 0;

      printf("comment: %s\n", buf);
      prop->comment = strdup(buf);
    }

  return 1;
}

teledisk_prop_t *
teledisk_open(const char *filename)
{
  int ret;
  FILE *f;
  teledisk_prop_t *prop;

  f = fopen(filename, "rb");
  if (f == NULL)
    return NULL;

  prop = malloc(sizeof(teledisk_prop_t));
  if (prop == NULL)
    return NULL;

  prop->f = f;
  prop->heads = 0;
  prop->cylinders = 0;
  prop->sector_size = 0;
  prop->sectors_total = 0;
  prop->comment = NULL;
  prop->comment_date = NULL;

  if (!read_header(prop))
    {
      free(prop);
      return NULL;
    }

  prop->filename = strdup(filename);

  while (242)
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
  if (prop->comment_date != NULL)
    free(prop->comment_date);

  free(prop);
  return;
}

int
teledisk_read_sector(teledisk_prop_t *prop, int c, int h, int s)
{
  int a = -1;

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
    {
      memset(prop->buf, 0, sizeof(prop->buf));
      return -1;
    }

  if (decode_sector(prop, a, prop->buf) < 0)
    {
      memset(prop->buf, 0, sizeof(prop->buf));
      return -1;
    }

  return prop->offset[a].l;
}
