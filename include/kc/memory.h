/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: memory.h,v 1.7 2000/05/21 17:33:58 tp Exp $
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

#ifndef __kc_memory_h
#define __kc_memory_h

#include <list.h>
#include <iostream.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/ic.h"

typedef enum { MEM_DISABLED, MEM_WRITE_PROTECT, MEM_ENABLED } MemState;

class MemArea;
class MemAreaPtr;
class MemAreaGroup;

class MemArea
{
public:
  enum page_vals {
    PAGE_SHIFT = 10,
    PAGES      = 64,
    PAGE_MASK  = 0x3ff,
    PAGE_SIZE  = 0x400
  };
  
private:
  int _prio;
  word_t _addr;
  bool _active;
  bool _readonly;
  byte_t *_mem, *_mem_r, *_mem_w;
  static byte_t *_scratch_r, *_scratch_w;
  MemAreaGroup *_group;

public:
  MemArea(MemAreaGroup *group, byte_t *mem, word_t addr,
          int prio, bool ro);
  virtual ~MemArea(void);

  const char * get_name(void);
  word_t get_addr(void);
  int get_prio(void);
  void set_active(bool active);
  void set_readonly(bool ro);
  inline bool is_active(void) { return _active; }
  inline bool is_readonly(void) { return _readonly; }
  inline byte_t * get_read_ptr(void) { return _mem_r; }
  inline byte_t * get_write_ptr(void) { return _mem_w; }
  static inline int PAGE_INDEX(dword_t addr) { return addr >> PAGE_SHIFT; }
  static inline word_t PAGE_ADDR(int idx) { return idx << PAGE_SHIFT; }
};

class MemAreaGroup
{
private:
  typedef list<MemArea *> mem_area_list_t;

private:
  int _prio;
  bool _active;
  bool _readonly;
  word_t _addr;
  dword_t _size;
  byte_t *_mem;
  char *_name;
  mem_area_list_t _l;

public:
  MemAreaGroup(const char *name, word_t addr, dword_t size,
               byte_t *mem, int prio, bool ro);
  virtual ~MemAreaGroup(void);

  void add(MemAreaPtr *area_ptr[]);
  void remove(MemAreaPtr *area_ptr[]);
  void set_active(bool active);
  void set_readonly(bool ro);
  inline const char *get_name(void) { return _name; }
};

class MemAreaPtr
{
private:
  typedef list<MemArea *> mem_area_list_t;

  mem_area_list_t _l;
  MemArea *_ptr;

protected:
  byte_t * get_read_ptr_p(void);
  byte_t * get_write_ptr_p(void);
  
public:
  MemAreaPtr(void);
  void add(MemArea *area);
  void remove(MemArea *area);
  void info(void);
  inline int size(void)
    {
      return _l.size();
    }
  inline byte_t * get_read_ptr(void)
    {
      return _ptr ? _ptr->get_read_ptr() : get_read_ptr_p();
    }
  inline byte_t * get_write_ptr(void)
    {
      return _ptr ? _ptr->get_write_ptr() : get_write_ptr_p();
    }
};

class Memory : public InterfaceCircuit
{
private:
  MemAreaPtr *_mem_ptr[MemArea::PAGES];

public:
  byte_t *_memrptr[MemArea::PAGES];
  byte_t *_memwptr[MemArea::PAGES];

protected:
  virtual void loadROM(const char *filename, void *buf, long len, int force);
  virtual void loadRAM(const char *filename, word_t addr);
  void reload_mem_ptr(void);
  void * get_page_addr_r(word_t addr);
  void * get_page_addr_w(word_t addr);

public:
  Memory(void);
  virtual ~Memory(void) {}
    
  virtual void dump(word_t addr);
  virtual void loadRAM(const char *filename);
  virtual bool loadRAM(istream *is, bool with_block_nr = false);
  virtual void scratch_mem(byte_t *ptr, int len);
  virtual void info(void);
  virtual MemAreaPtr ** get_mem_ptr(void);
  virtual MemAreaGroup * register_memory(const char *name,
                                         word_t addr, dword_t size,
                                         byte_t *mem, int prio,
                                         bool ro);
  virtual void unregister_memory(MemAreaGroup *group);
    
  virtual void dumpCore(void)                     = 0;
  virtual byte_t memRead8(word_t addr)            = 0;
  virtual void memWrite8(word_t addr, byte_t val) = 0;
  virtual byte_t * getIRM(void)                   = 0;

  virtual void iei(byte_t val) {}
  virtual void reti(void) {}
  virtual void reset(bool power_on = false) = 0;
};

#endif /* __kc_memory_h */
