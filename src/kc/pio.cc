/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: pio.cc,v 1.11 2001/01/21 23:04:26 tp Exp $
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

#include <iostream.h>
#include <iomanip.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/tape.h"
#include "kc/memory.h"

#include "libdbg/dbg.h"

// #define PIO_OUT_CTRL_DEBUG

PIO::PIO(void)
{
  _cb_a_in  = 0;
  _cb_a_out = 0;
  _cb_b_in  = 0;
  _cb_b_out = 0;
  reset(true);
  z80->register_ic(this);
}

PIO::~PIO(void)
{
  z80->unregister_ic(this);
}

void
PIO::iei(byte_t val)
{
  _irq_enable[A] = val;
  _irq_enable[B] = val;
  ieo(val);
}

void
PIO::reset(bool power_on)
{
  _irq[A]        = _irq[B]        = 0;
  _value[A]      = _value[B]      = 0;
  _irq_vector[A] = _irq_vector[B] = 0;
  _ready[A]      = _ready[B]      = 1;
  _strobe[A]     = _strobe[B]     = 0;

  _irq_enable[A] = _irq_enable[B] = 0;
  _irq_active[A] = _irq_active[B] = 0;

  _ext[A] = 0xff;
  _ext[B] = 0xff;
  _ext_fn[A] = 0;
  _ext_fn[B] = 0;

  _bit_mode[A] = 0;
  _bit_mode[B] = 0;
  _bit_mode_follows[A] = false;
  _bit_mode_follows[B] = false;

  _irq_mask[A] = 0xff;
  _irq_mask[B] = 0xff;
  _irq_h_l[A] = 0;
  _irq_h_l[B] = 0;
  _irq_and_or[A] = 0;
  _irq_and_or[B] = 0;
  _irq_mask_follows[A] = false;
  _irq_mask_follows[B] = false;
}

byte_t
PIO::in_A_DATA(void) {
  byte_t ret;
#if 0
  cout << "PIO A: in DATA: 0x" << hex << setfill('0') << setw(2)
       << (int)_value[A] << endl;
#endif

  if (_cb_a_in) _cb_a_in->callback_A_in();
  if (_mode[A] == 3)
    {
      ret = (_value[A] & ~_bit_mode[A]) | (_ext[A] & _bit_mode[A]);
      // cout.form("PIO: [A] in while in mode 3 -> 0x%02x\n", ret);
      return ret;
    }
  return _value[A];
}

byte_t
PIO::in_B_DATA(void) {
  byte_t ret;
#if 0
  cout << "PIO B: in DATA: 0x" << hex << setfill('0') << setw(2)
       << (int)_value[B] << "\n";
#endif

  if (_cb_b_in) _cb_b_in->callback_B_in();
  if (_mode[B] == 3)
    {
      ret = (_value[B] & ~_bit_mode[B]) | (_ext[B] & _bit_mode[B]);
      // cout.form("PIO: [B] in while in mode 3 -> 0x%02x\n", ret);
      return ret;
    }
  return _value[B];
}

byte_t
PIO::in_A_CTRL(void) { return 0xff; } /* fixme: */
byte_t
PIO::in_B_CTRL(void) { return 0xff; } /* fixme: */

void
PIO::out_A_DATA(byte_t val)
{
  change_A(_value[A] ^ val, val);
  _value[A] = val;
  _ready[A] = 1;
  if (_cb_a_out)
    _cb_a_out->callback_A_out(val);
}

void
PIO::out_B_DATA(byte_t val)
{
  change_B(_value[B] ^ val, val);
  _value[B]   = val;
  _irq[B]     = 1;
  _ready[B]   = 1;
  if (_cb_b_out)
    _cb_b_out->callback_B_out(val);
}

void
PIO::out_CTRL(int port, byte_t val)
{
  char p = (port == A) ? 'A' : 'B';

  /*
   *  bit mode
   *
   *  +-----+-----+-----+-----+-----+-----+-----+-----+
   *  | IO7 | IO6 | IO5 | IO4 | IO3 | IO2 | IO1 | IO0 |
   *  +-----+-----+-----+-----+-----+-----+-----+-----+
   */
  if (_bit_mode_follows[port])
    {
      _bit_mode[port] = val;
      _bit_mode_follows[port] = false;
#ifdef PIO_OUT_CTRL_DEBUG
      cout.form("PIO: [%c] new bit mode: %02x (0 = out/ 1 = in)\n", p, _bit_mode[port]);
#endif
      return;
    }

  /*
   *  interrupt mask (for bit input/output)
   *
   *  +-----+-----+-----+-----+-----+-----+-----+-----+
   *  | MB7 | MB6 | MB5 | MB4 | MB3 | MB2 | MB1 | MB0 |
   *  +-----+-----+-----+-----+-----+-----+-----+-----+
   */
  if (_irq_mask_follows[port])
    {
      /*
       *  we store the mask negated because a bit set to 0 means
       *  this bit is used!
       */
      _irq_mask[port] = ~val;
      _irq_mask_follows[port] = false;
#ifdef PIO_OUT_CTRL_DEBUG
      cout.form("PIO: [%c] new irq mask (inverted): %02x\n", p, _irq_mask[port]);
#endif
      return;
    }
  
  /*
   *  interrupt vector
   *
   *  +----+----+----+----+----+----+----+---+
   *  | V7 | V6 | V5 | V4 | V3 | V2 | V1 | 0 |
   *  +----+----+----+----+----+----+----+---+
   */
  if ((val & 1) == 0)
    {
      _irq_vector[port] = val;
#ifdef PIO_OUT_CTRL_DEBUG
      cout.form("PIO: [%c] new irq vector: 0x%02x\n", p, val);
#endif
      return;
    }
  
  switch (val & 0x0f)
    {
      /*
       *  interrupt enable/disable
       *
       *  +----+---+---+---+---+---+---+---+
       *  | EI | X | X | X | 0 | 0 | 1 | 1 |
       *  +----+---+---+---+---+---+---+---+
       */
    case 0x03:
      _irq[port] = (val >> 7) & 0x01;
      if (_irq[port])
        {
#ifdef PIO_OUT_CTRL_DEBUG
          cout.form("PIO: [%c] irq enabled\n", p);
#endif
          _irq_enable[port] = 1;
        }
      else
        {
#ifdef PIO_OUT_CTRL_DEBUG
          cout.form("PIO: [%c] irq disabled\n", p);
#endif
          _irq_enable[port] = 0;
        }
      break;
      
      /*
       *  interrupt control word
       *
       *  +----+-----+-----+----+---+---+---+---+
       *  | EI | A/O | H/L | MF | 0 | 1 | 1 | 1 |
       *  +----+-----+-----+----+---+---+---+---+
       */
    case 0x07:
      _irq[port] = (val >> 7) & 1;
      if (_irq[port])
        {
#ifdef PIO_OUT_CTRL_DEBUG
          cout.form("PIO: [%c] irq enabled\n", p);
#endif
          _irq_enable[port] = 1;
        }
      else
        {
#ifdef PIO_OUT_CTRL_DEBUG
          cout.form("PIO: [%c] irq disabled\n", p);
#endif
          _irq_enable[port] = 0;
        }
      
      _irq_and_or[port] = (val >> 6) & 1;
#ifdef PIO_OUT_CTRL_DEBUG
      if (_irq_and_or[port])
        cout.form("PIO: [%c] AND/OR mode set to AND\n", p);
      else
        cout.form("PIO: [%c] AND/OR mode set to OR\n", p);
#endif
      
      _irq_h_l[port] = (val >> 5) & 1;
#ifdef PIO_OUT_CTRL_DEBUG
      if (_irq_h_l[port])
        cout.form("PIO: [%c] H/L mode set to H\n", p);
      else
        cout.form("PIO: [%c] H/L mode set to L\n", p);
#endif
      
      if (val & 0x10)
        _irq_mask_follows[port] = true;
      
      break;
      
      /*
       *  mode control word
       *
       *  +----+----+---+---+---+---+---+---+
       *  | M1 | M0 | X | X | 1 | 1 | 1 | 1 |
       *  +----+----+---+---+---+---+---+---+
       */
    case 0x0f:
      _mode[port] = (val >> 6) & 0x03;
#ifdef PIO_OUT_CTRL_DEBUG
      cout.form("PIO: [%c] new mode: %d - ", p, _mode[port]);
#endif
      switch (_mode[port])
        {
        case 0:
#ifdef PIO_OUT_CTRL_DEBUG
          cout << "byte output" << endl;
#endif
          break;
        case 1:
#ifdef PIO_OUT_CTRL_DEBUG
          cout << "byte input" << endl;
#endif
          break;
        case 2:
#ifdef PIO_OUT_CTRL_DEBUG
          cout << "byte input/output" << endl;
#endif
          break;
        case 3:
#ifdef PIO_OUT_CTRL_DEBUG
          cout << "bit mode" << endl;
#endif
          _bit_mode_follows[port] = true;
          break;
        }
      break;
    default:
      cout.form("PIO: [%c] ??? unknon control byte: %02x\n", p, val);
      break;
    }
}

void
PIO::out_A_CTRL(byte_t val)
{
  out_CTRL(A, val);
}

void
PIO::out_B_CTRL(byte_t val)
{
  out_CTRL(B, val);
}

void
PIO::set_EXT(int port, byte_t mask, byte_t val)
{
  byte_t old;
  char p = "AB"[port];

  // cout.form("PIO: [%c] external value: mask = 0x%02x, val = 0x%02x (old = 0x%02x)\n", p, mask, val, _ext[port]);

  if (_irq_active[port] )
    {
      cout.form("PIO::set_EXT() - irq active\n");
      return;
    }

  old = _ext_fn[port];
  _ext[port] = ((_ext[port] & ~mask) | (val & mask));
  
  if (_irq_and_or[port])
    {
      /* AND */
      if (_irq_h_l[port])
        _ext_fn[port] = ((_ext[port] & _irq_mask[port]) == _irq_mask[port]);
      else
        _ext_fn[port] = ((_ext[port] & _irq_mask[port]) == 0);
    }
  else
    {
      /* OR */
      if (_irq_h_l[port])
        _ext_fn[port] = ((_ext[port] & _irq_mask[port]) != 0);
      else
        _ext_fn[port] = ((_ext[port] & _irq_mask[port]) != _irq_mask[port]);
    }

#if 0  
  cout.form("PIO: [%c] _ext_fn: A/O = %d, H/L = %d, mask = 0x%02x\n",
            p, _irq_and_or[port], _irq_h_l[port], _irq_mask[port]);
  cout.form("PIO: [%c] _ext_fn: old = %02x, _ext_fn = %02x\n",
            p, old, _ext_fn[port]);
#endif
  
  if (_irq_enable[port])
    {
      if ((old == 0) && (_ext_fn[port] == 1))
        {
          _strobe[port] = 1;
          if (z80->triggerIrq(_irq_vector[port]))
            {
              // cout.form("PIO::set_EXT() - %c irq ack\n", p);
              _strobe[port] = 0;
              _irq_active[port] = 1;
              z80->handleIrq(_irq_vector[port]);
            }
          else
            {
              // cout.form("PIO::set_EXT() - %c irq nack\n", p);
            }
        }
      else
        {
          // cout.form("PIO::set_EXT() - %c IEI is low\n", p);
        }
    }
}

void
PIO::set_A_EXT(byte_t mask, byte_t val)
{
  set_EXT(A, mask, val);
}

void
PIO::set_B_EXT(byte_t mask, byte_t val)
{
  set_EXT(B, mask, val);
}

void
PIO::reti(void)
{
  //cout.form("PIO: reti - A: %d, B: %d\n", _irq_active[a], _irq_active[B]);
  if (_irq_active[A])
    {
      _irq_active[A] = 0;
    }
  else
    {
      if (_strobe[A])
        {
          if (z80->triggerIrq(_irq_vector[A]))
            {
              //cout.form("PIO::reti() A - %04xh\n", z80->getPC());
              //cout.form("PIO::reti() A - irq ack\n");
              _irq_active[A] = 1;
              z80->handleIrq(_irq_vector[A]);
            }
          else
            {
              //cout.form("PIO::reti() A - irq nack\n");
            }
          _strobe[A] = 0;
          return;
        }
    }
  
  if (_irq_active[B])
    {
      _irq_active[B] = 0;
    }
  else
    {
      if (_strobe[B])
        {
          if (z80->triggerIrq(_irq_vector[B]))
            {
              //cout.form("PIO::reti() B - %04xh\n", z80->getPC());
              //cout.form("PIO::reti() B - irq ack\n");
              _irq_active[B] = 1;
              z80->handleIrq(_irq_vector[B]);
            }
          else
            {
              //cout.form("PIO::reti() B - irq nack\n");
            }
          _strobe[B] = 0;
        }
    }
}

void
PIO::strobe_A(void)
{
  long long cnt;
  static long long ocnt = 0;

  cnt = z80->getCounter();
  DBG(2, form("KCemu/PIO/strobe/A/timing",
              "PIO::strobe_A() : %10Ld - %Ld \n",
              cnt, cnt - ocnt));
  ocnt = cnt;

  if (_irq_active[A])
    {
      DBG(2, form("KCemu/PIO/strobe/A/active",
		  "PIO::strobe_A() : irq active!\n"));
      return;
    }
  if (_irq_enable[A])
    {
      _strobe[A] = 1;
      if (z80->triggerIrq(_irq_vector[A]))
        {
          //cout.form("PIO::strobe_A() - irq ack\n");
          _strobe[A] = 0;
          _irq_active[A] = 1;
          z80->handleIrq(_irq_vector[A]);
        }
      else
        {
	  DBG(2, form("KCemu/PIO/strobe/A/nack",
		      "PIO::strobe_A() : irq not acknowledged!\n"));
        }
    }
  else
    {
      DBG(2, form("KCemu/PIO/strobe/A/iei",
		  "PIO::strobe_A() : IEI is low!\n"));
    }
}

void
PIO::strobe_B(void)
{
  long long cnt;
  static long long ocnt = 0;

  cnt = z80->getCounter();
  DBG(2, form("KCemu/PIO/strobe/B/timing",
              "PIO::strobe_B() : %10Ld - %Ld \n",
              cnt, cnt - ocnt));
  ocnt = cnt;

  if (_irq_active[B])
    {
      DBG(2, form("KCemu/PIO/strobe/B/active",
		  "PIO::strobe_B() : irq active!\n"));
      return;
    }
  if (_irq_enable[B])
    {
      _strobe[B] = 1;
      if (z80->triggerIrq(_irq_vector[B]))
        {
          //cout.form("PIO::strobe_B() - irq ack\n");
          _strobe[B] = 0;
          _irq_active[B] = 1;
          z80->handleIrq(_irq_vector[B]);
        }
      else
        {
	  DBG(2, form("KCemu/PIO/strobe/B/nack",
		      "PIO::strobe_B() : irq not acknowledged!\n"));
        }
    }
  else
    {
      DBG(2, form("KCemu/PIO/strobe/B/iei",
		  "PIO::strobe_B() : IEI is low!\n"));
    }
}

int
PIO::ready_A(void)
{
    return _ready[A];
}

int
PIO::ready_B(void)
{
    return _ready[B];
}

void
PIO::register_callback_A_in(PIOCallbackInterface *cbi)
{
  _cb_a_in = cbi;
}

void
PIO::register_callback_A_out(PIOCallbackInterface *cbi)
{
  _cb_a_out = cbi;
}

void
PIO::register_callback_B_in(PIOCallbackInterface *cbi)
{
  _cb_b_in = cbi;
}

void
PIO::register_callback_B_out(PIOCallbackInterface *cbi)
{
  _cb_b_out = cbi;
}

void
PIO::info(void)
{
    cout << "  PIO:" << endl;
    cout << "  ----                   port A        port B" << endl;
    cout << "\tmode:            "     << (int)_mode[A]
	 << "             " << (int)_mode[B] << endl;
    cout << "\tirq vector:      " << hex << setfill('0') << setw(2)
	 << (int)_irq_vector[A]
	 << "h           " << hex << setfill('0') << setw(2)
         << (int)_irq_vector[B] << "h" << endl;
    cout << "\tirq enabled:     " << ((_irq[A])?"yes":"no ")
	 << "           " << ((_irq[B])?"yes":"no ") << endl;
    cout << "\tstrobe:          " << (int)_strobe[A]
	 << "             " << (int)_strobe[B] << endl;
    cout << "\tready:           " << (int)_ready[A]
	 << "             " << (int)_ready[B] << endl;
    cout << "\tvalue:           " << hex << setfill('0') << setw(2)
	 << (int)_value[A]
         << "h           " << hex << setfill('0') << setw(2)
	 << (int)_value[B] << "h" << endl << endl;
}
