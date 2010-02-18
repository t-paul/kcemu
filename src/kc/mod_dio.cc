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

#include "kc/daisy.h"
#include "kc/ctc_dio.h"
#include "kc/pio_dio.h"
#include "kc/mod_dio.h"

ModuleDIO::ModuleDIO(ModuleDIO &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _ctc = new CTC_DIO;
  _pio = new PIO_DIO;
  _portg_ctc = NULL;
  _portg_pio = NULL;

  set_valid(true);
}

ModuleDIO::ModuleDIO(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3)
{
  _ctc = NULL;
  _pio = NULL;
  _portg_ctc = NULL;
  _portg_pio = NULL;

  set_valid(true);
}

ModuleDIO::~ModuleDIO(void)
{
  if (_ctc != NULL)
    delete _ctc;
  if (_pio != NULL)
    delete _pio;
}

/*
byte_t
ModuleDIO::m_in(word_t addr)
{
  
  return 0x00;
  }*/

void
ModuleDIO::m_out(word_t addr, byte_t val)
{
  if (val & 1)
    {
      if (_portg_ctc == NULL)
	{
	  _portg_ctc = ports->register_ports("CTC DIO", 0x00, 4, _ctc, 0);
	  daisy->add_last(_ctc);
	}

      if (_portg_pio == NULL)
	{
	  _portg_pio = ports->register_ports("PIO DIO", 0x04, 4, _pio, 0);
	  daisy->add_last(_pio);
	}
    }
  else
    {
      if (_portg_ctc != NULL)
	{
	  ports->unregister_ports(_portg_ctc);
	  daisy->remove(_ctc);
	}
      _portg_ctc = NULL;

      if (_portg_pio != NULL)
	{
	  ports->unregister_ports(_portg_pio);
	  daisy->remove(_pio);
	}
      _portg_pio = NULL;
    }
}

ModuleInterface *
ModuleDIO::clone(void)
{
  return new ModuleDIO(*this);
}

void
ModuleDIO::reset(bool power_on)
{
}
