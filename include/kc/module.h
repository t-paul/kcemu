/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: module.h,v 1.8 2001/04/14 15:14:35 tp Exp $
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

#ifndef __kc_module_h
#define __kc_module_h

#include "kc/config.h"
#include "kc/system.h"

#include "kc/ports.h"

typedef enum
{
  KC_MODULE_ALL, KC_MODULE_KC_85_1, KC_MODULE_KC_85_3,
} mod_type_t;

class ModuleInterface
{
private:
  bool _valid;
  byte_t _id;
  char *_name;
  mod_type_t _mod_type;

public:
  ModuleInterface(const char *name, byte_t id, mod_type_t mod_type);
  virtual ~ModuleInterface(void);

  virtual void out(word_t addr, byte_t val) = 0;
  virtual ModuleInterface * clone(void) = 0;

  virtual byte_t in(word_t addr);
  virtual byte_t get_id(void);
  virtual const char * get_name(void);
  virtual mod_type_t get_type(void);
  virtual void set_valid(bool valid);
  virtual bool is_valid(void);
};

class Module : public PortInterface
{
public:
  enum {
    NR_MODULES = 64,
  };
  ModuleInterface *_module[NR_MODULES];
 
public:
  Module();
  virtual ~Module(void);
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
  virtual void insert(byte_t slot, ModuleInterface *module);
  virtual void remove(byte_t slot);
};

#endif /* __kc_module_h */
