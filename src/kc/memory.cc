/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: memory.cc,v 1.16 2002/10/31 01:46:35 torsten_paul Exp $
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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fstream>

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/memory.h"
#include "ui/status.h"

using namespace std;

byte_t *MemArea::_scratch_r;
byte_t *MemArea::_scratch_w;

MemArea::MemArea(MemAreaGroup *group, byte_t *mem,
                 word_t addr, int prio, bool ro)
{
  if (!_scratch_r)
    {
      _scratch_r = new byte_t[PAGE_SIZE];
      _scratch_w = new byte_t[PAGE_SIZE];
      memset(_scratch_r, 0xff, PAGE_SIZE);
    }
  
  _group = group;
  _addr = addr;
  _prio = prio;
  _readonly = ro;

  _mem = mem;
  if (_mem)
    {
      _active = false;
      _mem_r = _mem;
      _mem_w = ro ? _scratch_w : _mem;
    }
  else
    {
      _active = true;
      _mem_r = _scratch_r;
      _mem_w = _scratch_w;
    }
}

MemArea::~MemArea(void)
{
}

void
MemArea::set_active(bool active)
{
  if (!_mem) return;
  _active = active;
}

void
MemArea::set_readonly(bool ro)
{
  if (!_mem) return;
  _readonly = ro;
  _mem_w = ro ? _scratch_w : _mem;
}

const char *
MemArea::get_name(void)
{
  return _group->get_name();
}

word_t
MemArea::get_addr(void)
{
  return _addr;
}

int
MemArea::get_prio(void)
{
  return _prio;
}

MemAreaGroup::MemAreaGroup(const char *name, word_t addr, dword_t size,
                           byte_t *mem, int prio, bool ro)
{
  _active = false;
  _prio = prio;
  _readonly = ro;
  _name = strdup(name);
  _addr = addr;
  _size = size;
  _mem = mem;
}

MemAreaGroup::~MemAreaGroup(void)
{
  free(_name);
}

void
MemAreaGroup::set_active(bool active)
{
  for (mem_area_list_t::iterator it = _l.begin();it != _l.end();it++)
    (*it)->set_active(active);
}

void
MemAreaGroup::set_readonly(bool ro)
{
  for (mem_area_list_t::iterator it = _l.begin();it != _l.end();it++)
    (*it)->set_readonly(ro);
}

void
MemAreaGroup::add(MemAreaPtr *area_ptr[])
{
  int a;
  byte_t *mem;
  word_t addr;
  MemArea *area;

  mem = _mem;
  addr = _addr;
  for (a = MemArea::PAGE_INDEX(_addr);a < MemArea::PAGE_INDEX(_addr + _size);a++)
    {
      area = new MemArea(this, mem, addr, _prio, _readonly);
      area_ptr[a]->add(area);
      _l.push_back(area);
      if (_mem) mem += MemArea::PAGE_SIZE;
      addr += MemArea::PAGE_SIZE;
    }
}

void
MemAreaGroup::remove(MemAreaPtr *area_ptr[])
{
  word_t addr;
  mem_area_list_t::iterator it;

  for (it = _l.begin();it != _l.end();it++)
    {
      addr = (*it)->get_addr() / MemArea::PAGE_SIZE;
      area_ptr[addr]->remove(*it);
    }
  _l.erase(_l.begin(), _l.end());
}

MemAreaPtr::MemAreaPtr(void)
{
  _ptr = 0;
}

void
MemAreaPtr::add(MemArea *area)
{
  int prio;
  mem_area_list_t::iterator it;

  prio = area->get_prio();
  for (it = _l.begin();it != _l.end();it++)
    if ((*it)->get_prio() > prio) break;
  _l.insert(it, area);
}

void
MemAreaPtr::remove(MemArea *area)
{
  _l.remove(area);
}

void
MemAreaPtr::info(void)
{
  mem_area_list_t::iterator it;
  
  /*
   *  display registered memory areas but don't list the fallback
   *  entry that comes as the last entry
   */
  for (it = _l.begin();it != --(_l.end());it++)
    {
      cerr << " [\"" << (*it)->get_name() << "\" "
	   << ((*it)->is_active() ? 'A' : 'a')
	   << ((*it)->is_readonly() ? 'R' : 'r')
	   << " ]";
    }
}

byte_t *
MemAreaPtr::get_read_ptr_p(void)
{
  for (mem_area_list_t::iterator it = _l.begin();it != _l.end();it++)
    if ((*it)->is_active())
      return (*it)->get_read_ptr();

  return 0;
}

byte_t *
MemAreaPtr::get_write_ptr_p(void)
{
  for (mem_area_list_t::iterator it = _l.begin();it != _l.end();it++)
    if ((*it)->is_active())
      return (*it)->get_write_ptr();

  return 0;
}

Memory::Memory(void) : InterfaceCircuit("Memory")
{
  for (int a = 0;a < MemArea::PAGES;a++)
    _mem_ptr[a] = new MemAreaPtr();            
}

Memory::~Memory(void)
{
  for (int a = 0;a < MemArea::PAGES;a++)
    delete _mem_ptr[a];
}

void
Memory::loadROM(const char *filename, void *buf, long len, int force)
{
    ifstream is;

    is.open(filename, ios::in | ios::binary);
    if (!is) {
	if (!force) return;
	cerr << "can't open file '" << filename << "'\n";
	exit(1);
    }
    is.read((char *)buf, len);
    if (!is) {
	if (force) {
	    cerr << "error while reading '" << filename << "'\n";
	    exit(1);
	}
    }
    is.close();
}

void
Memory::loadRAM(const char *filename)
{
  if (filename == NULL)
    return;

  long addr = 0;
  char *ptr = strstr(filename, "0x");
  if (ptr != NULL)
    addr = strtol(ptr, NULL, 0) & 0xffff;

  loadRAM(filename, addr);
}

void
Memory::loadRAM(const char *filename, word_t addr)
{
  ifstream is;
    
  is.open(filename, ios::in | ios::binary);
  if (!is)
    return;

  loadRAM(&is, addr);
}

bool
Memory::loadRAM(istream *is, word_t addr)
{
  int c;

  while (242)
    {
      c = is->get();
      if (c == EOF)
	break;

      memWrite8(addr++, c);
    }

  return true;
}

bool
Memory::loadRAM_Z1013(istream *is, word_t addr)
{
  int a, c;

  /*
   *  skip header
   */
  for (a = 0;a < 36;a++)
    c = is->get();

  a = 0;
  while (242)
    {
      c = is->get();
      if (c == EOF)
	break;

      if (((a % 36) > 1) && ((a % 36) < 34))
	memWrite8(addr++, c);

      a++;
    }

  return true;
}

bool
Memory::loadRAM(istream *is, bool with_block_nr)
{
  int a, c, idx;
  unsigned char buf[129], *ptr;
  unsigned short load_addr, end_addr;

  /* read header */
  if (with_block_nr)
    {
      for (a = 0;a < 129;a++)
	buf[a] = is->get(); // workaround for memstream problems
      ptr = &buf[1];
    }
  else
    {
      for (a = 0;a < 128;a++)
	buf[a] = is->get(); // workaround for memstream problems
      ptr = &buf[0];
    }
  if (!(*is))
    return false;

  a = 0;
  if ((ptr[0] == 0xd3) && (ptr[1] == 0xd3) && (ptr[2] == 0xd3))
    {
      load_addr = 0x0401;
      end_addr = load_addr + (ptr[11] | (ptr[12] << 8));
      switch (get_kc_type())
	{
	case KC_TYPE_85_1:
	case KC_TYPE_87:
          memWrite8(0x03d7, end_addr & 0xff);
          memWrite8(0x03d8, end_addr >> 8);
	  break;

	case KC_TYPE_85_2:
	case KC_TYPE_85_3:
	case KC_TYPE_85_4:
          memWrite8(0x03d7, end_addr & 0xff);
          memWrite8(0x03d8, end_addr >> 8);
          memWrite8(0x03d9, end_addr & 0xff);
          memWrite8(0x03da, end_addr >> 8);
          memWrite8(0x03db, end_addr & 0xff);
          memWrite8(0x03dc, end_addr >> 8);
	  break;
	default:
	  break;
        }

      for (;a < 115;a++)
	memWrite8(load_addr++, ptr[a + 13]);
    }
  else
    {
      load_addr  = (ptr[17] & 0xff) | ((ptr[18] << 8) & 0xff00);
      //cout << __PRETTY_FUNCTION__ << ": load_addr = " << load_addr << endl;
    }

  idx = 0;
  while (242)
    {
      c = is->get();
      if (c == EOF)
        break;
      if ((!with_block_nr) || (idx > 0))
        {
          memWrite8(load_addr++, c);
          a++;
        }
      idx++;
      if (idx == 129)
        idx = 0;
    }

  //cout << __PRETTY_FUNCTION__ << ": len = " << a << endl;

  return true;
}

MemAreaPtr **
Memory::get_mem_ptr(void)
{
  return _mem_ptr;
}

MemAreaGroup *
Memory::register_memory(const char *name, word_t addr, dword_t size,
                        byte_t *mem, int prio, bool ro)
{
  MemAreaGroup *group;

  group = new MemAreaGroup(name, addr, size, mem, prio, ro);
  group->add(get_mem_ptr());
  group->set_active(true);
  reload_mem_ptr();
  
  return group;
}

void
Memory::unregister_memory(MemAreaGroup *group)
{
  group->remove(get_mem_ptr());
  reload_mem_ptr();
  delete group;
}

void
Memory::reload_mem_ptr(void)
{
  int a;
  
  for (a = 0;a < MemArea::PAGES;a++)
    {
      _memrptr[a] = _mem_ptr[a]->get_read_ptr();
      _memwptr[a] = _mem_ptr[a]->get_write_ptr();
    }
}

void *
Memory::get_page_addr_r(word_t addr)
{
  word_t a = addr >> MemArea::PAGE_SHIFT;
  return _memrptr[a];
}

void *
Memory::get_page_addr_w(word_t addr)
{
  word_t a = addr >> MemArea::PAGE_SHIFT;
  return _memwptr[a];
}

void
Memory::scratch_mem(byte_t *ptr, int len)
{
  srand(time(NULL));
  while (len-- > 0)
    *ptr++ = (byte_t)(256.0 * rand() / (RAND_MAX + 1.0));
}

void
Memory::dump(word_t addr)
{
  int a, b, c;
  for (a = 0;a < 8;a++)
    {
      cerr << hex << setw(4) << setfill('0') << (addr + 16 * a) << ": ";
      for (b = 0;b < 16;b++)
	{
	  cerr << hex << setw(2) << setfill('0') << memRead8(addr + 16 * a + b) << " ";
	  if (b == 7) cerr << ": ";
	}
      cerr << "   ";
      for (b = 0;b < 16;b++)
	{
	  c = memRead8(addr + 16 * a + b);
	  if (c > 0x20 && c < 0x80)
	    {
	      cerr << (char)c;
	    }
	  else
	    {
	      cerr << '.';
	    }
	}
      cerr << endl;
    }
}

void
Memory::info(void)
{
  int a;
  word_t addr;
  
  cerr << "  Memory:" << endl;
  cerr << "  -------" << endl << endl;

  addr = 0;
  for (a = 0;a < MemArea::PAGES;a++)
    {
      if (_mem_ptr[a]->size() > 1)
        {
	  cerr << "  " << hex << setw(4) << setfill('0') << addr << "h:";
          _mem_ptr[a]->info();
	  cerr << endl;
        }
      addr += MemArea::PAGE_SIZE;
    }
  cerr << endl << endl;
}
