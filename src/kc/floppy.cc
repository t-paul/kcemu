/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: floppy.cc,v 1.3 2001/04/14 15:16:04 tp Exp $
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

/*
 *  The following CP/M Disk Parameter Block description is from the
 *  22DISK Tool by Sydex:
 *
 *  BSH refers to the block shift or the number of left shifts needed
 *       to translate an allocation block number to a relative sector
 *       address.
 *  
 *  BLM refers to the block mask or a bit mask of ones  corresponding
 *       to the number of bits specified by BSH.  For example, if BSH
 *       is 3, BLM is 7 (3 bits).
 *  
 *  EXM refers  to  the  extent  mask  or a bit mask of ones used in
 *       delimiting extents, or groups of 128 128-byte records.
 *  
 *  DSM refers to the total number of sectors present on a  diskette.
 *       This  quantity,  like all of the CP/M system variables,  as-
 *       sumes a 128-byte sector.
 *  
 *  DRM refers to the total number of directory entries available  on
 *       a diskette.  DRM is also related to the AL0 and AL1 fields.
 *  
 *  AL0 and AL1 form a bit mask,  beginning with the most significant
 *       bit in byte AL0 and ending with the least significant bit in
 *       AL1.   These two bytes map the first 16 allocation blocks of
 *       the  disk.   A 1 bit in a position indicates that an alloca-
 *       tion block is reserved,  usually for the directory (see DRM,
 *       above).
 *  
 *  OFS indicates the number of tracks that are used for storing the
 *       CP/M system and BIOS at the beginning of a diskette.
 *  
 *  SOFS indicates the number of sectors that are  used  for  storing
 *       the  CP/M  system  and  BIOS at the beginning of a diskette.
 *       SOFS is used when the number of sectors used by the CP/M  is
 *       not  an  integral  number of tracks,  such as for the Coleco
 *       Adam.
 *
 *
 *  A working parameter block to read standard KC-CAOS disks looks
 *  like this:
 *
 *    BEGIN KC KC
 *    DENSITY MFM,LOW
 *    CYLINDERS 80
 *    SIDES 2
 *    SECTORS 5,1024
 *    SIDE1 0 1,2,3,4,5
 *    SIDE2 1 1,2,3,4,5
 *    ORDER SIDES
 *    BSH 4 BLM 15 EXM 0 DSM 390 DRM 127 AL0 11000000B AL1 0 OFS 4
 *    END
 *
 *  SPT 5 BSH 3 BLM 7 EXM 0 DSM 799 DRM ? AL0 AL1 0 CKS ? OFS 4 PSH ? PHM ?
 
 *  parameters for CDM create command:
 *
 *  $> create disk.img 819200 -b1024 -d127 -o4 -s5
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/rc.h"
#include "kc/fdc.h"
#include "kc/floppy.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

static FILE *__f;
static struct {
  int  head;
  int  cylinder;
  int  sector;
  int  count;
  long offset;
} __offset[5000];
static int __offset_len = 0;

static void
read_offsets(FILE *f)
{
  int a, acyl, asid, lcyl, lsid, lsec, llen, c, count;
  long idx, offset;

  idx = 0;
  while (242)
    {
      acyl = fgetc(f); idx++; if (acyl == EOF) return;
      asid = fgetc(f); idx++; if (asid == EOF) return;
      lcyl = fgetc(f); idx++; if (lcyl == EOF) return;
      lsid = fgetc(f); idx++; if (lsid == EOF) return;
      lsec = fgetc(f); idx++; if (lsec == EOF) return;
      llen = fgetc(f); idx++; if (llen == EOF) return;
      c    = fgetc(f); idx++; if (c    == EOF) return;
      count = c;
      c    = fgetc(f); idx++; if (c    == EOF) return;
      count |= (c << 8);

      __offset[__offset_len].head     = lsid;
      __offset[__offset_len].cylinder = lcyl;
      __offset[__offset_len].sector   = lsec;
      __offset[__offset_len].count    = count;
      __offset[__offset_len].offset   = idx;
      __offset_len++;

      for (a = 0;a < count;a++)
        {
          c = fgetc(f);
          idx++;
          if (c == EOF)
            return;
        }
    }
}

static void
list_offsets(void)
{
  int a;

  for (a = 0;a < __offset_len;a++)
    {
      printf("%3d: %d - %4d - %2d - %8ld\n",
             a,
             __offset[a].head,
             __offset[a].cylinder,
             __offset[a].sector,
             __offset[a].offset);
    }
}

class CMD_floppy_attach : public CMD
{
private:
  Floppy *_f;
public:
  CMD_floppy_attach(Floppy *f) : CMD("floppy-attach")
    {
      _f = f;
      register_cmd("floppy-attach");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *filename;

      if (!args)
        return;

      filename = args->get_string_arg("filename");
      if (!filename)
        return;

      _f->attach(filename);
    }
};

Floppy::Floppy(void)
{
  const char *filename;

  _head_count = 0;
  _cylinder_count = 0;
  _sector_size = 0;
  _sectors_per_cylinder = 0;

  filename = RC::instance()->get_string("Floppy Disk",
		"/home/tp/projects/KCemu-data/Floppy/caos.dump");
  attach(filename);

  CMD *cmd;
  cmd = new CMD_floppy_attach(this);
}

Floppy::~Floppy(void)
{
}

int
Floppy::get_head_count(void)
{
  return _head_count;
}

int
Floppy::get_cylinder_count(void)
{
  return _cylinder_count;
}

int
Floppy::get_sector_size(void)
{
  return _sector_size;
}

int
Floppy::get_sectors_per_cylinder(void)
{
  return _sectors_per_cylinder;
}

Floppy::floppy_type_t
Floppy::check_type(const char *filename)
{
  FILE *f;
  char buf[128];
  struct stat statbuf;

  if (stat(filename, &statbuf) != 0)
    return FLOPPY_TYPE_UNKNOWN;

  f = fopen(filename, "rb");
  if (!f)
    return FLOPPY_TYPE_UNKNOWN;

  if (fread(buf, 1, 128, f) != 128)
    {
      fclose(f);
      return FLOPPY_TYPE_UNKNOWN;
    }
  fclose(f);
 
  if (strncmp(buf, "<CPM_Disk>", 10) == 0)
    return FLOPPY_TYPE_CDM;

  if (statbuf.st_size == 825600)
    return FLOPPY_TYPE_22DSK;

  return FLOPPY_TYPE_UNKNOWN;
}

void
Floppy::attach(const char *filename)
{
  int a, c, h, s;
  floppy_type_t type;

  if (__f)
    fclose(__f);

  type = check_type(filename);

  __f = fopen(filename, "rb+");
  if (!__f)
    return;

  switch (type)
    {
    case FLOPPY_TYPE_CDM:
      a = 0;
      for (c = 0;c < 80;c++)
        for (h = 0;h < 2;h++)
          for (s = 1;s <= 5;s++)
            {
              __offset[a].head = h;
              __offset[a].cylinder = c;
              __offset[a].sector = s;
              __offset[a].count = 1024;
              __offset[a].offset = 1024 * a + 128;
              a++;
            }
      _head_count = 2;
      _cylinder_count = 80;
      _sector_size = 1024;
      _sectors_per_cylinder = 5;
      list_offsets();
      break;
    case FLOPPY_TYPE_22DSK:
      read_offsets(__f);
      _head_count = 2;
      _cylinder_count = 80;
      _sector_size = 1024;
      _sectors_per_cylinder = 5;
      break;
    case FLOPPY_TYPE_UNKNOWN:
      break;
    }
}

bool
Floppy::seek(int head, int cylinder, int sector)
{
  int a;

  for (a = 0;a < __offset_len;a++)
    {
      if ((__offset[a].head == head) &&
          (__offset[a].cylinder == cylinder) &&
          (__offset[a].sector == sector))
        {
          DBG(2, form("KCemu/Floppy/seek",
                      "Floppy::seek(): H/C/S: %d/%d/%d -> %ld\n",
                      head, cylinder, sector, __offset[a].offset));
          
          fseek(__f, __offset[a].offset, SEEK_SET);
	  _head = head;
	  _cylinder = cylinder;
	  _sector = sector;
          return false;
        }
    }
  DBG(2, form("KCemu/Floppy/seek",
              "Floppy::seek(): H/C/S: %d/%d/%d -> failed!\n",
              head, cylinder, sector));
  _head = 0;
  _cylinder = 0;
  _sector = 1;
  return true;
}

int
Floppy::read_sector(byte_t *buf, int len)
{
  if (len != _sector_size)
    return -1;
  
  DBG(2, form("KCemu/Floppy/read_sector",
              "Floppy::read_sector(): H/C/S: %d/%d/%d (size = %d)\n",
              _head, _cylinder, _sector, _sector_size));

  return fread(buf, 1, _sector_size, __f);
}

int
Floppy::write_sector(byte_t *buf, int len)
{
  if (len != _sector_size)
    return -1;
  
  DBG(2, form("KCemu/Floppy/write_sector",
              "Floppy::write_sector(): H/C/S: %d/%d/%d (size = %d)\n",
              _head, _cylinder, _sector, _sector_size));

  return fwrite(buf, 1, _sector_size, __f);
}
