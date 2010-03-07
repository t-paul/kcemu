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

/*
 *  File Version Log:
 *
 *  1.00:  first release
 *  1.01:  reduced length of the name field to free some room for
 *         the file type flag in the directory entry
 *         6 bytes are now unused and should be set to 0
 *  1.02:  argh, I thought the block number of tape blocks doesn't
 *         need to be stored in the file - this is wrong as I found
 *         one application that writes two blocks with a block
 *         number 01 (WordPro) again this proves that Murphy's Law is
 *         right every time ;-)
 *         also I added a new field in the file header that holds the
 *         machine type which the file is for (using 2 bytes may be
 *         a bit excessive but we still have some unused space in the
 *         file header so...)
 */
#ifndef __libtape_format_h
#define __libtape_format_h

#include <list>
#include <fstream>

#define __KCT_VERSION__ (0x0102)

#define KCT_VERSION_MINOR(x) (x & 0xff)
#define KCT_VERSION_MAJOR(x) ((x >> 8) & 0xff)

#define KCT_DIR_BLOCKS (57)
#define KCT_NAME_LENGTH (32)

/*
 *  only 1 byte values are allowed here because
 *  the filetype is stored in the directory entry
 *  as a single byte!
 *
 *  FIXME: I don't know if this is safe on machines
 *         with big endian byte order, I suspect it
 *         is not :-(
 */
typedef enum
{
  KCT_TYPE_COM      = 0x00,
  KCT_TYPE_BASICODE = 0x31,
  KCT_TYPE_BAS      = 0xd3,
  KCT_TYPE_DATA     = 0xd4,
  KCT_TYPE_LIST     = 0xd5,
  KCT_TYPE_BAS_P    = 0xd7,
  KCT_TYPE_BIN      = 0xff,
} kct_file_type_t;

/*
 *  only 2 byte values allowed!
 */
typedef enum
{
  KCT_MACHINE_UNKNOWN = 0x0000, /* should not be used in new files! */
  KCT_MACHINE_KC_85_1 = 0x0001,
  KCT_MACHINE_KC_85_2 = 0x0002,
  KCT_MACHINE_KC_85_3 = 0x0004,
  KCT_MACHINE_KC_85_4 = 0x0008,
  KCT_MACHINE_KC_87   = 0x0010,
  KCT_MACHINE_ALL     = 0xffff,
} kct_machine_type_t;

/*
 *  the actual values are the same as defined in
 *  linux: asm/errno.h
 */
typedef enum
{
  KCT_OK_READONLY       = -1,
  KCT_OK                =  0,
  KCT_ERROR_NOENT       =  2,
  KCT_ERROR_IO          =  5,
  KCT_ERROR_NOMEM       = 12,
  KCT_ERROR_ACCESS      = 13,
  KCT_ERROR_EXIST       = 17,
  KCT_ERROR_INVAL       = 23,
  KCT_ERROR_NAMETOOLONG = 36,
} kct_error_t;

typedef struct
{
  char           id[18];
  unsigned short version;
  unsigned int   unused;
  unsigned int   offset[KCT_DIR_BLOCKS];
  unsigned int   chain_offset;
} kct_header_t;

typedef unsigned char kct_bam_t[256];

typedef struct
{
  unsigned int   magic;
  unsigned int   file_crc;
  unsigned short load_addr;
  unsigned short start_addr;
  unsigned int   uncompressed_size;
  unsigned int   compressed_size;
  unsigned int   offset;
  unsigned short machine;
  unsigned char  type;
  unsigned char  unused[4];
  char           name[KCT_NAME_LENGTH + 1];
} kct_dirent_t;

typedef struct
{
  bool               auto_start;
  unsigned short     load_addr;
  unsigned short     start_addr;
  unsigned int       size;
  kct_file_type_t    type;
  kct_machine_type_t machine;
} kct_file_props_t;

typedef kct_dirent_t kct_dirblock_t[4];

typedef struct
{
  unsigned int  link;
  unsigned char data[252];
} kct_data_t;

class KCTDir : public std::list<kct_dirent_t *>
{
public:
  KCTDir(void);
  virtual ~KCTDir(void);
  
  void clear(void);
  void add(kct_dirent_t *dirent);
};

class KCTFile
{
public:
  static const unsigned short KCT_VERSION;
  static const unsigned long BLOCK_SIZE;
  static const unsigned long HEADER_OFFSET;
  static const unsigned long BAM_OFFSET;
  static const unsigned long DIR_BLOCKS;
  static const unsigned long DIR_BLOCK1_OFFSET;
  static const unsigned long DIRBLOCK_OFFSET_MASK;
  static const unsigned long DIRBLOCK_INDEX_MASK;
  static const unsigned long DIRBLOCK_MAGIC;
  
private:
  bool           _readonly;
  std::fstream  *_f;
  kct_header_t   _header;
  kct_bam_t      _bam;
  kct_dirblock_t _dirblock;
  KCTDir         _dir;
  
protected:
  bool header_read(kct_header_t &header, unsigned long offset);
  void header_write(kct_header_t &header, unsigned long offset);
  
  void bam_clear(kct_bam_t &bam);
  bool bam_block_available(kct_bam_t &bam, long block);
  void bam_block_free(kct_bam_t &bam, long block);
  void bam_block_allocate(kct_bam_t &bam, long block);
  void bam_read(kct_bam_t &bam, unsigned long offset);
  void bam_write(kct_bam_t &bam, unsigned long offset);
  unsigned long bam_block_find_free(kct_bam_t &bam);

  kct_error_t dirblock_clear(kct_dirblock_t &dirblock);
  kct_error_t dirblock_read(kct_dirblock_t &dirblock, unsigned long offset);
  kct_error_t dirblock_write(kct_dirblock_t &dirblock, unsigned long offset);
  
  kct_error_t dirent_allocate(const char        *filename,
                              unsigned short     load,
                              unsigned short     start,
                              unsigned long      len,
                              unsigned long      clen,
                              unsigned long      crc,
                              unsigned long     *offset,
                              kct_file_type_t    type,
                              kct_machine_type_t machine);

  int translate_index(int idx);
  int find_entry(const char *name);

public:
  KCTFile(void);
  virtual ~KCTFile(void);

  const char * type_name(kct_file_type_t type);
  
  void list(void);
  bool is_readonly(void);

  kct_error_t create(const char *filename);
  kct_error_t open(const char *filename);
  kct_error_t close(void);
  kct_error_t remove(int idx);
  kct_error_t remove(const char *name);
  kct_error_t rename(int idx, const char *to);
  kct_error_t rename(const char *name, const char *to);
  kct_error_t find(const char *name);

  kct_error_t write(const char *filename,
                    const unsigned char *buf,
                    unsigned long len,
                    unsigned short load_addr,
                    unsigned short start_addr,
                    kct_file_type_t type,
                    kct_machine_type_t machine = KCT_MACHINE_ALL);
  std::istream *   read(int idx,
                   kct_file_props_t *props = NULL);
  std::istream *   read(const char *name,
                   kct_file_props_t *props = NULL);
  KCTDir *    readdir(void);

  const char * get_error_string(kct_error_t error);

#ifdef DEBUG
  void test(void);
  void print_bam(void);
  void print_block_list(void);
#endif /* DEBUG */
};

#endif /* __libtape_format_h */
