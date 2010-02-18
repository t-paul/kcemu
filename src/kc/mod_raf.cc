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

#include "kc/system.h"
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/memory.h"
#include "kc/mod_raf.h"

#include "libdbg/dbg.h"

using namespace std;

ModuleRAF::ModuleRAF(ModuleRAF &tmpl) : ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _port = tmpl._port;
  _size = tmpl._size;

  _ram[0] = new byte_t[_size];
  _ram[1] = new byte_t[_size];

  Memory::scratch_mem(_ram[0], _size);
  Memory::scratch_mem(_ram[1], _size);
  _portg = ports->register_ports(get_name(), _port, 4, this, 0);

  set_valid(true);
}

ModuleRAF::ModuleRAF(const char *name, byte_t port, dword_t size) : ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _ram[0] = NULL;
  _ram[1] = NULL;
  _port = port;
  _size = size / 2;
  _portg = NULL;
  set_valid(true);
}

ModuleRAF::~ModuleRAF(void)
{
  if (_ram[0])
    delete[] _ram[0];
  if (_ram[1])
    delete[] _ram[1];
  if (_portg)
    ports->unregister_ports(_portg);

  _ram[0] = NULL;
  _ram[1] = NULL;
  _portg = NULL;
}

void
ModuleRAF::m_out(word_t addr, byte_t val) { }

ModuleInterface *
ModuleRAF::clone(void)
{
  return new ModuleRAF(*this);
}

void
ModuleRAF::reset(bool power_on)
{
}

byte_t
ModuleRAF::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 3)
    {
    case 0:
      val = in_data(0, addr);
      break;
    case 1:
      val = in_ctrl(0, addr);
      break;
    case 2:
      val = in_data(1, addr);
      break;
    case 3:
      val = in_ctrl(1, addr);
      break;
    }

  DBG(2, form("KCemu/ModuleRAF/in",
              "ModuleRAF::in():  %04xh: %04x <- %02x\n",
              z80->getPC(), addr, val));
  return val;
}

byte_t
ModuleRAF::in_ctrl(int card, word_t addr)
{
  byte_t val = 0xff;

  DBG(2, form("KCemu/ModuleRAF/ctrl",
              "ModuleRAF::in_ctrl():  %04xh: card%d: %04x <- %02x\n",
              z80->getPC(), card, addr, val));

  return val;
}

byte_t
ModuleRAF::in_data(int card, word_t addr)
{
  byte_t val = 0xff;

  dword_t mem = _addr[card] | ((addr >> 8) & 0x7f);

  if (mem < _size)
    val = _ram[card][mem];
  
  DBG(2, form("KCemu/ModuleRAF/data",
              "ModuleRAF::in_data():  %04xh: card%d: addr = %06xh <- %02x\n",
              z80->getPC(), card, mem, val));

  return val;
}

void
ModuleRAF::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/ModuleRAF/out",
              "ModuleRAF::out(): %04xh: %04x -> %02x\n",
              z80->getPC(), addr, val));

  switch (addr & 3)
    {
    case 0:
      out_data(0, addr, val);
      break;
    case 1:
      out_ctrl(0, addr, val);
      break;
    case 2:
      out_data(1, addr, val);
      break;
    case 3:
      out_ctrl(1, addr, val);
      break;
    }
}

void
ModuleRAF::out_ctrl(int card, word_t addr, byte_t val)
{
  word_t ts = (addr & 0xff00) | val;

  _prot[card] = (ts >> 15) | (ts >> 14);
  _addr[card] = (ts & 0x3fff) << 7;

  DBG(2, form("KCemu/ModuleRAF/ctrl",
              "ModuleRAF::out_ctrl(): %04xh: card%d: addr = %06xh, protection = %s\n",
              z80->getPC(), card, _addr[card], _prot[card] ? "ON" : "OFF"));
}

void
ModuleRAF::out_data(int card, word_t addr, byte_t val)
{
  dword_t mem = _addr[card] | ((addr >> 8) & 0x7f);

  DBG(2, form("KCemu/ModuleRAF/data",
              "ModuleRAF::out_data(): %04xh: card%d: addr = %06xh -> %02x%s\n",
              z80->getPC(), card, mem, val, _prot[card] ? " protection = ON": ""));

  if ((_prot[card] == 0) && (mem < _size))
    _ram[card][mem] = val;
}
