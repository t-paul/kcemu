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

#ifndef __kc_memory_h
#define __kc_memory_h

#include <list>
#include <iostream>

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
  bool _read_through;
  bool _write_through;
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
  void set_read_through(bool rt);
  void set_write_through(bool wt);
  inline bool is_active(void) { return _active; }
  inline bool is_readonly(void) { return _readonly; }
  inline bool is_read_through(void) { return _read_through; }
  inline bool is_write_through(void) { return _write_through; }
  inline byte_t * get_read_ptr(void) { return _mem_r; }
  inline byte_t * get_write_ptr(void) { return _mem_w; }
  static inline int PAGE_INDEX(dword_t addr) { return addr >> PAGE_SHIFT; }
  static inline word_t PAGE_ADDR(int idx) { return idx << PAGE_SHIFT; }
};

class MemAreaGroup
{
private:
  typedef std::list<MemArea *> mem_area_list_t;

private:
  int _prio;
  bool _active;
  bool _readonly;
  bool _read_through;
  bool _write_through;
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
  void set_read_through(bool rt);
  void set_write_through(bool wt);
  inline bool is_active(void) { return _active; }
  inline bool is_readonly(void) { return _readonly; }
  inline bool is_read_through(void) { return _read_through; }
  inline bool is_write_through(void) { return _write_through; }
  inline const char *get_name(void) { return _name; }
};

class MemAreaPtr
{
private:
  typedef std::list<MemArea *> mem_area_list_t;

  mem_area_list_t _l;

public:
  MemAreaPtr(void);
  void add(MemArea *area);
  void remove(MemArea *area);
  void info(void);
  inline int size(void)
    {
      return _l.size();
    }
  byte_t * get_read_ptr(void);
  byte_t * get_write_ptr(void);
};

typedef struct {
    MemAreaGroup **group;
    const char    *name;
    word_t         addr;
    dword_t        size;
    byte_t        *mem;
    int            prio;
    bool           ro;
    bool           active;
    int            model;
} memory_group_t;

class Memory : public InterfaceCircuit
{
private:
  MemAreaPtr *_mem_ptr[MemArea::PAGES];

  static unsigned int seed_x; /* the seeds for...        */
  static unsigned int seed_y; /* ...the pseudo random... */
  static unsigned int seed_z; /* ...number generator     */

public:
  byte_t *_memrptr[MemArea::PAGES];
  byte_t *_memwptr[MemArea::PAGES];

  static bool load_rom(const char *key, void *buf);
  static bool load_rom(const char *filename, void *buf, long len, bool force);

protected:
  static unsigned int mem_rand();
  static void mem_rand_seed(unsigned int seed1, unsigned int seed2, unsigned int seed3);

  void init_memory_groups(memory_group_t mem[]);
  virtual void loadRAM(const char *filename, word_t addr);

  void * get_page_addr_r(word_t addr);
  void * get_page_addr_w(word_t addr);

public:
  Memory(void);
  virtual ~Memory(void);

  static void scratch_mem(byte_t *ptr, int len);

  virtual void dump(word_t addr);
  virtual void loadRAM(const char *filename);
  virtual bool loadRAM(std::istream *is, word_t addr);
  virtual bool loadRAM_Z1013(std::istream *is, word_t addr);
  virtual bool loadRAM(std::istream *is, bool with_block_nr = false);
  virtual void info(void);
  virtual MemAreaPtr ** get_mem_ptr(void);
  virtual MemAreaGroup * register_memory(const char *name,
					 word_t addr, dword_t size,
					 byte_t *mem, int prio,
					 bool ro);
  virtual void unregister_memory(MemAreaGroup *group);
  virtual void reload_mem_ptr(void);

  virtual void dumpCore(void)                     = 0;
  virtual byte_t memRead8(word_t addr)            = 0;
  virtual void memWrite8(word_t addr, byte_t val) = 0;
  virtual byte_t * get_irm(void)                  = 0;
  virtual byte_t * get_char_rom(void)             = 0;

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void) {}
  virtual void irqreq(void) {}
  virtual word_t irqack() { return IRQ_NOT_ACK; }
  virtual void reset(bool power_on = false) = 0;
};

#endif /* __kc_memory_h */
