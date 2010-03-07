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
#include <stdio.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/gdc.h"

#include "libdbg/dbg.h"

using namespace std;

GDC::GDC(void) : InterfaceCircuit("VIS")
{
  reset(true);
  z80->register_ic(this);
}

GDC::~GDC(void)
{
  z80->unregister_ic(this);
}

byte_t
GDC::in(word_t addr)
{
  byte_t val = 0xff;

  switch (addr & 0x01)
    {
    case 0x00:
      /*
       *  status register
       *
       *  bit 0 (0x01): 0 not ready, 1 ready ? [17a1h]
       *  bit 1 (0x02): 0 ready, 1 not ready ? [1fa7h]
       *  bit 2 (0x04): 0 not ready, 1 ready ? [1797h]
       *  bit 5 (0x20): 1 while vertical retrace
       */
      val = _control;
      break;
    case 0x01:
      if ((_cmd & 0xe4) == 0xa0)
	{
	  if ((_ridx & 1) == 0)
	    val = _mem[_ptr];
	  else
	    {
	      val = _col[_ptr];
	      _ptr = (_ptr + 1) & 0xffff;
	    }
	  //printf("RDAT: %4d, ptr = %04x, val = %02x\n", _ridx, _ptr, val);
	}
      else if (_cmd == 0xe0)
	{
	  switch (_ridx)
	    {
	    case 0: val = _ptr & 0xff; break;
	    case 1: val = (_ptr >> 8) & 0xff; break;
	    default:
	      val = 0;
	      break;
	    }
	  //printf("CURD: %4d, ptr = %04x, val = %02x\n", _ridx, _ptr, val);
	}
      else
	val = _mem[_ptr];

      _ridx++;
      DBG(2, form("KCemu/GDC/in_data",
		  "GDC::in():  %04xh cmd = %02x, val = %02x\n",
		  addr, _cmd, val));
    default:
      break;
    }
      
  DBG(2, form("KCemu/GDC/in",
              "GDC::in():  %04xh val = %02x\n",
              addr, val));

  return val;
}

void
GDC::out(word_t addr, byte_t val)
{
  switch (addr & 0x01)
    {
    case 0x00:
      _arg[_idx++] = val;
      break;
    case 0x01:
      info();
      _idx = 0;
      _ridx = 0;
      _cmd = val;
      break;
    }

  DBG(2, form("KCemu/GDC/out",
              "GDC::out(): %04xh: %04xh val = %02x [%c]\n",
              z80->getPC(), addr, val, isprint(val) ? val : '.'));

  if (_cmd == 0x00) // RESET
    _screen_on = 0;

  if ((_cmd & 0xfe) == 0x0e) // SYNC
    _screen_on = _cmd & 1;

  if ((_cmd & 0xfe) == 0x6e) // VSYNC
    ;

  if ((_cmd == 0x4b) && (_idx == 3)) // CCHAR
    {
      _nr_of_lines = _arg[0] & 0x1f;
      _cursor_top = _arg[1] & 0x1f;
      _cursor_bottom = (_arg[2] & 0xf8) >> 3;
    }

  if (_cmd == 0x6b) // START
    ;

  if ((_cmd & 0xfe) == 0x0c) // BCTRL
    _screen_on = _cmd & 1;

  if (_cmd == 0x46) // ZOOM
    ;

  if (_cmd == 0x49) // CURS
    {
      if (_idx == 2)
	{
	  _ptr = ((int)_arg[1] * 256 + _arg[0]);
	  _pptr = 0;
	}
      else if (_idx == 3)
	{
	  _ptr = (_ptr & 0xffff) | ((_arg[2] & 0x03) << 16);
	  _pptr = (_arg[2] >> 4) & 0x0f;
	  _mask = 1 << _pptr;
	  _mask_c = _mask >> 8;
	}
    }

  if (((_cmd & 0xf0) == 0x70) && (_idx > 0)) // PRAM
    {
      int idx = (_cmd & 0x0f) + _idx - 1;
      if (idx < 16)
	_pram[idx] = val;
    }

  if (_cmd == 0x47) // PITCH
    ;

  if (((_cmd & 0xe4) == 0x20) && (_idx > 0)) // WDAT
    {
      switch (_cmd & 0x18)
	{
	case 0x00: /* type 00 - two byte transfer */
	  if (_idx & 1)
	    _mem[_ptr] = (_mask & val) | (~_mask & _mem[_ptr]);
	  else
	    {
	      _col[_ptr] = (_mask_c & val) | (~_mask_c & _col[_ptr]);
	      _ptr = (_ptr + 1) & 0xffff;
	      while ((_idx == 2) && _figs_dc > 0)
		{
		  _mem[_ptr] = (_mask & _arg[0]) | (~_mask & _mem[_ptr]);
		  _col[_ptr] = (_mask_c & _arg[1]) | (~_mask_c & _col[_ptr]);
		  _ptr = (_ptr + 1) & 0xffff;
		  _figs_dc--;
		}
	    }
	  break;
	case 0x08: /* type 01 - invalid */
	  break;
	case 0x10: /* type 10 - low byte of word */
	  _mem[_ptr] = (_mask & val) | (~_mask & _mem[_ptr]);
	  _ptr = (_ptr + 1) & 0xffff;
	  break;
	case 0x18: /* type 11 - high byte of word */
	  _col[_ptr] = (_mask_c & val) | (~_mask_c & _col[_ptr]);
	  _ptr = (_ptr + 1) & 0xffff;
	  break;
	}
    }
      
  if ((_cmd == 0x4a) && (_idx > 0)) // MASK
    {
      if (_idx == 1)
	{
	  /*
	   *  the z1013 gdc driver only loads the lower byte
	   *  into the mask register :-(
	   */
	  _mask = _arg[0];
	}
      else
	{
	  _mask = _arg[0] | (_arg[1] << 8);
	  _mask_c = _arg[1];
	}
    }

  if ((_cmd == 0x4c) && (_idx > 2)) // FIGS
    _figs_dc = _arg[1] | ((_arg[2] & 0x3f) << 8);
  
  if (_cmd == 0x6c) // FIGD
    ;

  if (_cmd == 0x68) // GCHRD
    ;

  if ((_cmd & 0xe4) == 0xa0) // RDAT
    _figs_dc = 0;

  if (_cmd == 0xe0) // CURD
    ;

  if (_cmd == 0xc0) // LPRD
    ;

  if ((_cmd & 0xe4) == 0xa4) // DMAR
    ;

  if ((_cmd & 0xe4) == 0x24) // DMAW
    ;
}

byte_t
GDC::get_mem(int addr)
{
  return _mem[addr & 0xffff];
}

byte_t
GDC::get_col(int addr)
{
  return _col[addr & 0xffff];
}

bool
GDC::get_cursor(int addr)
{
  if (addr != _ptr)
    return false;

  return true;
}

bool
GDC::get_cursor(int addr, int line)
{
  if (addr != _ptr)
    return false;

  if (line < _cursor_top)
    return false;

  if (line > _cursor_bottom)
    return false;

  return true;
}

int
GDC::get_nr_of_lines(void)
{
  return _nr_of_lines;
}

int
GDC::get_screen_on(void)
{
  return _screen_on;
}

long
GDC::get_pram_SAD(int idx)
{
  if ((idx < 0) || (idx > 3))
    return 0;

  return _pram[4 * idx] | (_pram[4 * idx + 1] << 8) | ((_pram[4 * idx + 2] & 3) << 16);
}

long
GDC::get_pram_LEN(int idx)
{
  if ((idx < 0) || (idx > 3))
    return 0;

  return ((_pram[4 * idx + 2] & 0xf0) >> 4) | ((_pram[4 * idx + 3] & 0x3f) << 4);
}

void
GDC::v_retrace(bool value)
{
  if (value)
    _control |= 0x20;
  else
    _control &= ~0x20;
}

void
GDC::info(void)
{
  switch (_cmd)
    {
    case 0x00:
      DBG(2, form("KCemu/GDC/RESET",
		  "GDC: RESET ------------------------------- %02x\n"
		  "GDC: RESET mode                  %s\n"
		  "GDC: RESET -------------------------------\n",
		  _cmd,
		  ((_arg[0] & 0x20) ?
		   ((_arg[0] & 0x02) ? "  invalid" : "character") :
		   ((_arg[0] & 0x02) ? "  graphic": "    mixed"))
                  ));
      break;
      
    case 0x0e: case 0x0f:
      DBG(2, form("KCemu/GDC/SYNC",
		  "GDC: SYNC -------------------------------- %02x\n"
		  "GDC: SYNC display on/off               %s\n"
		  "GDC: SYNC mode (C/G)       %s (%d/%d)\n"
		  "GDC: SYNC retrace (I/S)      %s (%d/%d)\n"
		  "GDC: SYNC refresh (D)                  %s\n"
		  "GDC: SYNC drawing (F)         %s\n"
		  "GDC: SYNC --------------------------------\n",
		  _cmd,
		  (_cmd & 1) ? " on" : "off",
		  
		  ((_arg[0] & 0x20) ?
		   ((_arg[0] & 0x02) ? "  invalid" : "character") :
		   ((_arg[0] & 0x02) ? "  graphic" : "    mixed")),
                  (_arg[0] & 0x20) >> 5, // C
		  (_arg[0] & 0x02) >> 1, // G
		  
		  ((_arg[0] & 0x08) ?
		   ((_arg[0] & 0x01) ? "zeilensprung" : "doppelbild") :
		   ((_arg[0] & 0x01) ? "invalid" : "kein zeilensprung")),
		  (_arg[0] & 0x08) >> 3, // I
		  (_arg[0] & 0x01),      // S

		  ((_arg[0] & 0x04) ? "yes" : " no"),

		  ((_arg[0] & 0x10) ? "retrace only" : "      always")
		  ));
      break;

    case 0x6e: case 0x6f:
      DBG(2, form("KCemu/GDC/VSYNC",
		  "GDC: VSYNC ------------------------------- %02x\n"
		  "GDC: VSYNC external sync            %s\n"
		  "GDC: VSYNC -------------------------------\n",
		  _cmd,
		  (_cmd & 1) ? "master" : " slave"));
      break;

    case 0x4b:
      DBG(2, form("KCemu/GDC/CCHAR",
		  "GDC: CCHAR ------------------------------- %02x\n"
		  "GDC: CCHAR display cursor              %s\n"
		  "GDC: CCHAR number of lines           %5d\n"
		  "GDC: CCHAR cursor top                %5d\n"
		  "GDC: CCHAR cursor bottom             %5d\n"
		  "GDC: CCHAR steady cursor               %s\n"
		  "GDC: CCHAR cursor blink frequency    %5d\n"
		  "GDC: CCHAR -------------------------------\n",
		  _cmd,
		  (_arg[0] & 0x80) ? " on" : "off",
		  _nr_of_lines,
		  _cursor_top,
		  _cursor_bottom,
		  (_arg[1] & 0x20) ? " on" : "off",
		  ((_arg[1] & 0xc0) >> 6) | ((_arg[2] & 0x07) << 2)));
      break;

    case 0x6b:
      DBG(2, form("KCemu/GDC/START",
		  "GDC: START ------------------------------- %02x\n"
		  "GDC: START -------------------------------\n",
		  _cmd));
      break;

    case 0x0c: case 0x0d:
      DBG(2, form("KCemu/GDC/BCTRL",
		  "GDC: BCTRL ------------------------------- %02x\n"
		  "GDC: BCTRL display on/off              %s\n"
		  "GDC: BCTRL -------------------------------\n",
		  _cmd,
		  (_cmd & 1) ? " on" : "off"));
      break;

    case 0x46:
      DBG(2, form("KCemu/GDC/ZOOM",
		  "GDC: ZOOM -------------------------------- %02x\n"
		  "GDC: ZOOM --------------------------------\n",
		  _cmd));
      break;

    case 0x49:
      DBG(2, form("KCemu/GDC/CURS",
		  "GDC: CURS -------------------------------- %02x\n"
		  "GDC: CURS set cursor to addr:        %05x\n"
		  "GDC: CURS point address:                %02x\n"
		  "GDC: CURS --------------------------------\n",
		  _cmd,
		  _ptr,
		  _pptr));
      break;

    case 0x70: case 0x71: case 0x72: case 0x73:
    case 0x74: case 0x75: case 0x76: case 0x77:
    case 0x78: case 0x79: case 0x7a: case 0x7b:
    case 0x7c: case 0x7d: case 0x7e: case 0x7f:
      DBG(2, form("KCemu/GDC/PRAM",
		  "GDC: PRAM -------------------------------- %02x\n"
		  "GDC: PRAM write starting at register    %2d\n"
		  "GDC: PRAM register  0: %04x %04x %04x %04x\n"
		  "GDC: PRAM register  4: %04x %04x %04x %04x\n"
		  "GDC: PRAM register  8: %04x %04x %04x %04x\n"
		  "GDC: PRAM register 12: %04x %04x %04x %04x\n"
		  "GDC: PRAM SAD/LEN 1  %010x/%010x\n"
		  "GDC: PRAM IM/WD 1              %s/%d\n"
		  "GDC: PRAM SAD/LEN 2  %010x/%010x\n"
		  "GDC: PRAM IM/WD 2              %s/%d\n"
		  "GDC: PRAM SAD/LEN 3  %010x/%010x\n"
		  "GDC: PRAM IM/WD 3              %s/%d\n"
		  "GDC: PRAM SAD/LEN 4  %010x/%010x\n"
		  "GDC: PRAM IM/WD 4              %s/%d\n"
		  "GDC: PRAM --------------------------------\n",
		  _cmd,
		  
		  _cmd & 15,
		  _pram[ 0], _pram[ 1], _pram[ 2], _pram[ 3],
		  _pram[ 4], _pram[ 5], _pram[ 6], _pram[ 7],
		  _pram[ 8], _pram[ 9], _pram[10], _pram[11],
		  _pram[12], _pram[13], _pram[14], _pram[15],
		  
		  _pram[0] | (_pram[1] << 8) | ((_pram[2] & 3) << 16),
		  ((_pram[2] & 0xf0) >> 4) | ((_pram[3] & 0x3f) << 4),
		  ((_pram[3] & 0x40) >> 6) ? "  graphic" : "character",
		  (_pram[3] & 0x80) >> 7,
		  
		  _pram[4] | (_pram[5] << 8) | ((_pram[6] & 3) << 16),
		  ((_pram[6] & 0xf0) >> 4) | ((_pram[7] & 0x3f) << 4),
		  ((_pram[7] & 0x40) >> 6) ? "  graphic" : "character",
		  (_pram[7] & 0x80) >> 7,
		  
		  _pram[8] | (_pram[9] << 8) | ((_pram[10] & 3) << 16),
		  ((_pram[10] & 0xf0) >> 4) | ((_pram[11] & 0x3f) << 4),
		  ((_pram[11] & 0x40) >> 6) ? "  graphic" : "character",
		  (_pram[11] & 0x80) >> 7,
		  
		  _pram[12] | (_pram[13] << 8) | ((_pram[14] & 3) << 16),
		  ((_pram[14] & 0xf0) >> 4) | ((_pram[15] & 0x3f) << 4),
                  ((_pram[15] & 0x40) >> 6) ? "  graphic" : "character",
		  (_pram[15] & 0x80) >> 7));
      break;

    case 0x47:
      DBG(2, form("KCemu/GDC/PITCH",
		  "GDC: PITCH ------------------------------- %02x\n"
		  "GDC: PITCH line width                %5d\n"
		  "GDC: PITCH -------------------------------\n",
		  _cmd,
		  _arg[0]));
      break;

    case 0x20: case 0x21: case 0x22: case 0x23:
    case 0x28: case 0x29: case 0x2a: case 0x2b:
    case 0x30: case 0x31: case 0x32: case 0x33:
    case 0x38: case 0x39: case 0x3a: case 0x3b:
      DBG(2, form("KCemu/GDC/WDAT",
		  "GDC: WDAT -------------------------------- %02x\n"
		  "GDC: WDAT type                           %d\n"
		  "GDC: WDAT MOD                            %d\n"
		  "GDC: WDAT arg[0]                      0x%02x\n"
		  "GDC: WDAT arg[1]                      0x%02x\n"
		  "GDC: WDAT --------------------------------\n",
		  _cmd,
		  (_cmd & 0x18) >> 3,
		  (_cmd & 3),
		  _arg[0],
		  _arg[1]));
      break;

    case 0x4a:
      DBG(2, form("KCemu/GDC/MASK",
		  "GDC: MASK -------------------------------- %02x\n"
		  "GDC: MASK mask                        %04x\n"
		  "GDC: MASK --------------------------------\n",
		  _cmd,
		  _mask));
      break;

    case 0x4c:
      DBG(2, form("KCemu/GDC/FIGS",
		  "GDC: FIGS -------------------------------- %02x\n"
		  "GDC: FIGS count                      %05d\n"
		  "GDC: FIGS --------------------------------\n",
		  _cmd,
		  _figs_dc));
      break;

    case 0x6c:
      DBG(2, form("KCemu/GDC/FIGD",
		  "GDC: FIGD -------------------------------- %02x\n"
		  "GDC: FIGD --------------------------------\n",
		  _cmd));
      break;

    case 0x68:
      DBG(2, form("KCemu/GDC/GCHRD",
		  "GDC: GCHRD ------------------------------- %02x\n"
		  "GDC: GCHRD -------------------------------\n",
		  _cmd));
      break;

    case 0xa0: case 0xa1: case 0xa2: case 0xa3:
    case 0xa8: case 0xa9: case 0xaa: case 0xab:
    case 0xb0: case 0xb1: case 0xb2: case 0xb3:
    case 0xb8: case 0xb9: case 0xba: case 0xbb:
      DBG(2, form("KCemu/GDC/RDAT",
		  "GDC: RDAT -------------------------------- %02x\n"
		  "GDC: RDAT type                           %d\n"
		  "GDC: RDAT MOD                            %d\n"
		  "GDC: RDAT --------------------------------\n",
		  _cmd,
		  (_cmd & 0x18) >> 3,
		  (_cmd & 3)));
      break;

    case 0xe0:
      DBG(2, form("KCemu/GDC/CURD",
		  "GDC: CURD -------------------------------- %02x\n"
		  "GDC: CURD --------------------------------\n",
		  _cmd));
      break;

    case 0xc0:
      DBG(2, form("KCemu/GDC/LPRD",
		  "GDC: LPRD -------------------------------- %02x\n"
		  "GDC: LPRD --------------------------------\n",
		  _cmd));
      break;

    case 0xa4: case 0xa5: case 0xa6: case 0xa7:
    case 0xac: case 0xad: case 0xae: case 0xaf:
    case 0xb4: case 0xb5: case 0xb6: case 0xb7:
    case 0xbc: case 0xbd: case 0xbe: case 0xbf:
      DBG(2, form("KCemu/GDC/DMAR",
		  "GDC: DMAR -------------------------------- %02x\n"
		  "GDC: DMAR type                           %d\n"
		  "GDC: DMAR MOD                            %d\n"
		  "GDC: DMAR --------------------------------\n",
		  _cmd,
		  (_cmd & 0x18) >> 3,
		  (_cmd & 3)));
      break;

    case 0x24: case 0x25: case 0x26: case 0x27:
    case 0x2c: case 0x2d: case 0x2e: case 0x2f:
    case 0x34: case 0x35: case 0x36: case 0x37:
    case 0x3c: case 0x3d: case 0x3e: case 0x3f:
      DBG(2, form("KCemu/GDC/DMAW",
		  "GDC: DMAW -------------------------------- %02x\n"
		  "GDC: DMAW type                           %d\n"
		  "GDC: DMAW MOD                            %d\n"
		  "GDC: DMAW --------------------------------\n",
		  _cmd,
		  (_cmd & 0x18) >> 3,
		  (_cmd & 3)));
      break;

    default:
      cout << "CMD: 0x" << hex << (int)_cmd << " -";
      for (int a = 0;a < _idx;a++)
	cout << " 0x" << hex << (int)_arg[a];
      cout << endl;
    }
}

void
GDC::reti(void)
{
}

void
GDC::irqreq(void)
{
}

word_t
GDC::irqack(void)
{
  return IRQ_NOT_ACK;
}

void
GDC::reset(bool power_on)
{
  _idx = 0;
  _ptr = 0;
  _pptr = 0;
  _mask = 0;
  _mask_c = 0;
  _control = 5;
  _figs_dc = 0;
  _screen_on = 0;
  _nr_of_lines = 0;
  _cursor_top = 1;
  _cursor_bottom = 0;

  memset(_pram, 0, 16);
  memset(_mem, 0x20, 65536); // FIXME: handle screen blanking
  memset(_col, 0x00, 65536);
}
