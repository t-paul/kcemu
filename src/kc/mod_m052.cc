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

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/vdip.h"
#include "kc/daisy.h"
#include "kc/pio_vdip.h"
#include "kc/mod_m052.h"
#include "kc/kcnet/kcnet.h"
#include "kc/kcnet/pio_net.h"

ModuleNetworkUSB::ModuleNetworkUSB(ModuleNetworkUSB &tmpl) :
  ModuleSegmentedMemory(tmpl)
{
  _pio_net = new PIO_NET;
  _pio_vdip = new PIO_VDIP;
  _portg_pio_net = NULL;
  _portg_pio_vdip = NULL;

  set_valid(true);
}

ModuleNetworkUSB::ModuleNetworkUSB(const char *filename, const char *name, byte_t id) :
  ModuleSegmentedMemory(name, id, 4, 0x2000, filename)
{
  _pio_net = NULL;
  _pio_vdip = NULL;
  _portg_pio_net = NULL;
  _portg_pio_vdip = NULL;

  set_valid(true);
}

ModuleNetworkUSB::~ModuleNetworkUSB(void)
{
  if (_pio_net != NULL)
    delete _pio_net;
  if (_pio_vdip != NULL)
    delete _pio_vdip;
}

int
ModuleNetworkUSB::get_segment_index(word_t addr, byte_t val)
{
  return (val >> 3) & 3;
}

word_t
ModuleNetworkUSB::get_base_address(word_t addr, byte_t val)
{
  return ((val >> 5) & 7) * 0x2000;
}

byte_t
ModuleNetworkUSB::m_in(word_t addr)
{
  byte_t val = ModuleSegmentedMemory::m_in(addr);
  printf("ModuleNetworkUSB::m_in():  %04xh: %04x - %02x\n", z80->getPC(), addr, val);
  return val;
}

void
ModuleNetworkUSB::m_out(word_t addr, byte_t val)
{
  printf("ModuleNetworkUSB::m_out(): %04xh: %04x - %02x\n", z80->getPC(), addr, val);
  ModuleSegmentedMemory::m_out(addr, val);

  if (val & 4)
    {
      if (_portg_pio_net == NULL)
	{
	  _portg_pio_net = ports->register_ports("M052 PIO NET", 0x28, 4, _pio_net, 0);
	  daisy->add_last(_pio_net);
	}
      if (_portg_pio_vdip == NULL)
        {
	  _portg_pio_vdip = ports->register_ports("M052 PIO VDIP", 0x2c, 4, _pio_vdip, 0);
	  daisy->add_last(_pio_vdip);
          vdip->register_pio(_pio_vdip);
        }
    }
  else
    {
      if (_portg_pio_net != NULL)
	{
	  ports->unregister_ports(_portg_pio_net);
	  daisy->remove(_pio_net);
          kcnet->reset();
	}
      if (_portg_pio_vdip != NULL)
        {
          vdip->register_pio(NULL);
	  ports->unregister_ports(_portg_pio_vdip);
	  daisy->remove(_pio_vdip);
        }
      _portg_pio_net = NULL;
      _portg_pio_vdip = NULL;
    }
}

ModuleInterface *
ModuleNetworkUSB::clone(void)
{
  return new ModuleNetworkUSB(*this);
}
