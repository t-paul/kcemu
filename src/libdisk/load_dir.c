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

#include <ctype.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "libdisk/libdiskP.h"

// CP/M record size is 128
#define RECORD_SIZE    (128)                                      

// size of one directory entry
#define DIR_ENTRY_SIZE (32)                                       

#define SIZE_TO_REC(x) (((x) + (RECORD_SIZE - 1)) / RECORD_SIZE)

// directory size in bytes
#define DIR_SIZE(dpb)              (DIR_ENTRY_SIZE * ((dpb)->drm + 1))

// disk size in bytes
#define DISK_SIZE(dpb)             (RECORD_SIZE * (((dpb)->dsm + 1) << ((dpb)->bsh)))

// sector size in bytes
#define SECTOR_SIZE(dpb)           (RECORD_SIZE << ((dpb)->bsh))

// size of allocation unit
#define AU_SIZE(dpb)               (RECORD_SIZE << (dpb)->bsh)

#define SIZE_TO_AU(dpb,x)          (((x) + (AU_SIZE(dpb) - 1)) / AU_SIZE(dpb))

// FIXME: uses hardcoded disk geometry
#define CHS_TO_SECT(dpb,c,h,s)     (((s) - 1) + 5 * (h) + 10 * (c))

// FIXME: still uses hardcoded information that AU is 2 * SECTOR size
#define CHS_TO_AU(dpb,c,h,s)       (CHS_TO_SECT((dpb), (c), (h), (s)) / 2)

// FIXME: still uses hardcoded information that AU is 2 * SECTOR size
#define AU_TO_OFFSET(dpb,au,c,h,s) (CHS_TO_SECT((dpb), (c), (h), (s)) - 2 * (au))

// first data allocation unit (after directory, not counting system tracks)
#define FIRST_AU(dpb)              ((DIR_SIZE(dpb) + (AU_SIZE(dpb) - 1)) / AU_SIZE(dpb))

#define SYSTEM_NAME "@boot.sys"

/*
 *
 *  CP/M 2.2 disc formats
 *  (see http://www.seasip.demon.co.uk/Cpm/format22.html)
 *  
 *  CP/M 2.2 works with a much larger range of discs than CP/M 1.4. The
 *  disc statistics are stored in a parameter block (the DPB), which
 *  contains the following information:
 *  
 *  	DEFW	spt	;Number of 128-byte records per track
 *  	DEFB	bsh	;Block shift. 3 => 1k, 4 => 2k, 5 => 4k....
 *  	DEFB	blm	;Block mask. 7 => 1k, 0Fh => 2k, 1Fh => 4k...
 *  	DEFB	exm	;Extent mask, see later
 *  	DEFW	dsm	;(no. of blocks on the disc)-1
 *  	DEFW	drm	;(no. of directory entries)-1
 *  	DEFB	al0	;Directory allocation bitmap, first byte
 *  	DEFB	al1	;Directory allocation bitmap, second byte
 *  	DEFW	cks	;Checksum vector size, 0 for a fixed disc
 *  			;No. directory entries/4, rounded up.
 *  	DEFW	ofs	;Offset, number of reserved tracks
 *  
 *  The directory allocation bitmap is interpreted as:
 *  
 *         al0              al1
 *  b7b6b5b4b3b2b1b0 b7b6b5b4b3b2b1b0
 *   1 1 1 1 0 0 0 0  0 0 0 0 0 0 0 0
 *  
 *   - ie, in this example, the first 4 blocks of the disc contain the
 *   directory.
 *  
 *  
 *  The DPB is not stored on disc. It is either hardwired into the BIOS or
 *  generated on the fly when a disc is logged in.
 *  
 *  The reserved tracks will contain an image of CP/M 2.2, used when the
 *  system is rebooted. Discs can be formatted as data only discs, in
 *  which case they have no system tracks and cannot be used to reboot the
 *  system.
 *  
 *  CP/M 2.2 directory
 *  
 *  The CP/M 2.2 directory has only one type of entry:
 *  
 *  UU F1 F2 F3 F4 F5 F6 F7 F8 T1 T2 T3 EX S1 S2 RC   .FILENAMETYP....
 *  AL AL AL AL AL AL AL AL AL AL AL AL AL AL AL AL   ................
 *  
 *  UU = User number. 0-15 (on some systems, 0-31). The user number
 *       allows multiple files of the same name to coexist on the disc. 
 *       User number = 0E5h => File deleted
 *  Fn - filename
 *  Tn - filetype. The characters used for these are 7-bit ASCII.
 *       The top bit of T1 (often referred to as T1') is set if the
 *       file is read-only.
 *       T2' is set if the file is a system file (this corresponds
 *       to "hidden" on other systems). 
 *  EX = Extent counter, low byte - takes values from 0-31
 *  S2 = Extent counter, high byte.
 *  
 *       An extent is the portion of a file controlled by one directory
 *       entry.  If a file takes up more blocks than can be listed in one
 *       directory entry, it is given multiple entries, distinguished by
 *       their EX and S2 bytes. The formula is: Entry number =
 *       ((32*S2)+EX) / (exm+1) where exm is the extent mask value from
 *       the Disc Parameter Block.
 *  
 *  S1 - reserved, set to 0.
 *  RC - Number of records (1 record=128 bytes) used in this extent, low
 *       byte. The total number of records used in this extent is
 *  
 *       (EX & exm) * 128 + RC
 *  
 *       If RC is 80h, this extent is full and there may be another one on
 *       the disc.  File lengths are only saved to the nearest 128 bytes.
 *  
 *  AL - Allocation. Each AL is the number of a block on the disc. If an
 *       AL number is zero, that section of the file has no storage
 *       allocated to it (ie it does not exist). For example, a 3k file
 *       might have allocation 5,6,8,0,0.... - the first 1k is in block 5,
 *       the second in block 6, the third in block 8.
 *      
 *       AL numbers can either be 8-bit (if there are fewer than 256
 *       blocks on the disc) or 16-bit (stored low byte first).
 *
 *************************************************************************
 *
 *  $a = 2 * $au;
 *  $s = ($a % 5) + 1;
 *  $h = ($a % 10) / 5;
 *  $c = ($a / 10);
 *
 *  ($s - 1) + $h * 5 + $c * 10
 *
 *  Used disk layout: 2 sides, 80 tracks with 5 sectors of 1024 bytes each
 *  (This is the default disk format for the KC)
 */

static const char * loader_dir_get_name(void);
static int loader_dir_open(libdisk_prop_t *prop, const char *path);
static void loader_dir_close(libdisk_prop_t *prop);
static int loader_dir_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len);
static int loader_dir_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len);

static libdisk_loader_t _loader = {
  loader_dir_get_name,
  loader_dir_open,
  loader_dir_close,
  loader_dir_read_sector,
  loader_dir_write_sector
};

#define __DRM (127)                               // FIXME: hack, change to use dpb
#define __DIR_SIZE (DIR_ENTRY_SIZE * (__DRM + 1)) // FIXME: hack, change to use dpb
typedef struct {
  char *path;
  libdisk_dpb_t *dpb;
  char *filename[__DRM + 1];
  long size[__DRM + 1];
  long au[__DRM + 1];
  unsigned char dir[__DIR_SIZE];
} dir_data_t;

typedef struct sector_data_struct {
  int c;
  int h;
  int s;
  int au;
  int len;
  unsigned char *data;
  struct sector_data_struct *next;
} sector_data_t;

/*
 *  DISK PARAMETER BLOCK (for disk format 5*1024*80*2 OFF 2)
 */
static libdisk_dpb_t _dpb = {
  // logical disc descriptor

  80,   // spt: number of 128-byte records per track
  4,    // bsh: block shift. 3 => 1k, 4 => 2k, 5 => 4k....
  15,   // blm: block mask. 7 => 1k, 0Fh => 2k, 1Fh => 4k...
  0,    // exm: extent mask
  389,  // dsm: (no. of blocks on the disc) - 1
  127,  // drm: (no. of directory entries) - 1
  192,  // al0: directory allocation bitmap, first byte
  0,    // al1: directory allocation bitmap, second byte
  16,   // cks: checksum vector size, No. directory entries/4, rounded up.
  2,    // ofs: offset, number of reserved tracks
  
  // physical disc descriptor

  3,    // psh: physical block shift
  7,    // phm: physical block mask
  5,    // eot: number of last sector
  15,   // gap: sector gap
  80,   // ntr: number of tracks
};

static sector_data_t *_sector_data_head = NULL;
static sector_data_t *_sector_data_last = NULL;

static void dump_sector(unsigned char *sector, int length);
static void cache_sector_data(dir_data_t *data, int c, int h, int s, unsigned char *buf, int len);

static void
set_filename(unsigned char *buf, char *filename)
{
  int a;
  char *ptr;
  
  memset(buf, ' ', 11);

  for (a = 0;a < 8;a++)
    {
      if (filename[a] == '\0')
	break;
      if (filename[a] == '.')
	break;
      buf[a] = toupper(filename[a]);
    }

  ptr = strrchr(filename, '.');
  if (ptr == NULL)
    return;
  
  ptr++;
  for (a = 0;a < 3;a++)
    {
      if (ptr[a] == '\0')
	break;
      buf[8 + a] = toupper(ptr[a]);
    }
}

static int
set_directory_entry(dir_data_t *data, int idx, int start_au, long size, char *filename)
{
  int a;
  int au;
  int rec;
  int extend;

  au = 0;
  extend = 0;
  rec = SIZE_TO_REC(size);

  printf("set_directory_entry(): %d: '%s'\n", start_au, filename);

  do
    {
      memset(&data->dir[32 * idx], 0, 32);

      //printf("%d: %s - %d\n", extend, filename, rec);
      set_filename(&data->dir[32 * idx + 1], filename);

      data->dir[32 * idx] = 0; // user
      data->dir[32 * idx + 12] = extend;
      data->dir[32 * idx + 13] = 0;
      data->dir[32 * idx + 14] = 0;
      data->dir[32 * idx + 15] = (rec > 0x80) ? 0x80 : rec;

      for (a = 0;a < 8;a++)
	if (au < SIZE_TO_AU(data->dpb, size))
	  {
	    data->dir[32 * idx + 2 * a + 16] = (start_au + au) & 0xff;
	    data->dir[32 * idx + 2 * a + 17] = (start_au + au) >> 8;
	    au++;
	  }

      rec -= 0x80;
      extend++;
      idx++;
    }
  while (rec > 0);

  return extend;
}

static const char *
loader_dir_get_name(void)
{
  return "disk loader for unix directories";
}

static int
loader_dir_open(libdisk_prop_t *prop, const char *path)
{
  DIR *d;
  int a, idx, ro, au;
  char pwd[4096]; // FIXME: fix getcwd()!
  dir_data_t *data;
  struct stat statbuf;
  struct dirent *dirent;
  
  if (prop == NULL)
    return -1;

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(path) < 0)
    return -1;

  ro = 1;
  d = opendir(".");
  if (d == NULL)
    return -1;

  data = (dir_data_t *)malloc(sizeof(dir_data_t));
  if (data == NULL)
      return -1;

  data->path = strdup(path);
  data->dpb = &_dpb;

  memset(data->dir, 0xe5, DIR_SIZE(data->dpb));
  memset(data->au, 0, (data->dpb->drm + 1) * sizeof(long));
  memset(data->filename, 0, (data->dpb->drm + 1) * sizeof(char *));

  a = 0;
  while (242)
    {
      dirent = readdir(d);
      if (dirent == NULL)
	break;

      if (dirent->d_name[0] == '.')
	continue;

      if (strcmp(dirent->d_name, SYSTEM_NAME) == 0)
	continue;

      if (stat(dirent->d_name, &statbuf) < 0)
	continue;

      if (!S_ISREG(statbuf.st_mode))
	continue;

      if (statbuf.st_size > DISK_SIZE(data->dpb))
	continue;

      data->size[a] = statbuf.st_size;
      data->filename[a] = strdup(dirent->d_name);
      a++;
    }
  
  closedir(d);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  idx = 0;
  au = FIRST_AU(data->dpb);
  for (a = 0;(a <= data->dpb->drm) && (data->filename[a] != NULL);a++)
    {
      data->au[a] = au;
      idx += set_directory_entry(data,
				 idx,
				 au,
				 data->size[a],
				 data->filename[a]);
      
      au += SIZE_TO_AU(data->dpb, data->size[a]);
    }

#if 0
  {
    FILE *f;
    f = fopen("/tmp/dir", "wb");
    fwrite(data->dir, 1, DIR_SIZE, f);
    fclose(f);
  }
#endif
  
  prop->read_only = ro;
  prop->head_count = 2;
  prop->cylinder_count = 80;
  prop->sector_size = 1024;
  prop->sectors_per_cylinder = 5;
  prop->data = data;
  prop->loader = &_loader;

  _sector_data_head = NULL;
  
  return 0;
}

static void
loader_dir_close(libdisk_prop_t *prop)
{
  dir_data_t *data;
  sector_data_t *sector_data;

  if (prop == NULL)
    return;

  if (prop->data == NULL)
    return;

  data = (dir_data_t *)prop->data;

  free(data);

  while (_sector_data_head != NULL)
    {
      sector_data = _sector_data_head;
      _sector_data_head = sector_data->next;
      free(sector_data->data);
      free(sector_data);
    }

  prop->data = NULL;
}

static int
read_block(dir_data_t *data, unsigned char *buf, int idx, long offset, int len)
{
  FILE *f;
  char pwd[4096]; // FIXME: fix getcwd()!

  printf("read_block(): '%s' [%ld]\n", data->filename[idx], offset);

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(data->path) < 0)
    return -1;

  f = fopen(data->filename[idx], "rb");
  if (f == NULL)
    return -1;

  if (fseek(f, offset, SEEK_SET) < 0)
    return -1;

  memset(buf, '\0', len);
  fread(buf, 1, len, f);
  fclose(f);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  return len;
}

static int
write_block(dir_data_t *data, unsigned char *buf, int idx, long offset, int len)
{
  FILE *f;
  char pwd[4096]; // FIXME: fix getcwd()!

  printf("write_block(): '%s' [%ld]\n", data->filename[idx], offset);

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(data->path) < 0)
    return -1;

  f = fopen(data->filename[idx], "r+b");
  if (f == NULL)
    return -1;

  if (fseek(f, offset, SEEK_SET) < 0)
    return -1;

  fwrite(buf, 1, len, f);
  fclose(f);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  return len;
}

static int
read_system_block(char *path, unsigned char *buf, int block, int len)
{
  FILE *f;
  char pwd[4096]; // FIXME: fix getcwd()!

  printf("read_system_block(): block %d\n", block);

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(path) < 0)
    return -1;

  f = fopen(SYSTEM_NAME, "rb");
  if (f == NULL)
    return -1;

  if (fseek(f, block * 1024, SEEK_SET) < 0)
    return -1;

  memset(buf, '\0', len);
  fread(buf, 1, len, f);
  fclose(f);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  return len;
}

static int
write_system_block(char *path, unsigned char *buf, int block, int len)
{
  FILE *f;
  char pwd[4096]; // FIXME: fix getcwd()!

  printf("write_system_block(): block %d\n", block);

  if (getcwd(pwd, 4096) == NULL)
    return -1;

  if (chdir(path) < 0)
    return -1;

  f = fopen(SYSTEM_NAME, "r+b");
  if (f == NULL)
    return -1;

  if (fseek(f, block * 1024, SEEK_SET) < 0)
    return -1;

  fwrite(buf, 1, len, f);
  fclose(f);

  if (chdir(pwd) < 0)
    printf("can't restore working directory!\n"); // FIXME:

  return len;
}

static int
is_system_sector(libdisk_dpb_t *dpb, int c, int h, int s)
{
  return c < dpb->ofs;  
}

static int
is_directory_sector(libdisk_dpb_t *dpb, int c, int h, int s)
{
  /*
   *  Get allocation unit index translated by number of system tracks
   *  so we get index 0 for the first directory sector. The directory
   *  block allocation mask can only store 16 bits, so the maximum
   *  index of a directory allocation unit is 15.
   */
  int dir_al = CHS_TO_AU(dpb, c - dpb->ofs, h, s);
  if (dir_al > 15)
    return 0;

  int dir_al_mask = (dpb->al0 << 8) | dpb->al1;
  return dir_al_mask & (1 << (15 - dir_al));
}

static int
find_index(dir_data_t *data, int c, int h, int s)
{
  int a;
  int au = CHS_TO_AU(data->dpb, c - data->dpb->ofs, h, s);

  for (a = data->dpb->drm;a >= 0;a--)
    {
      if (data->au[a] == 0)
	continue;
      if (data->dir[32 * a] == 0xe5) // skip deleted
	continue;
      if (data->au[a] <= au)
	break;
    }

  return a;
}

static int
loader_dir_read_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int h, c, s;
  dir_data_t *data;

  if (prop == NULL)
    return -1;

  if (prop->data == NULL)
    return -1;

  h = prop->head;
  c = prop->cylinder;
  s = prop->sector;
  data = (dir_data_t *)prop->data;

  if (is_system_sector(data->dpb, c, h, s))
    {
      int block = CHS_TO_SECT(data->dpb, c, h, s);
      return read_system_block(data->path, buf, block, len);
    }
  
  if (is_directory_sector(data->dpb, c, h, s))
    {
      int block = CHS_TO_SECT(data->dpb, c - data->dpb->ofs, h, s);
      memcpy(buf, &data->dir[block * len], len);
      return len;
    }

  /*
   *  normal data sector
   */      
  int idx = find_index(data, c, h, s);
  if (idx < 0)
    {
      printf("*** allocation unit not found for sector C/H/S %d/%d/%d ***\n", c, h, s);
      //memset(buf, 0xe5, len);
      //int block = CHS_TO_SECT(data->dpb, c, h, s);
      //memcpy(buf, _debug_sector_buffer + len * block, len);
      return len;
    }
  
  int au_ofs = AU_TO_OFFSET(data->dpb, data->au[idx], c - data->dpb->ofs, h, s);
  return read_block(data, buf, idx, len * au_ofs, len);
}

static int
loader_dir_write_sector(libdisk_prop_t *prop, unsigned char *buf, int len)
{
  int h = prop->head;
  int c = prop->cylinder;
  int s = prop->sector;

  dir_data_t *data = (dir_data_t *)prop->data;

  printf("loader_dir_write_sector(): C/H/S = %d/%d/%d, len = %d\n",
	 c, h, s, len);

  dump_sector(buf, len);

  int block = CHS_TO_SECT(data->dpb, c, h, s);

  if (is_system_sector(data->dpb, c, h, s))
    {
      write_system_block(data->path, buf, block, len);
    }
  else if (is_directory_sector(data->dpb, c, h, s))
    {
      int block = CHS_TO_SECT(data->dpb, c - data->dpb->ofs, h, s);
      memcpy(&data->dir[block * len], buf, len);
    }
  else
    {
      int idx = find_index(data, c, h, s);

      if (idx < 0)
	{
	  cache_sector_data(data, c, h, s, buf, len);
	}
      else
	{
	  int au_ofs = AU_TO_OFFSET(data->dpb, data->au[idx], c - data->dpb->ofs, h, s);
	  return write_block(data, buf, idx, len * au_ofs, len);
	}
    }

  return len;
}

static void
cache_sector_data(dir_data_t *data, int c, int h, int s, unsigned char *buf, int len)
{
  sector_data_t *sector_data;

  sector_data = malloc(sizeof(sector_data_t));
  sector_data->c = c;
  sector_data->h = h;
  sector_data->s = s;
  sector_data->au = CHS_TO_AU(data->dpb, c - data->dpb->ofs, h, s);
  sector_data->len = len;
  sector_data->next = NULL;
  sector_data->data = malloc(len);
	  
  memcpy(sector_data->data, buf, len);
  
  if (_sector_data_head == NULL)
    {
      _sector_data_head = _sector_data_last = sector_data;
    }
  else
    {
      _sector_data_last->next = sector_data;
      _sector_data_last = sector_data;
    }
}

// DEBUG

static void
dump_sector(unsigned char *sector, int len)
{
  int a, b, blen;
  unsigned char buf[16];

  a = 0;
  while (a < len)
    {
      for (b = 0;b < 16;b++)
	{
	  if ((a + b) >= len)
	    break;
	  buf[b] = sector[a++];
	  if (a >= len)
	    {
	      printf("\n");
	      return;
	    }
	  blen = b + 1;
	}
      printf("%04x: ", a);
      for (b = 0;b < 16;b++)
	{
	  if (b < blen)
	    {
	      printf("%02x ", buf[b]);
	    }
	  else
	    {
	      printf("   ");
	    }
	  if (b == 7)
	    printf("- ");
	}
      printf(" |  ");
      for (b = 0;b < blen;b++)
	{
	  printf("%c", (isprint(buf[b])) ? buf[b] : '.');
	  if (b == 7)
	    printf(" - ");
	}
      printf("\n");
      if (blen  != 16)
	break;
    }
  printf("\n");
}

void
loader_dir_init(void)
{
  libdisk_register_loader(&_loader);
}
