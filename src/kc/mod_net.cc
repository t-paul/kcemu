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
#include "kc/daisy.h"
#include "kc/memory.h"
#include "kc/mod_net.h"
#include "kc/kcnet/pio_net.h"

#include "libdbg/dbg.h"

using namespace std;

ModuleNetwork::ModuleNetwork(ModuleNetwork &tmpl) : ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _pio = new PIO_NET();
  _portg = ports->register_ports("PIO KCNET", 0xc0, 4, _pio, 0);
  daisy->add_last(_pio);
  
  set_valid(true);
}

ModuleNetwork::ModuleNetwork(const char *name) : ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _pio = NULL;
  _portg = NULL;
  set_valid(true);
}

ModuleNetwork::~ModuleNetwork(void)
{
  if (_portg)
    ports->unregister_ports(_portg);
  if (_pio)
    {
      daisy->remove(_pio);
      delete _pio;
    }

  _pio = NULL;
  _portg = NULL;
}

void
ModuleNetwork::m_out(word_t addr, byte_t val)
{
}

ModuleInterface *
ModuleNetwork::clone(void)
{
  return new ModuleNetwork(*this);
}

void
ModuleNetwork::reset(bool power_on)
{
}
