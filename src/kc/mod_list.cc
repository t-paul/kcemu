/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: mod_list.cc,v 1.11 2001/04/29 22:01:13 tp Exp $
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
 *  a7h: Floppy Disk Basis ROM -- always at slot fch!
 *  efh: M001 Digital IN/OUT
 *  eeh: M003 V24
 *  ---: M005 User (empty modul)
 *  ---: M007 Adapter (empty modul)
 *  e7h: M010 ADU1
 *  f6h: M011 64k RAM
 *  fbh: M012 Texor
 *  f4h: M022 Expander RAM (16k)
 *  f7h: M025 User PROM (8k)
 *  fbh: M026 Forth
 *  fbh: M027 Development
 *  e3h: M029 DAU1
 */

#include <string.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/rc.h"
#include "kc/mod_ram.h"
#include "kc/mod_ram1.h"
#include "kc/mod_64k.h"
#include "kc/mod_rom.h"
#include "kc/mod_disk.h"
#include "kc/mod_v24.h"
#include "kc/mod_list.h"

#include "ui/ui.h"

#include "libdbg/dbg.h"

ModuleList::ModuleList(void)
{
  int a;
  char *ptr;
  const char *mod;
  ModuleInterface *m;
  ModuleListEntry *entry;
  
  /*
   *  placeholder for a not present module ;-)
   */
  m = 0;
  _mod_list.push_back(new ModuleListEntry(_("<no module>"), m));

  /*
   *  V24 module
   */
  if (RC::instance()->get_int("Enable V24-Module"))
    {
      m = new ModuleV24("M003", 0xee);
      entry = new ModuleListEntry(_("M003: V24 (not working!)"), m);
      _mod_list.push_back(entry);
    }

  /*
   *  RAM module 64k
   */
  m = new Module64k("M011", 0xf6);
  _mod_list.push_back(new ModuleListEntry(_("M011: 64k RAM"), m));
  
  /*
   *  RAM modules 16k at 4000h and 8000h (kc85/1)
   */
  m = new ModuleRAM1("RAM4", 0x4000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/4000h)"), m));
  m = new ModuleRAM1("RAM8", 0x8000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/8000h)"), m));

  /*
   *  IRM Expansion for color display (kc85/1)
   */
  m = new ModuleRAM1("IRMX", 0xe800, 0x0800);
  _mod_list.push_back(new ModuleListEntry(_("IRM Color Expansion"), m));

  /*
   *  RAM module 16k (kc85/3)
   */
  m = new ModuleRAM("M022", 0xf4);
  _mod_list.push_back(new ModuleListEntry(_("M022: Expander RAM (16k)"), m));

  /*
   *  basic
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m006.rom");
  m = new ModuleROM(ptr, "Basic", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M006: Basic"), m));
  delete ptr;

  /*
   *  texor
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m012.rom");
  m = new ModuleROM(ptr, "M012", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M012: Texor"), m));
  delete ptr;

  /*
   *  forth
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m026.rom");
  m = new ModuleROM(ptr, "M026", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M026: Forth"), m));
  delete ptr;

  /*
   *  development
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m027.rom");
  m = new ModuleROM(ptr, "M027", 0x2000, 0xfb);
  entry = new ModuleListEntry(_("M027: Development"), m);
  _mod_list.push_back(entry);
  delete ptr;

  /*
   *  wordpro ROM version for kc85/3
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m900.rom");
  m = new ModuleROM(ptr, "M900", 0x2000, 0xfb);
  entry = new ModuleListEntry(_("M900: WordPro '86 (KC85/3)"), m);
  _mod_list.push_back(entry);
  delete ptr;

  /*
   *  wordpro ROM version for kc85/4
   */
  ptr = new char[strlen(kcemu_datadir) + 10];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/m901.rom");
  m = new ModuleROM(ptr, "M901", 0x2000, 0xfb);
  entry = new ModuleListEntry(_("M901: WordPro '86 (KC85/4)"), m);
  _mod_list.push_back(entry);
  delete ptr;

  /*
   *  Floppy Disk Basis ROM
   */
  ptr = new char[strlen(kcemu_datadir) + 12];
  strcpy(ptr, kcemu_datadir);
  strcat(ptr, "/floppy.rom");
  m = new ModuleDisk(ptr, "Floppy Disk Basis", 0x2000, 0xa7);
  entry = new ModuleListEntry(_("Floppy Disk Basis"), m);
  _mod_list.push_back(entry);
  _init_floppy_basis = 0;
  if (RC::instance()->get_int("Floppy Disk Basis"))
    {
      _init_floppy_basis = entry;
    }
  delete ptr;

  _nr_of_bd = RC::instance()->get_int("Busdrivers");
  if (_nr_of_bd < 0) _nr_of_bd = 0;
  if (_nr_of_bd > MAX_BD) _nr_of_bd = MAX_BD;

  for (a = 0;a < 4 * _nr_of_bd + 2;a++)
    {
      mod = RC::instance()->get_string_i(a, "Module");
      _init_mod[a] = mod;
    }
}

ModuleList::~ModuleList(void)
{
  ModuleListEntry *entry;
  ModuleList::iterator it;

  for (it = module_list->begin();it != module_list->end();it++)
    {
      entry = *it;
      delete entry->get_mod();
      delete entry;
    }
}

void
ModuleList::init(void)
{
  int a;
  ModuleInterface *m;
  ModuleList::iterator it;

  for (a = 0;a < 4 * _nr_of_bd + 2;a++)
    {
      if (!_init_mod[a]) continue;
      for (it = module_list->begin();it != module_list->end();it++)
        {
          m = (*it)->get_mod();
          if (!m) continue;
          if (strcmp(m->get_name(), _init_mod[a]) == 0)
            insert(a, *it);
        }
    }
  if (_init_floppy_basis)
    {
      /* floppy disk basis ROM is always in slot fch! */
      insert(61, _init_floppy_basis);
    }
}

void
ModuleList::insert(int slot, ModuleListEntry *entry)
{
  ModuleInterface *m;
  
  /*
   *  the module object is deleted in module->insert()
   *  or module->remove()!
   */
  m = entry->get_mod();

  if (m)
    module->insert(slot, m->clone());
  else
    module->remove(slot);

  ui->getModuleInterface()->insert(slot, m);
  ui->getModuleInterface()->activate(slot, 0);
}
