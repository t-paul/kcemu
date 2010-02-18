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

#ifndef __kc_mod_list_h
#define __kc_mod_list_h

#include <list>
#include <string.h>

#include <stdlib.h>

#include "kc/system.h"
#include "kc/prefs/types.h"

#include "kc/kc.h"
#include "kc/module.h"

using namespace std;

class ModuleListEntry
{
private:
  char *_name;
  kc_type_t _type;
  ModuleInterface *_mod;
  
public:
  ModuleListEntry(const char *name, ModuleInterface *mod, kc_type_t type)
    {
      _mod = mod;
      _type = type;
      _name = strdup(name);
    }
  virtual ~ModuleListEntry(void)
    {
      free(_name);
    }
  virtual ModuleInterface * get_mod(void) { return _mod; }
  virtual const char * get_name(void) { return _name; }
  virtual kc_type_t get_kc_type() { return _type; }
};

class ModuleList
{
private:
  enum {
    MAX_BD = 4,
  };

  int _nr_of_bd;
  char *_init_mod[4 * MAX_BD + 2];
  ModuleListEntry *_color_expansion;
  ModuleListEntry *_init_floppy_basis_f8;
  ModuleListEntry *_init_floppy_basis_fc;
  ModuleListEntry *_init_color_expansion;

public:
  typedef list<ModuleListEntry *> mod_list_t;
  typedef mod_list_t::iterator iterator;
    
private:
  mod_list_t _mod_list;

protected:
  virtual void add_custom_modules(void);
  virtual void init_modules(int max_modules);
  virtual int init_modules_autostart(int idx);
  virtual int init_modules_configfile(int idx, int max_modules);
  virtual int init_modules_commandline(int idx, int max_modules, const char *param);

public:
  ModuleList(void);
  virtual ~ModuleList(void);
  virtual void init(void);
  virtual int get_nr_of_busdrivers(void) { return _nr_of_bd; }

  ModuleListEntry * get_color_expansion();
  
  virtual void insert(int slot, ModuleListEntry *entry);
  virtual iterator begin(void) { return _mod_list.begin(); }
  virtual iterator end(void) { return _mod_list.end(); }
};

#endif /* __kc_mod_list_h */
