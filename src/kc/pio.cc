/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: pio.cc,v 1.17 2002/10/31 01:46:35 torsten_paul Exp $
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

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/tape.h"
#include "kc/memory.h"

#include "libdbg/dbg.h"

//#define PIO_OUT_CTRL_DEBUG

using namespace std;

PIO::PIO(void) : InterfaceCircuit("PIO")
{
  _cb_a_in  = 0;
  _cb_a_out = 0;
  _cb_b_in  = 0;
  _cb_b_out = 0;
  reset(true);
  z80->register_ic(this);
  _z80_irq_mask = z80->daisy_chain_get_irqmask();
}

PIO::~PIO(void)
{
  z80->unregister_ic(this);
}

void
PIO::reset(bool power_on)
{
  _irq[A]        = _irq[B]        = 0;
  _value[A]      = _value[B]      = 0;
  _irq_vector[A] = _irq_vector[B] = 0;
  _ready[A]      = _ready[B]      = 1;
  _strobe[A]     = _strobe[B]     = 0;
  _mode[A]       = _mode[B]       = 0;

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
  int cb;
  byte_t ret;

  cb = -1;
  if (_cb_a_in)
    cb = _cb_a_in->callback_A_in();

  if (_mode[A] == 3)
    {
      ret = (_value[A] & ~_bit_mode[A]) | (_ext[A] & _bit_mode[A]);
      DBG(2, form("KCemu/PIO/A/in_DATA",
		  "PIO::in():  port A DATA (mode %d): val = %02x, mask = %02x, ext = %02x\n",
		  _mode[A], ret, _bit_mode[A], _ext[A]));
    }
  else
    {
      ret = _value[A];
      if (cb >= 0)
	ret = cb;
      DBG(2, form("KCemu/PIO/A/in_DATA",
		  "PIO::in():  port A DATA (mode %d): val = %02x\n",
		  _mode[A], ret));
    }

  return ret;
}

byte_t
PIO::in_B_DATA(void) {
  int cb;
  byte_t ret;

  cb = -1;
  if (_cb_b_in)
    cb = _cb_b_in->callback_B_in();

  if (_mode[B] == 3)
    {
      ret = (_value[B] & ~_bit_mode[B]) | (_ext[B] & _bit_mode[B]);
      DBG(2, form("KCemu/PIO/B/in_DATA",
		  "PIO::in():  port B DATA (mode %d): val = %02x, mask = %02x, ext = %02x\n",
		  _mode[B], ret, _bit_mode[B], _ext[B]));
    }
  else
    {
      ret = _value[B];
      if (cb >= 0)
	ret = cb;
      DBG(2, form("KCemu/PIO/B/in_DATA",
		  "PIO::in():  port B DATA (mode %d): val = %02x\n",
		  _mode[B], ret));
    }

  return ret;
}

byte_t
PIO::in_A_CTRL(void) {
  byte_t ret = 0xff;

  DBG(2, form("KCemu/PIO/A/in_CTRL",
              "PIO::in():  port A CTRL (mode %d): val = %02x\n",
              _mode[A], ret));

  return ret; /* FIXME: */
}

byte_t
PIO::in_B_CTRL(void) {
  byte_t ret = 0xff;

  DBG(2, form("KCemu/PIO/B/in_CTRL",
              "PIO::in():  port B CTRL (mode %d): val = %02x\n",
              _mode[B], ret));

  return ret; /* FIXME: */
}

void
PIO::out_A_DATA(byte_t val)
{
  DBG(2, form("KCemu/PIO/A/out_DATA",
              "PIO::out(): port A DATA (mode %d): val = %02x\n",
              _mode[B], val));

  change_A(_value[A] ^ val, val);
  _value[A] = val;
  _ready[A] = 1;

  if (_cb_a_out)
    _cb_a_out->callback_A_out(val);
}

void
PIO::out_B_DATA(byte_t val)
{
  DBG(2, form("KCemu/PIO/B/out_DATA",
              "PIO::out(): port B DATA (mode %d): val = %02x\n",
              _mode[B], val));

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

  if (port == A)
    {
      DBG(2, form("KCemu/PIO/A/out_CTRL",
		  "PIO::out(): port A CTRL (mode %d): val = %02x\n",
		  p, _mode[A], val));
    }
  else
    {
      DBG(2, form("KCemu/PIO/B/out_CTRL",
		  "PIO::out(): port B CTRL (mode %d): val = %02x\n",
		  p, _mode[B], val));
    }

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
      cout << "PIO: [" << p << "] ??? unknon control byte: "
	   << hex << setw(2) << setfill('0') << val
	   << endl;
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
#if 0
  char p = "AB"[port];
#endif

  if (_irq_active[port] )
    {
      cout << "PIO::set_EXT() - irq active" << endl;
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
  
  if ((old == 0) && (_ext_fn[port] == 1))
    trigger_irq(port);
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
PIO::trigger_irq(int port)
{
  if (_irq_enable[port])
    {
      _strobe[port] = 1;
      irq();
    }
}

void
PIO::irqreq(void)
{
  DBG(2, form("KCemu/PIO/reti",
	      "PIO::irqreq()\n"));
  z80->set_irq_line(_z80_irq_mask);
}

word_t
PIO::irqack(void)
{
  if (_strobe[A])
    {
      _strobe[A] = 0;
      _irq_active[A] = 1;
      z80->reset_irq_line(_z80_irq_mask);
      return _irq_vector[A];
    }

  if (_irq_active[A])
    return IRQ_NOT_ACK;

  if (_strobe[B])
    {
      _strobe[B] = 0;
      _irq_active[B] = 1;
      z80->reset_irq_line(_z80_irq_mask);
      return _irq_vector[B];
    }

  return IRQ_NOT_ACK;
}

void
PIO::reti(void)
{
  //cout.form("PIO: reti - A: %d, B: %d\n", _irq_active[a], _irq_active[B]);
  if (_irq_active[A])
    _irq_active[A] = 0;
  if (_irq_active[B])
      _irq_active[B] = 0;

  if (_strobe[A])
    {
      trigger_irq(A);
      return;
    }

  if (_strobe[B])
    trigger_irq(B);
}

void
PIO::strobe_A(void)
{
  //_strobe[A] = 1; this blocks the keyboard when the tape loader is active
  trigger_irq(A);
}

void
PIO::strobe_B(void)
{
  //_strobe[B] = 1;
  trigger_irq(B);
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
	 << (int)_value[B] << "h" << endl;
    cout << "\text_mask:        " << hex << setfill('0') << setw(2)
	 << (int)_bit_mode[A]
         << "h           " << hex << setfill('0') << setw(2)
	 << (int)_bit_mode[B] << "h" << endl;
    cout << "\text:             " << hex << setfill('0') << setw(2)
	 << (int)_ext[A]
         << "h           " << hex << setfill('0') << setw(2)
	 << (int)_ext[B] << "h" << endl << endl;
}
