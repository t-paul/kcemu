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

#include <string.h>
#include <iostream>
#include <iomanip>

#include <zlib.h>
#include <unistd.h>
#include <stdlib.h>

#include "kc/memstream.h"

#include "libtape/kct.h"

#include "libdbg/dbg.h"

#define CHECK_HEAD(buffer, byte)           \
        (((buffer)[0] & 0xff) == (byte) && \
         ((buffer)[1] & 0xff) == (byte) && \
         ((buffer)[2] & 0xff) == (byte))

using namespace std;

const unsigned short KCTFile::KCT_VERSION          = __KCT_VERSION__;
const unsigned long  KCTFile::BLOCK_SIZE           = 256;
const unsigned long  KCTFile::HEADER_OFFSET        = 0;
const unsigned long  KCTFile::BAM_OFFSET           = 256;
const unsigned long  KCTFile::DIR_BLOCKS           = KCT_DIR_BLOCKS;
const unsigned long  KCTFile::DIR_BLOCK1_OFFSET    = 512;
const unsigned long  KCTFile::DIRBLOCK_OFFSET_MASK = 0xffffff00;
const unsigned long  KCTFile::DIRBLOCK_INDEX_MASK  = 0x000000ff;
const unsigned long  KCTFile::DIRBLOCK_MAGIC       = 0x6564434b;

static ostream& cout_write(int width, char fill, ios::fmtflags justify)
{
  cout.width(width);
  cout.fill(fill);
  cout.setf(justify, ios::left | ios::right | ios::internal);
  return cout;
}

KCTDir::KCTDir(void)
{
}

KCTDir::~KCTDir(void)
{
  clear();
}

void
KCTDir::clear(void)
{
  for (iterator it = begin();it != end();it++)
    delete *it;

  erase(begin(), end());
}

void
KCTDir::add(kct_dirent_t *dirent)
{
  insert(end(), dirent);
}

KCTFile::KCTFile(void)
{  
  _f = 0;
}

KCTFile::~KCTFile(void)
{
  close();
}

const char *
KCTFile::type_name(kct_file_type_t type)
{
  switch (type)
    {
    case KCT_TYPE_COM:    return "COM";
    case KCT_TYPE_BAS:    return "BASIC";
    case KCT_TYPE_DATA:   return "DATA";
    case KCT_TYPE_LIST:   return "LIST";
    case KCT_TYPE_BAS_P:  return "BASIC*";
    case KCT_TYPE_BIN:    return "BIN";
    case KCT_TYPE_BASICODE: return "BASICODE";
    }
  return "???";
}

/*
 *  HEADER functions
 */
bool
KCTFile::header_read(kct_header_t &header, unsigned long offset)
{
  _f->seekg(offset);
  if (_f->fail())
    {
      //cerr << "KCTFile::header_read(): seek error" << endl;
      return false;
    }
  _f->read((char *)&header, sizeof(kct_header_t));
  if (_f->fail())
    {
      //cerr << "KCTFile::header_read(): read error" << endl;
      return false;
    }
  if (strcmp("KCemu tape file\032", header.id) != 0) return false;

  if (header.version != KCT_VERSION)
    {
      cerr << "Warning: tape version mismatch!"
	   << endl
	   << "         expected v"
           << KCT_VERSION_MAJOR(KCT_VERSION)
	   << "."
           << KCT_VERSION_MINOR(KCT_VERSION)
	   << " and got v"
           << KCT_VERSION_MAJOR(header.version)
	   << "."
           << KCT_VERSION_MINOR(header.version)
	   << endl;

    }

  return true;
}
void
KCTFile::header_write(kct_header_t &header, unsigned long offset)
{
  _f->seekp(offset);
  if (_f->fail())
    {
      cerr << "KCTFile::header_write(): seek error" << endl;
      exit(1);
    }
  _f->write((char *)&header, sizeof(kct_header_t));
  if (_f->fail())
    {
      cerr << "KCTFile::header_write(): write error" << endl;
      exit(1);
    }  
}

/*
 *  BAM functions
 */
void
KCTFile::bam_clear(kct_bam_t &bam)
{
  memset(bam, 0, sizeof(kct_bam_t));
}

bool
KCTFile::bam_block_available(kct_bam_t &bam, long block)
{
  long a = block / 8;
  long b = block % 8;

  if (bam[a] & (128 >> b)) return false;
  return true;
}

void
KCTFile::bam_block_free(kct_bam_t &bam, long block)
{
  long a = block / 8;
  long b = block % 8;

  bam[a] &= ~(128 >> b);
}

void
KCTFile::bam_block_allocate(kct_bam_t &bam, long block)
{
  long a = block / 8;
  long b = block % 8;

  bam[a] |= (128 >> b);
}

unsigned long
KCTFile::bam_block_find_free(kct_bam_t &bam)
{
  unsigned int a;
  
  for (a = 0;a < 8 * sizeof(kct_bam_t);a++)
    {
      if (bam_block_available(bam, a))
        {
          bam_block_allocate(bam, a);
          return a * 256;
        }
    }

  return 0;
}

void
KCTFile::bam_read(kct_bam_t &bam, unsigned long offset)
{
  _f->seekg(offset);
  if (_f->fail())
    {
      cerr << "KCTFile::bam_read(): seek error" << endl;
      exit(1);
    }
  _f->read((char *)bam, sizeof(kct_bam_t));
  if (_f->fail())
    {
      cerr << "KCTFile::bam_read(): read error" << endl;
      exit(1);
    }
}
void
KCTFile::bam_write(kct_bam_t &bam, unsigned long offset)
{
  _f->seekp(offset);
  if (_f->fail())
    {
      cerr << "KCTFile::bam_write(): seek error" << endl;
      exit(1);
    }
  _f->write((char *)bam, sizeof(kct_bam_t));
  if (_f->fail())
    {
      cerr << "KCTFile::bam_write(): write error" << endl;
      exit(1);
    }  
}

/*
 *  DIRENT functions
 */
kct_error_t
KCTFile::dirblock_clear(kct_dirblock_t &dirblock)
{
  memset(dirblock, 0, sizeof(kct_dirblock_t));
  dirblock[0].magic = DIRBLOCK_MAGIC;
  dirblock[1].magic = DIRBLOCK_MAGIC;
  dirblock[2].magic = DIRBLOCK_MAGIC;
  dirblock[3].magic = DIRBLOCK_MAGIC;
  return KCT_OK;
}

kct_error_t
KCTFile::dirblock_read(kct_dirblock_t &dirblock, unsigned long offset)
{
  unsigned long o = offset & DIRBLOCK_OFFSET_MASK;
  
  _f->seekg(o);
  if (_f->fail())
    {
      cerr << "KCTFile::dirblock_read(): seek error" << endl;
      return KCT_ERROR_IO;
    }
  _f->read((char *)dirblock, sizeof(kct_dirblock_t));
  if (_f->fail())
    {
      cerr << "KCTFile::dirblock_read(): read error" << endl;
      return KCT_ERROR_IO;
    }
  if ((dirblock[0].magic != DIRBLOCK_MAGIC) ||
      (dirblock[1].magic != DIRBLOCK_MAGIC) ||
      (dirblock[2].magic != DIRBLOCK_MAGIC) ||
      (dirblock[3].magic != DIRBLOCK_MAGIC))
    {
      cerr << "KCTFile::dirblock_read(): wrong magic" << endl;
      return KCT_ERROR_IO;
    }
  return KCT_OK;
}

kct_error_t
KCTFile::dirblock_write(kct_dirblock_t &dirblock, unsigned long offset)
{
  unsigned long o = offset & DIRBLOCK_OFFSET_MASK;
  
  _f->seekp(o);
  if (_f->fail())
    {
      cerr << "KCTFile::dirblock_write(): seek error" << endl;
      return KCT_ERROR_IO;
    }
  _f->write((char *)dirblock, sizeof(kct_dirblock_t));
  if (_f->fail())
    {
      cerr << "KCTFile::dirblock_write(): write error" << endl;
      return KCT_ERROR_IO;
    }
  return KCT_OK;
}

kct_error_t
KCTFile::dirent_allocate(const char        *filename,
			 unsigned short     load,
			 unsigned short     start,
                         unsigned long      len,
                         unsigned long      clen,
                         unsigned long      crc,
                         unsigned long     *offset,
			 kct_file_type_t    type,
                         kct_machine_type_t machine)
{
  int idx, ent;
  unsigned int a;
  unsigned long o;
  unsigned long pos;
  kct_error_t ret;

  idx = -1;
  for (a = 0;a < DIR_BLOCKS;a++)
    {
      if ((_header.offset[a] & 0x0f) != 0x0f)
        {
          idx = a;
          break;
        }
    }

  if (idx == -1)
    return KCT_ERROR_NOMEM;

  o = _header.offset[idx];

  if (o == 0)
    {
      ret = dirblock_clear(_dirblock);
      if (ret != KCT_OK)
        return ret;
      _f->seekp(0, ios::end);
      pos = _f->tellp();
      if ((pos % 256) != 0)
        {
          cerr << "warning: format error" << endl;
        }
      pos = (pos + 255) & DIRBLOCK_OFFSET_MASK;
      ent = 0;
      o = *offset;
      *offset = bam_block_find_free(_bam);
      if (*offset == 0)
        return KCT_ERROR_NOMEM;
      _header.offset[idx] = o | 1;
      bam_write(_bam, BAM_OFFSET);
    }
  else
    {
      if ((o & 1) == 0) ent = 0;
      else if ((o & 2) == 0) ent = 1;
      else if ((o & 4) == 0) ent = 2;
      else /*if ((o & 8) == 0)*/ ent = 3; /* o & 0xf is known to be != 0xf */
      //o &= DIRBLOCK_OFFSET_MASK;
      _header.offset[idx] |= (1 << ent);
      ret = dirblock_read(_dirblock, o);
      if (ret != KCT_OK)
        return ret;
    }
    
  header_write(_header, HEADER_OFFSET);

  _dirblock[ent].magic             = DIRBLOCK_MAGIC;
  _dirblock[ent].file_crc          = crc;
  _dirblock[ent].compressed_size   = clen;
  _dirblock[ent].uncompressed_size = len;
  _dirblock[ent].offset            = *offset;
  _dirblock[ent].load_addr         = load;
  _dirblock[ent].start_addr        = start;
  _dirblock[ent].type              = type;
  _dirblock[ent].machine           = machine;
  strncpy(_dirblock[ent].name, filename, KCT_NAME_LENGTH);
  _dirblock[ent].name[KCT_NAME_LENGTH] = '\0';

  return dirblock_write(_dirblock, o);
}

kct_error_t
KCTFile::create(const char *filename)
{
  close();

  if (access(filename, F_OK) == 0)
    return KCT_ERROR_EXIST;
  
  /*
   *  create file; fails with gcc-3.2 when including ios::in :-(
   */
  _f = new fstream(filename, ios::out | ios::binary | ios::trunc);
  if (_f->fail())
    {
      delete _f;
      _f = 0;
      return KCT_ERROR_IO;
    }
  _f->close();

  /*
   *  reopen with in/out
   */
  _f = new fstream(filename, ios::in | ios::out | ios::binary);
  if (_f->fail())
    {
      delete _f;
      _f = 0;
      return KCT_ERROR_IO;
    }

  memset(&_header, 0, sizeof(kct_header_t));
  strcpy(_header.id, "KCemu tape file\032");
  _header.version = KCT_VERSION;
  
  _f->write((char *)&_header, sizeof(_header));
  if (_f->fail())
    {
      /* cerr << "write error (header)" << endl; */
      return KCT_ERROR_IO;
    }

  bam_clear(_bam);
  bam_block_allocate(_bam, 0); /* header */
  bam_block_allocate(_bam, 1); /* the bam itself */
  _f->write((char *)&_bam, sizeof(kct_bam_t));
  if (_f->fail())
    {
      /* cerr << "write error (bam)" << endl; */
      return KCT_ERROR_IO;
    }

  return KCT_OK;
}

/*
 *  return values: KCT_ERROR_NOENT
 *                 KCT_ERROR_IO
 *                 KCT_OK
 */
kct_error_t
KCTFile::open(const char *filename)
{
  close();

  if (access(filename, R_OK | W_OK) == 0)
    {
      _readonly = false;
      _f = new fstream(filename, ios::in | ios::out | ios::binary);
    }
  else if (access(filename, R_OK) == 0)
    {
      _readonly = true;
      _f = new fstream(filename, ios::in | ios::binary);
    }
  else
    {
      return KCT_ERROR_NOENT;
    }

  if (_f->fail())
    return KCT_ERROR_NOENT;

  if (header_read(_header, HEADER_OFFSET) == 0)
    return KCT_ERROR_IO;
  
  bam_read(_bam, BAM_OFFSET);

  if (_readonly)
    return KCT_OK_READONLY;

  return KCT_OK;
}

bool
KCTFile::is_readonly(void)
{
  return _readonly;
}

kct_error_t
KCTFile::close(void)
{
  if (_f != 0)
    {
      _f->close();
      delete _f;
    }
  _f = 0;
  return KCT_OK;
}

void
KCTFile::list(void)
{
  int com;
  const char *type;
  float ratio;
  unsigned long count, c_total, u_total;

  count = 0;
  c_total = 0;
  u_total = 0;
  
  cout << "--------------------------------------------------------------------------"
       << endl
       << "Filename                         type  load   start  compr.     uncompr."
       << endl
       << "                                       addr   addr   size       size"
       << endl
       << "--------------------------------------------------------------------------"
       << endl;
  
  for (KCTDir::iterator it = _dir.begin();it != _dir.end();it++)
    {
      com = 0;
      switch ((*it)->type)
	{
	case KCT_TYPE_COM:    type = "COM"; com = 1; break;
	case KCT_TYPE_DATA:   type = "DATA"; break;
	case KCT_TYPE_LIST:   type = "LIST"; break;
	case KCT_TYPE_BAS:    type = "BAS";  break;
	case KCT_TYPE_BAS_P:  type = "PBAS"; break;
	case KCT_TYPE_BIN:    type = "BIN";  break;
	case KCT_TYPE_BASICODE: type = "BAC";  break;
	default:              type = "???";  break;
	}

      cout_write(32, ' ', ios::left) << (*it)->name << " ";
      if (com && ((*it)->start_addr == 0xffff))
	{
	  cout_write( 5, ' ', ios::left) << type << " 0x";
	  cout_write( 4, '0', ios::right) << hex << (*it)->load_addr << " -      0x";
	  cout_write( 8, '0', ios::right) << hex << (*it)->compressed_size << " 0x";
	  cout_write( 8, '0', ios::right) << hex << (*it)->uncompressed_size << endl;
	}
      else if (com)
	{
	  cout_write( 5, ' ', ios::left) << type << " 0x";
	  cout_write( 4, '0', ios::right) << hex << (*it)->load_addr << " 0x";
	  cout_write( 4, '0', ios::right) << hex << (*it)->start_addr << " 0x";
	  cout_write( 8, '0', ios::right) << hex << (*it)->compressed_size << " 0x";
	  cout_write( 8, '0', ios::right) << hex << (*it)->uncompressed_size << endl;
        }
      else
	{
	  cout_write( 5, ' ', ios::left) << type << " -      -      0x";
	  cout_write( 8, '0', ios::right) << hex << (*it)->compressed_size << " 0x";
	  cout_write( 8, '0', ios::right) << hex << (*it)->uncompressed_size << endl;
	}
      count++;
      c_total += (*it)->compressed_size;
      u_total += (*it)->uncompressed_size;
    }

  ratio = 0;
  if (u_total != 0)
    ratio = (100.0 * c_total) / u_total;

  cout << "--------------------------------------------------------------------------"
       << endl
       << "files: " << dec << count << ", compressed size: "
       << c_total << ", uncompressed size: "
       << u_total << ", ratio: "
       << setprecision(3) << ratio << "%"
       << endl
       << "--------------------------------------------------------------------------"
       << endl;
}

KCTDir *
KCTFile::readdir(void)
{
  unsigned int a, b;
  kct_dirent_t *dirent;

  _dir.clear();

  for (a = 0;a < DIR_BLOCKS;a++)
    {
      if (_header.offset[a] == 0) continue;
      dirblock_read(_dirblock, _header.offset[a]);
      for (b = 0;b < 4;b++)
        {
          if (_header.offset[a] & (1 << b))
            {
              dirent = new kct_dirent_t();
              memcpy(dirent, &_dirblock[b], sizeof(kct_dirent_t));
              _dir.add(dirent);
            }
        }
    }

  return &_dir;
}

int
KCTFile::translate_index(int idx)
{
  int _idx;
  unsigned int a, b;

  _idx = 0;
  for (a = 0;a < DIR_BLOCKS;a++)
    {
      for (b = 0;b < 4;b++)
        {
          if (_header.offset[a] & (1 << b))
	    idx--;

          if (idx < 0)
	    return _idx;

	  _idx++;
        }
    }

  return -1;
}

istream *
KCTFile::read(int idx, kct_file_props_t *props)
{
  int ret;
  unsigned long csize, usize;
  unsigned long offset;
  kct_data_t data;
  unsigned char *cbuf, *ubuf, *ptr;
  
  idx = translate_index(idx);

  offset = _header.offset[idx / 4];

  dirblock_read(_dirblock, offset);

  idx %= 4;

  csize = _dirblock[idx].compressed_size;
  cbuf = new unsigned char[csize + 256];
  ptr = cbuf;
  
  offset = _dirblock[idx].offset;
  while (offset != 0)
    {
      _f->seekg(offset);
      _f->read((char *)&data, BLOCK_SIZE);
      memcpy(ptr, &data.data, 252);
      ptr += 252;
      offset = data.link;
    }

  usize = _dirblock[idx].uncompressed_size;
  ubuf = new unsigned char[usize];
  ret = uncompress(ubuf, &usize, cbuf, csize);
  delete[] cbuf;

  if (ret != Z_OK)
    {
      cerr << "error in uncompress: " << ret << endl;
      return NULL;
    }

  if (props)
    {
      if (ubuf[16] < 3)
        props->auto_start = false;
      else
        props->auto_start = true;
      props->load_addr = _dirblock[idx].load_addr;
      props->start_addr = _dirblock[idx].start_addr;
      props->size = usize;
      props->type = (kct_file_type_t)_dirblock[idx].type;
      props->machine = (kct_machine_type_t)_dirblock[idx].machine;
    }
  else
    cerr << "kct_props: NULL!" << endl;

  return new memstream((unsigned char *)ubuf, usize);
}

istream *
KCTFile::read(const char *name, kct_file_props_t *props)
{
  int a;

  a = 0;
  for (KCTDir::iterator it = _dir.begin();it != _dir.end();it++)
    {
      if (strcmp((*it)->name, name) == 0)
        return read(a, props);
      a++;
    }
  cerr << "KCTFile::read(): file not found!" << endl;
  return NULL;
}

/*
 *  return: KCT_ERROR_EXIST if filename already in archive
 *          KCT_ERROR_NOMEM on memory allocation error
 *          KCT_ERROR_IO    if file compression fails or
 *                          no block free in the archive
 *          KCT_OK          if all went well
 */
kct_error_t
KCTFile::write(const char *filename,
               const unsigned char *buf,
               unsigned long len,
               unsigned short load_addr,
               unsigned short start_addr,
               kct_file_type_t type,
               kct_machine_type_t machine)
{
  int ret;
  unsigned int a;
  unsigned long clen;
  unsigned char *cbuf;
  unsigned long crc;
  unsigned long offset;
  kct_error_t err;

  /*
   *  check if the filename already exists in the archive
   */
  readdir();
  if (find(filename) == KCT_OK)
    return KCT_ERROR_EXIST;

  DBG(1, form("KCemu/KCT/write",
              "KCTFile::write(): name = '%s'\n"
              "KCTFile::write(): type = %s (%02x)\n"
              "KCTFile::write(): addr = %04x/%04x\n",
              filename, type_name(type), type,
              load_addr, start_addr));

  clen = len + (len / 1000 + 1) + 12;
  cbuf = new unsigned char [clen];
  if (cbuf == NULL)
    return KCT_ERROR_NOMEM;

  ret = compress(cbuf, &clen, buf, len);

  DBG(1, form("KCemu/KCT/write",
              "KCTFile::write(): compress returned %d\n" 
              "KCTFile::write():  len = %d\n"
              "KCTFile::write(): clen = %d\n",
              ret, len, clen));

  if (ret != Z_OK)
    {
      cerr << "error in compress" << endl;
      return KCT_ERROR_IO;
    }

  crc = crc32(0L, Z_NULL, 0);
  crc = crc32(crc, buf, len);

  offset = bam_block_find_free(_bam);
  if (offset == 0)
    return KCT_ERROR_IO;

  err = dirent_allocate(filename, load_addr, start_addr,
                        len, clen, crc, &offset,
                        type, machine);
  if (err != KCT_OK)
    return err;

  for (;;)
    {
      _f->seekp(offset);
      if (clen < 252)
        {
          offset = 0;
          _f->write((char *)&offset, 4);
          _f->write((char *)cbuf, clen);
          /* pad to 256 byte */
          for (a = 0;a < 252 - clen;a++) _f->put(0);
          break;
        }
      else
        {
          offset = bam_block_find_free(_bam);
          if (offset == 0)
            return KCT_ERROR_IO;
          _f->write((char *)&offset, 4);
          _f->write((char *)cbuf, 252);
        }
      cbuf += 252;
      clen -= 252;
    }

  bam_write(_bam, BAM_OFFSET);

  return KCT_OK;
}

kct_error_t
KCTFile::remove(int idx)
{
  int entry;
  kct_data_t data;
  unsigned long offset;

  idx = translate_index(idx);

  entry = idx % 4;
  idx /= 4;
  offset = _header.offset[idx];
  _header.offset[idx] &= ~(1 << entry);

  dirblock_read(_dirblock, offset);
  if ((_header.offset[idx] & DIRBLOCK_INDEX_MASK) == 0)
    {
      /* cerr << "*** free dirblock ***" << endl; */
      _header.offset[idx] = 0;
      bam_block_free(_bam, offset / BLOCK_SIZE);
    }
  header_write(_header, HEADER_OFFSET);

  /* cerr << "KCTFile::remove(): name = " << _dirblock[entry].name << endl; */

  offset = _dirblock[entry].offset;
  while (offset != 0)
    {
      bam_block_free(_bam, offset / BLOCK_SIZE);
      _f->seekg(offset);
      _f->read((char *)&data, BLOCK_SIZE);
      offset = data.link;
    }
  bam_write(_bam, BAM_OFFSET);

  readdir(); /* reload internal directory list (_dir) */

  return KCT_OK;
}

kct_error_t
KCTFile::rename(int idx, const char *to)
{
  int entry;
  kct_error_t err;
  unsigned long offset;

  idx = translate_index(idx);

  entry = idx % 4;
  idx /= 4;
  offset = _header.offset[idx];

  dirblock_read(_dirblock, offset);
  strncpy(_dirblock[entry].name, to, KCT_NAME_LENGTH);
  _dirblock[entry].name[KCT_NAME_LENGTH] = '\0';

  err = dirblock_write(_dirblock, offset);

  readdir(); /* reload internal directory list (_dir) */

  return err;
}

int
KCTFile::find_entry(const char *name)
{
  int a = 0;

  for (KCTDir::iterator it = _dir.begin();it != _dir.end();it++)
    {
      if (strcmp((*it)->name, name) == 0)
        return a;
      a++;
    }

  return -1;
}

kct_error_t
KCTFile::find(const char *name)
{
  int idx;

  idx = find_entry(name);
  if (idx < 0)
    return KCT_ERROR_NOENT;

  return KCT_OK;
}

kct_error_t
KCTFile::remove(const char *name)
{
  int idx;

  idx = find_entry(name);
  if (idx < 0)
    return KCT_ERROR_NOENT;

  return remove(idx);
}

kct_error_t
KCTFile::rename(const char *name, const char *to)
{
  int idx;

  idx = find_entry(to);
  if (idx >= 0)
    return KCT_ERROR_EXIST;

  idx = find_entry(name);
  if (idx < 0)
    return KCT_ERROR_NOENT;
  
  return rename(idx, to);
}

const char *
KCTFile::get_error_string(kct_error_t error)
{
  switch (error)
    {
    case KCT_OK_READONLY:
      return "success";
    case KCT_OK:
      return "success";
    case KCT_ERROR_NOENT:
      return "no such file";
    case KCT_ERROR_IO:
      return "io error";
    case KCT_ERROR_NOMEM:
      return "out of memory error";
    case KCT_ERROR_ACCESS:
      return "access error";
    case KCT_ERROR_EXIST:
      return "file already exists";
    case KCT_ERROR_INVAL:
      return "invalid parameter";
    case KCT_ERROR_NAMETOOLONG:
      return "name too long";
    }
  return "<unknown>";
}

#ifdef DEBUG
void
KCTFile::test(void)
{
  int a;
  bool ok;
  kct_bam_t bam;

  /*
    cerr << "***" << endl
    << "*** KCTFile::test()" << endl
    << "***" << endl;
  */

  ok = true;

  if (sizeof(kct_header_t) != 256)
    {
      ok = false;
      cerr << "*** sizeof kct_header_t != 256 (" << sizeof(kct_header_t) << ")" << endl;
    }
  
  if (sizeof(kct_bam_t) != 256)
    {
      ok = false;
      cerr << "*** sizeof kct_bam_t != 256 (" << sizeof(kct_bam_t) << ")" << endl;
    }

  if (sizeof(kct_data_t) != 256)
    {
      ok = false;
      cerr << "*** sizeof kct_data_t != 256 (" << sizeof(kct_data_t) << ")" << endl;
    }

  if (sizeof(kct_dirblock_t) != 256)
    {
      ok = false;
      cerr << "*** sizeof kct_dirblock_t != 256 (" << sizeof(kct_dirblock_t) << ")" << endl;
    }

  bam_clear(bam);
  for (a = 0;a < 256;a++)
    {
      if (bam[a] != 0)
        {
          ok = false;
          cerr << "*** clearing BAM failed" << endl;
          break;
        }
    }

  for (a = 0;a < 2048;a++)
    {
      if (!bam_block_available(bam, a))
        {
          ok = false;
          cerr << "*** bam_block_available error (1)" << endl;
          break;
        }
      bam_block_allocate(bam, a);
      if (bam_block_available(bam, a))
        {
          ok = false;
          cerr << "*** bam_block_available error (2)" << endl;
          break;
        }
      bam_block_free(bam, a);
      if (!bam_block_available(bam, a))
        {
          ok = false;
          cerr << "*** bam_block_available error (3)" << endl;
          break;
        }
      bam_block_allocate(bam, a);
    }

  if (!ok) cerr << "*** check failed!" << endl;
}

void
KCTFile::print_bam(void)
{
  int a, b, c;

  for (a = 0;a < 16;a++)
    {
      for (b = 0;b < 16;b++)
        {
          for (c = 0;c < 8;c += 2)
            {
	      switch (((_bam[16 * a + b]) & (0xc0 >> c)) >> (6 - c))
                {
                case 0: cout << '-'; break;
                case 1: cout << ']'; break;
                case 2: cout << '['; break;
                case 3: cout << '#'; break;
		}
	    }
	}
      cout << endl;
    }
}

void
KCTFile::print_block_list(void)
{
  int a, b, c;

  for (a = 0;a < 16;a++)
    {
      for (b = 0;b < 16;b++)
        {
          for (c = 0;c < 8;c++)
            {
	      if ((_bam[16 * a + b]) & (0x80 >> c))
                cout << hex << setw(8) << (32768 * a + 2048 * b + 256 * c) << endl;
	    }
	}
    }
}
#endif /* DEBUG */
