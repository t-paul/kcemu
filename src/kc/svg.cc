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
#include "kc/svg.h"
#include "kc/tape.h"
#include "kc/keyb9.h"
#include "kc/memory9.h"

#include "libdbg/dbg.h"

/*
 *
 * +-----+---------------------+-----------------------------------+
 * | REG |      Function       |                Bit                |
 * |     |                     |   7 | 6 | 5 | 4 | 3 | 2 | 1 | 0   |
 * +-----+---------------------+-----------------------------------+
 * |   0 | Tonfrequenz Kanal A |              FL (A)               |
 * |   1 |                     |   x | x | x | x |      FH(A)      |
 * +-----+---------------------+-----------------------------------+
 * |   2 | Tonfrequenz Kanal B |              FL (B)               |
 * |   3 |                     |   x | x | x | x |      FH(B)      |
 * +-----+---------------------+-----------------------------------+
 * |   4 | Tonfrequenz Kanal C |              FL (C)               |
 * |   5 |                     |   x | x | x | x |      FH(C)      |
 * +-----+---------------------+-----------------------------------+
 * |   6 | Rauschfrequenz      |   x | x | x |          RP         |
 * +-----+---------------------+-----------------------------------+
 * |   7 | Auswahl Ton- bzw.   |         |  Rauschen |    Ton      |
 * |     | Rauschkanal         |   x | x | C | B | A | C | B | A   |
 * +-----+---------------------+-----------------------------------+
 * |   8 |                   A |   x | x | x | H |      L(A)       |
 * |   9 | Lautst�rke Kanal  B |   x | x | x | H |      L(B)       |
 * |  10 |                   C |   x | x | x | H |      L(C)       |
 * +-----+---------------------+-----------------------------------+
 * |  11 | H�llkurvenfrequenz  |              FHL                  |
 * |  12 |                     |              FHH                  |
 * +-----+---------------------+-----------------------------------+
 * |  13 | H�llkurvenform      |   x | x | x |          HKF        |
 * +-----+---------------------+-----------------------------------+
 *
 *  Parameter f�r Tonfrequenz
 *    FW    = 3750000 / (32 + F)
 *    FL(n) = FW | 256
 *    FH(n) = FW mod 256
 *    F - reale Tonfrequenz (in Hz)
 *  Rauschparameter
 *    RP = 3750000 / 32 * F
 *    RF - Rauschparameter (0...31)
 *    F - maximaler Frequenzanteil im Rauschen
 *
 */
SVG::SVG(void)
{
  _val_a0 = 0;
  _val_a1 = 0;
  _val_a8 = 0;
  _val_aa = 0;
  _val_ab = 0;

  for (int a = 0;a < 4;a++)
    _page_info[a] = 0;

  for (int a = 0;a < 14;a++)
    _sound_reg[a] = 0;

  for (int a = 0;a < 3;a++)
    _sound_val[a] = 0;

  for (int a = 0;a < 3;a++)
    _sound_freq[a] = 0;
}

SVG::~SVG(void)
{
}

byte_t
SVG::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 0xff)
    {
    case 0xa0:
      val = _val_a0;
      DBG(2, form("KCemu/SVG/in_A8",
		  "SVG::in(a0h):  %04xh val = %02x (%d)\n",
		  addr, val, val));
      break;
    case 0xa1:
      val = _val_a1;
      DBG(2, form("KCemu/SVG/in_A1",
		  "SVG::in(a1h):  %04xh val = %02x (%d)\n",
		  addr, val, val));
      break;
    case 0xa8:
      val = _val_a8;
      DBG(2, form("KCemu/SVG/in_A8",
		  "SVG::in(a8h):  %04xh val = %02x (%d)\n",
		  addr, val, val));
      break;
    case 0xa9:
      /*
       *  keyboard input
       */
      val = ((Keyboard9 *)keyboard)->get_val();
      DBG(2, form("KCemu/SVG/in_A9",
		  "SVG::in(a9h):  %04xh val = %02x (%d)\n",
		  addr, val, val));
      break;
    case 0xaa:
      val = _val_aa;
      DBG(2, form("KCemu/SVG/in_AA",
		  "SVG::in(aah):  %04xh val = %02x (%d)\n",
		  addr, val, val));
      break;
    case 0xab:
      val = _val_ab;
      DBG(2, form("KCemu/SVG/in_AB",
		  "SVG::in(abh):  %04xh val = %02x (%d)\n",
		  addr, val, val));
      break;
    }

  DBG(2, form("KCemu/SVG/in",
              "SVG::in(%02xh):  %04xh val = %02x (%d)\n",
              addr & 0xff, addr, val, val));

  return val;
}

void
SVG::out(word_t addr, byte_t val)
{
  int bit;

  DBG(2, form("KCemu/SVG/out",
              "SVG::out(): %04xh val = %02x [%c]\n",
              addr, val, isprint(val) ? val : '.'));

  switch (addr & 0xff)
    {
    case 0xa0:
      /*
       *  select sound register
       */
      DBG(2, form("KCemu/SVG/out_A0",
		  "SVG::out(): %04xh val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));
      _val_a0 = val;
      break;
    case 0xa1:
      /*
       *  sound register parameter
       */
      DBG(2, form("KCemu/SVG/out_A1",
		  "SVG::out(): %04xh val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));

      _val_a1 = val;
      _sound_reg[_val_a0] = val;

      _sound_val[A] = _sound_reg[0] | ((_sound_reg[1] & 0x0f) << 8);
      _sound_val[B] = _sound_reg[2] | ((_sound_reg[3] & 0x0f) << 8);
      _sound_val[C] = _sound_reg[4] | ((_sound_reg[5] & 0x0f) << 8);

      _sound_freq[A] = _sound_val[A] == 0 ? 0 : 3750000 / (_sound_val[A] * 32);
      _sound_freq[B] = _sound_val[B] == 0 ? 0 : 3750000 / (_sound_val[B] * 32);
      _sound_freq[C] = _sound_val[C] == 0 ? 0 : 3750000 / (_sound_val[C] * 32);

      DBG(2, form("KCemu/SVG/SOUND",
		  "SVG::out(): SOUND reg %2d = %02xh %3d /"
		  " %02x %02x-%02x %02x-%02x %02x-%02x"
		  "/%02x/%02x %02x %02x-%02x %02x-%02x\n",
		  _val_a0, val, val,
		  _sound_reg[ 0], _sound_reg[ 1], _sound_reg[ 2], _sound_reg[ 3],
		  _sound_reg[ 4], _sound_reg[ 5], _sound_reg[ 6], _sound_reg[ 7],
		  _sound_reg[ 8], _sound_reg[ 9], _sound_reg[10], _sound_reg[11],
		  _sound_reg[12], _sound_reg[13]));

      DBG(2, form("KCemu/SVG/SOUNDREGS",
		  "SVG: SOUND: ------------------------------------\n"
		  "SVG: SOUND: Channel Select           = %c%c%c\n"
		  "SVG: SOUND: Noise Channel Select     = %c%c%c\n"
		  "SVG: SOUND: Envelope Select          = %c%c%c\n"
		  "SVG: SOUND: Frequency / Loudness (A) = %d / %d -> %dHz\n"
		  "SVG: SOUND: Frequency / Loudness (B) = %d / %d -> %dHz\n"
		  "SVG: SOUND: Frequency / Loudness (C) = %d / %d -> %dHz\n"
		  "SVG: SOUND: Noise Frequency          = %d\n"
		  "SVG: SOUND: Envelope Frequency       = %d\n"
		  "SVG: SOUND: Envelope type            = %d\n"
		  "SVG: SOUND: ------------------------------------\n",
		  (_sound_reg[7] &  1) ? 'A' : 'a',
		  (_sound_reg[7] &  2) ? 'B' : 'b',
		  (_sound_reg[7] &  4) ? 'C' : 'c',
		  (_sound_reg[7] &  8) ? 'A' : 'a',
		  (_sound_reg[7] & 16) ? 'B' : 'b',
		  (_sound_reg[7] & 32) ? 'C' : 'c',
		  (_sound_reg[8] & 16) ? 'A' : 'a',
		  (_sound_reg[9] & 16) ? 'B' : 'b',
		  (_sound_reg[10] & 16) ? 'C' : 'c',
		  _sound_val[A], _sound_reg[8] & 0x0f, _sound_freq[0],
		  _sound_val[B], _sound_reg[9] & 0x0f, _sound_freq[1],
		  _sound_val[C], _sound_reg[10] & 0x0f, _sound_freq[2],
		  _sound_reg[6] & 0x1f,
		  _sound_reg[11] | (_sound_reg[12] << 8),
		  _sound_reg[13] & 0x1f));


      break;
      /*
       *  change memory configuration:
       *
       *  +---------+---------+---------+---------+
       *  |         |         |         |         |
       *  | Page  3 | Page  2 | Page  1 | Page  0 |
       *  |         |         |         |         |
       *  +----+----+----+----+----+----+----+----+
       *       |         |         |         |
       *       |         |         |         |
       *       +---------+---------+---------+-------  00 - Slot 0
       *                                               01 - Slot 1
       *                                               02 - Slot 2
       *                                               03 - Slot 3
       *
       */
    case 0xa8:
      DBG(2, form("KCemu/SVG/out_A8",
		  "SVG::out(): %04xh val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));

      if ((_val_a8 & 0x03) != (val & 0x03))
	set_page(0, val & 0x03);
      if ((_val_a8 & 0x0c) != (val & 0x0c))
	set_page(1, (val & 0x0c) >> 2);
      if ((_val_a8 & 0x30) != (val & 0x30))
	set_page(2, (val & 0x30) >> 4);
      if ((_val_a8 & 0xc0) != (val & 0xc0))
	set_page(3, (val & 0xc0) >> 6);
      _val_a8 = val;
      break;
    case 0xaa:
      DBG(2, form("KCemu/SVG/out_AA",
		  "SVG::out(): %04xh val = %02x [%c]\n",
		  addr, val, isprint(val) ? val : '.'));

      /*
       *  bit 0: output keyboard
       *  bit 1: output keyboard
       *  bit 2: output keyboard
       *  bit 3: output keyboard
       *  bit 4: tape motor (0 = on / 1 = off)
       *  bit 5: tape data
       *  bit 6: green led
       *  bit 7: key click (0 = off / 1 = on)
       *
       */
      ((Keyboard9 *)keyboard)->set_val(val & 15);
      _val_aa = val;
      break;
    case 0xab:
      DBG(2, form("KCemu/SVG/out_AB",
		  "SVG::out(): %04xh val = %02x %d%d%d%d%d%d%d%d\n",
		  addr, val,
		  (val & 128) >> 7,
		  (val &  64) >> 6,
		  (val &  32) >> 5,
		  (val &  16) >> 4,
		  (val &   8) >> 3,
		  (val &   4) >> 2,
		  (val &   2) >> 1,
		  (val &   1)));

      bit = val & 1;
      switch (val & 0x0e)
	{
	case 0x08:
	  DBG(2, form("KCemu/SVG/TAPEMOTOR",
		      "SVG::out(): TAPE MOTOR: %s\n",
		      bit ? "off" : "on"));
	  tape->power(!bit);
	  break;
	case 0x0a:
	  DBG(2, form("KCemu/SVG/TAPEDATA",
		      "SVG::out(): TAPE DATA: %d\n",
		      bit));
	  tape->tape_signal();
	  break;
	case 0x0c:
	  DBG(2, form("KCemu/SVG/CAPSLOCKLED",
		      "SVG::out(): CAPS LOCK LED: %s\n",
		      bit ? "off" : "on"));
	  break;
	case 0x0e:
	  DBG(2, form("KCemu/SVG/KEYCLICK",
		      "SVG::out(): KEYCLICK: %s\n",
		      bit ? "on" : "off"));
	  break;
	}
      _val_ab = val;
      break;
    }
}

void
SVG::set_page(int page, int slot)
{
  _page_info[page] = slot;
#if 0
  cout << "setting page " << page << " to slot " << slot
       << " => [" << _page_info[0]
       << "/" << _page_info[1]
       << "/" << _page_info[2]
       << "/" << _page_info[3]
       << "]" << endl;
#endif
  ((Memory9 *)memory)->set_page(page, slot);
}
