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

#include <ctype.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/vis.h"

#include "libdbg/dbg.h"

VIS::VIS(void) : InterfaceCircuit("VIS")
{
  reset(true);
  z80->register_ic(this);
}

VIS::~VIS(void)
{
  z80->unregister_ic(this);
}

byte_t
VIS::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 0xff)
    {
    case 0x9c:
      val = _char[_idx];
      _idx = (_idx + 1) & 0x07ff;
      break;
    }

  DBG(2, form("KCemu/VIS/in",
              "VIS::in():  %04xh val = %02x\n",
              addr, val));

  return val;
}

/*
 *  screen 0/8:
 *  
 *    VIS::out(): 129dh val = 12 [.]  Text 25x40
 *    VIS::out(): 009dh val = 00 [.]
 *
 *  screen 1/9:
 *
 *    VIS::out(): 109dh val = 10 [.]  Text 25x80
 *    VIS::out(): 009dh val = 00 [.]
 *
 *  screen 2:
 *
 *    VIS::out(): 129dh val = 12 [.]  Graphic 360 x 200 (4 of 16 Colors)
 *    VIS::out(): 019dh val = 01 [.]
 *
 *  screen 3:
 *
 *    VIS::out(): 109dh val = 10 [.]  Graphic 640 x 200 (4 of 16 Colors)
 *    VIS::out(): 019dh val = 01 [.]
 *
 *  screen 5:
 *
 *    VIS::out(): 109dh val = 10 [.]  Graphic 320 x 200 (16 of 16 Colors)
 *    VIS::out(): 029dh val = 02 [.]
 *
 *  border color:
 *    
 *    VIS::out(): 209dh / command:   val = 20 [ ] -> border color =  0
 *    VIS::out(): 219dh / command:   val = 21 [!] -> border color =  1
 *    VIS::out(): 229dh / command:   val = 22 ["] -> border color =  2
 *    VIS::out(): 239dh / command:   val = 23 [#] -> border color =  3
 *    VIS::out(): 2f9dh / command:   val = 2f [/] -> border color = 15
 *
 *  COLOR 0,0
 *    VIS::out(): 309dh / command:   val = 30 [0]
 *    VIS::out(): 029dh / command:   val = 42 [B]
 *    VIS::out(): 019dh / command:   val = 54 [T]
 *    VIS::out(): 009dh / command:   val = 66 [f]
 *
 *  COLOR 0,1
 *    VIS::out(): 309dh / command:   val = 30 [0]
 *    VIS::out(): 029dh / command:   val = 43 [C]
 *    VIS::out(): 019dh / command:   val = 55 [U]
 *    VIS::out(): 009dh / command:   val = 67 [g]
 *
 *  COLOR 1,0
 *    VIS::out(): 319dh / command:   val = 31 [1]
 *    VIS::out(): 029dh / command:   val = 42 [B]
 *    VIS::out(): 019dh / command:   val = 54 [T]
 *    VIS::out(): 009dh / command:   val = 66 [f]
 *
 *  COLOR 1,1
 *    VIS::out(): 319dh / command:   val = 31 [1]
 *    VIS::out(): 029dh / command:   val = 43 [C]
 *    VIS::out(): 019dh / command:   val = 55 [U]
 *    VIS::out(): 009dh / command:   val = 67 [g]
 *
 *  COLOR 2,0
 *    VIS::out(): 329dh / command:   val = 32 [2]
 *    VIS::out(): 029dh / command:   val = 42 [B]
 *    VIS::out(): 019dh / command:   val = 54 [T]
 *    VIS::out(): 009dh / command:   val = 66 [f]
 *    
 *  COLOR 2,1
 *    VIS::out(): 329dh / command:   val = 32 [2]
 *    VIS::out(): 029dh / command:   val = 43 [C]
 *    VIS::out(): 019dh / command:   val = 55 [U]
 *    VIS::out(): 009dh / command:   val = 67 [g]
 */
void
VIS::out(word_t addr, byte_t val)
{
  static byte_t last_val = 0;

  DBG(2, form("KCemu/VIS/out",
              "VIS::out(): %04xh val = %02x [%c]\n",
              addr, val, isprint(val) ? val : '.'));

  switch (addr & 0xff)
    {
    case 0x9c:
      DBG(2, form("KCemu/VIS/out_9C",
		  "VIS::out(): %04xh / char data: val = %02x [%c] %c%c%c%c%c%c%c%c\n",
		  addr, val, isprint(val) ? val : '.',
		  (val &   1) ? '#' : ' ',
		  (val &   2) ? '#' : ' ',
		  (val &   4) ? '#' : ' ',
		  (val &   8) ? '#' : ' ',
		  (val &  16) ? '#' : ' ',
		  (val &  32) ? '#' : ' ',
		  (val &  64) ? '#' : ' ',
		  (val & 128) ? '#' : ' '));
      _char[_idx] = val;
      _changed[_idx / 8] = 1;
      _idx = (_idx + 1) & 0x07ff;
      break;
    case 0x9d:
      DBG(2, form("KCemu/VIS/out_9D",
		  "VIS::out(): %04xh / command:   val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));

      if ((last_val == 0x12) && (val == 0x00))
	set_mode(0);
      else if ((last_val == 0x10) && (val == 0x00))
	set_mode(1);
      else if ((last_val == 0x12) && (val == 0x01))
	set_mode(2);
      else if ((last_val == 0x10) && (val == 0x01))
	set_mode(3);
      else if ((last_val == 0x10) && (val == 0x02))
	set_mode(5);
      else if ((val & 0xf0) == 0x20)
	set_border(val & 0x0f);
      else if ((val & 0xf0) == 0x30)
	set_color(0, val & 0x0f);
      else if ((val & 0xf0) == 0x40)
	set_color(1, val & 0x0f);
      else if ((val & 0xf0) == 0x50)
	set_color(2, val & 0x0f);
      else if ((val & 0xf0) == 0x60)
	set_color(3, val & 0x0f);

      last_val = val;

      break;
    case 0x9e:
      DBG(2, form("KCemu/VIS/out_9E",
		  "VIS::out(): %04xh / char idx:  val = %02x (%d)\n",
		  addr, val, val));
      _idx = 8 * val;
      break;
    }
}

int
VIS::get_mode()
{
  return _mode;
}

void
VIS::set_mode(int mode)
{
  DBG(1, form("KCemu/VIS/mode",
	      "VIS::out(): mode change %d -> %d\n",
	      _mode, mode));
  _mode = mode;
}

int
VIS::get_border(void)
{
  return _border;
}

void
VIS::set_border(int border)
{
  DBG(1, form("KCemu/VIS/border",
	      "VIS::out(): border change %2d -> %2d\n",
	      _border, border));
  _border = border;
}

int
VIS::get_color(int idx)
{
  idx &= 3;
  return _color[idx];
}

void
VIS::set_color(int idx, int color)
{
  idx &= 3;
  DBG(1, form("KCemu/VIS/color",
	      "VIS::out(): color change [%d] %2d -> %2d [%2d/%2d/%2d/%2d]\n",
	      idx, _color[idx], color,
	      _color[0], _color[1], _color[2], _color[3]));

  if (_color[idx] != color)
    {
      _color[idx] = color;
      _color_palette_changed = 1;
    }
}

int
VIS::get_changed(byte_t idx)
{
  return _changed[idx];
}

void
VIS::reset_changed(void)
{
  memset(_changed, 0, 0x100);
}

int
VIS::is_color_palette_changed(void)
{
  return _color_palette_changed;
}

void
VIS::reset_color_palette_changed(void)
{
  _color_palette_changed = 0;
}

byte_t *
VIS::get_memory(void)
{
  return _char;
}

void
VIS::reti(void)
{
}

void
VIS::irqreq(void)
{
}

word_t
VIS::irqack(void)
{
  return IRQ_NOT_ACK;
}

void
VIS::reset(bool power_on)
{
  int a;
  
  _mode = 0;
  _border = 0;
  _color[0] = 0;
  _color[1] = 0;
  _color[2] = 0;
  _color[3] = 0;
  _color_palette_changed = 0;

  if (power_on)
    for (a = 0;a < 0x0800;a++)
      _char[a] = 0;
}
