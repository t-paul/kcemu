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

#include <stdio.h>

#include "kc/poly880.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

/*
 *  partial implementation for the poly880 reset / single step / NMI logic
 *
 *  Currently only the CTC triggered NMI flip flop is emulated. It's set
 *  by the ZC/TO0 value of CTC channel 0 and can be reset by PIO 1 port B
 *  bit 6 (SCON).
 *
 */
Poly880::Poly880(void)
{
  _scon = false;
  _nmi_flip_flop = false;
}

Poly880::~Poly880(void)
{
}

void
Poly880::set_scon(bool value)
{
  if (_scon == value)
    return;

  DBG(2, form("KCemu/Poly880/set_scon",
              "Poly880::set_scon(): value = %d%s\n",
              value,
	      _scon ? "" : " - NMI Flip-Flop reset"));

  if (!_scon)
    _nmi_flip_flop = false;

  _scon = value;
}

void
Poly880::trigger_send(void)
{
  if (_nmi_flip_flop)
    return;

  DBG(2, form("KCemu/Poly880/trigger_send",
              "Poly880::trigger_send() - NMI Flip-Flop set\n"));

  _nmi_flip_flop = true;
  CMD_EXEC("emu-nmi");
}
