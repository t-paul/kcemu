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

#ifndef __kc_module_h
#define __kc_module_h

#include "kc/system.h"

#include "kc/ports.h"

typedef enum
{
  KC_MODULE_ALL,
  KC_MODULE_KC_85_1,
  KC_MODULE_KC_85_3,
  KC_MODULE_LC_80,
  KC_MODULE_Z1013,
  KC_MODULE_A5105,
} mod_type_t;

class ModuleInterface
{
private:
  bool _valid;
  byte_t _id;
  char *_name;
  char *_error_text;
  mod_type_t _mod_type;

public:
  ModuleInterface(const char *name, byte_t id, mod_type_t mod_type);
  virtual ~ModuleInterface(void);

  virtual byte_t m_in(word_t addr);
  virtual void m_out(word_t addr, byte_t val) = 0;
  virtual ModuleInterface * clone(void) = 0;
  virtual void reset(bool power_on = false) = 0;

  virtual byte_t get_id(void);
  virtual const char * get_name(void);
  virtual mod_type_t get_type(void);
  virtual void set_valid(bool valid);
  virtual bool is_valid(void);

  virtual const char * get_error_text(void);
  virtual void set_error_text(const char *text);
};

class Module : public PortInterface
{
public:
  enum {
    NR_MODULES = 64,
  };
  ModuleInterface *_module[NR_MODULES];

private:
  int get_slot_by_addr(word_t addr);
  ModuleInterface * get_module_by_addr(word_t addr);

public:
  Module();
  virtual ~Module(void);
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
  virtual bool is_empty(byte_t slot);
  virtual void insert(byte_t slot, ModuleInterface *module);
  virtual void remove(byte_t slot);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_module_h */
