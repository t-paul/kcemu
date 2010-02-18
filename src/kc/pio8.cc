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
#include <iostream>
#include <iomanip>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/pio8.h"

#include "libdbg/dbg.h"

#include "kc/cb.h" // FIXME: DEBUG
#include "cmd/cmd.h" // FIXME: DEBUG

#define memory ((Memory1 *)memory)

using namespace std;

static byte_t head0[] = { 0x00, 0x00, 0x00, 0x20, 0x26, 0x20, 0xd1 };
static byte_t prog0[] = {
  0xdd, 0x21, 0x20, 0x20, 0xcd, 0x5a, 0x08, 0xfe, 0x10, 0x20, 0xf9,
  0x76, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xae,
  0xe3, 0x4f, 0xc2, 0xce, 0x6b
};

static byte_t head1[] = { 0x01, 0x00, 0x00, 0x20, 0x2c, 0x20, 0xc0 };
static byte_t prog1[] = {
  0x21, 0x26, 0x20, 0xe5, 0xdd, 0x21, 0x20, 0x20, 0xdd, 0xe3, 0x06,
  0x32, 0xcd, 0x83, 0x08, 0x10, 0xfb, 0x18, 0xf5, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xae,
  0xe3, 0x4f, 0xc2, 0xce, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static byte_t head2[] = { 0x02, 0x00, 0x00, 0x20, 0x1e, 0x20, 0xd4 };
static byte_t prog2[] = {
  0x3e, 0xff, 0xd3, 0xf5, 0x06, 0x50, 0xcd, 0x18, 0x20, 0x10, 0xfb,
  0x3e, 0xfd, 0xd3, 0xf5, 0x06, 0x50, 0xcd, 0x18, 0x20, 0x10, 0xfb,
  0x18, 0xe8, 0x0e, 0xff, 0x0d, 0x20, 0xfd, 0xc9
};

class TapeTest : public Callback, public CMD {
private:
  int _idx;
  int _state;
  int _1, _2;
  byte_t *_head;
  byte_t *_prog;
  byte_t _prog_len;

public:
  TapeTest(void);
  virtual ~TapeTest(void);
  void callback(void *data);
  void execute(CMD_Args *args, CMD_Context context);
  int send_byte(byte_t *data, int len, int state);
};

TapeTest::TapeTest(void) : Callback("TapeTest"), CMD("TAPETEST")
{
  register_cmd("lc80-key-f10", 0);
  register_cmd("lc80-key-f11", 1);
  register_cmd("lc80-key-f12", 2);
}

TapeTest::~TapeTest(void)
{
}

void
TapeTest::callback(void *data)
{
  static int cnt;

  long i = (long)data;
  if (i == 0)
    {
      pio->set_B_EXT(1, 1);
      return;
    }
  else
    {
      pio->set_B_EXT(1, 0);
    }

  switch (_state)
    {
    case 0:
      cnt = 4000;
      z80->addCallback(500, this, (void *)0);
      z80->addCallback(1000, this, (void *)1);
      //cout << "state => 1" << endl;
      _state = 1;
      break;
    case 1:
      z80->addCallback(500, this, (void *)0);
      z80->addCallback(1000, this, (void *)1);
      if (--cnt == 0)
	{
	  _state = 2;
	  //cout << "state => 2" << endl;
	}
      break;
    case 2:
      cnt = 4000;
      z80->addCallback(200, this, (void *)0);
      z80->addCallback(400, this, (void *)1);
      //cout << "state => 3" << endl;
      _idx = 0;
      _state = 3;
      break;
    case 3:
      _state = send_byte(_head, 7, 3);
      break;
    case 4:
      z80->addCallback(200, this, (void *)0);
      z80->addCallback(400, this, (void *)1);
      if (--cnt == 0)
	{
	  _idx = 0;
	  _state = 5;
	  cnt = 4000;
	  //cout << "state => 5" << endl;
	}
      break;
    case 5:
      _state = send_byte(_prog, _prog_len, 5);
      break;
    case 6:
      z80->addCallback(200, this, (void *)0);
      z80->addCallback(400, this, (void *)1);
      if (--cnt == 0)
	{
	  _state = 7;
	  //cout << "state => 7" << endl;
	}
      break;
    case 7:
      //cout << "TAPE END" << endl;
      break;
    default:
      break;
    }
}

int
TapeTest::send_byte(byte_t *data, int len, int state)
{
  static int i = 0;

  if (_1 == 0)
    {
      if (i == 0)
	{
	  _1 = 3;
	  _2 = 12;
	}
      else if (i == 9)
	{
	  _1 = 6;
	  _2 = 6;
	}
      else if (data[_idx] & (1 << (i - 1)))
	{
	  _1 = 6;
	  _2 = 6;
	}
      else
	{
	  _1 = 3;
	  _2 = 12;
	}
    }

  if (_2 > 0)
    {
      _2--;
      z80->addCallback(200, this, (void *)0);
      z80->addCallback(400, this, (void *)1);
    }
  else if (_1 > 0)
    {
      _1--;
      z80->addCallback(500, this, (void *)0);
      z80->addCallback(1000, this, (void *)1);
    }

  if (_1 == 0)
    {
      i++;
      if (i > 9)
	{
	  i = 0;
	  _idx++;
	  if (_idx >= len)
	    return state + 1;
	}
    }

  return state;
}

void
TapeTest::execute(CMD_Args *args, CMD_Context context)
{
  _idx = 0;
  _state = 0;
  _1 = _2 = 0;

  switch (context)
    {
    case 1:
      _head = head1;
      _prog = prog1;
      _prog_len = sizeof(prog1);
      break;
    case 2:
      _head = head2;
      _prog = prog2;
      _prog_len = sizeof(prog2);
      break;
    default:
      _head = head0;
      _prog = prog0;
      _prog_len = sizeof(prog0);
      break;
    }

  z80->addCallback(0, this, (void *)1);
}

static TapeTest *__tape_test;

void
PIO8_1::draw_leds(void)
{
#if 0
  int a;

  printf("\x1b\x5b\x48");

  // --- line 1 ---------------

  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 4)
	printf(" --- ");
      else
	printf("     ");
    }
  printf("        \n");
  
  // --- line 2 ---------------

  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 2)
	printf("|   ");
      else
	printf("    ");
      
      if (_led_value[a] & 1)
	printf("|");
      else
	printf(" ");
    }
  printf("        \n");

  // --- line 3 ---------------

  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 2)
	printf("|   ");
      else
	printf("    ");
      
      if (_led_value[a] & 1)
	printf("|");
      else
	printf(" ");
    }
  printf("        \n");

  // --- line 4 ---------------
  
  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 8)
	printf(" --- ");
      else
	printf("     ");
    }
  printf("        \n");

  // --- line 5 ---------------

  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 64)
	printf("|   ");
      else
	printf("    ");
      
      if (_led_value[a] & 32)
	printf("|");
      else
	printf(" ");
    }
  printf("        \n");

  // --- line 6 ---------------
  
  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 64)
	printf("|   ");
      else
	printf("    ");
      
      if (_led_value[a] & 32)
	printf("|");
      else
	printf(" ");
    }
  printf("        \n");

  // --- line 7 ---------------

  for (a = 0;a < 6;a++)
    {
      if (_led_value[a] & 128)
	printf(" --- ");
      else
	printf("     ");
    }
  printf("        \n");
  printf("                                      \n");
#endif
}

PIO8_1::PIO8_1(void)
{
  __tape_test = new TapeTest();
  reset(true);
}

PIO8_1::~PIO8_1(void)
{
}

byte_t
PIO8_1::in(word_t addr)
{
  DBG(2, form("KCemu/PIO/8a/in",
              "PIO8_1::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return in_A_DATA();
    case 1:
      return in_B_DATA();
    case 2:
      return in_A_CTRL();
    case 3:
      return in_B_CTRL();
    }

  return 0; // shouldn't be reached
}

void
PIO8_1::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/8a/out",
              "PIO8_1::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO8_1::change_A(byte_t changed, byte_t val)
{
  _led_value_latch = ~val;
}

void
PIO8_1::change_B(byte_t changed, byte_t val)
{
  int selected_led = -1;

  _led_value[6] = (val & 2) ? 1 : 0;

  if (changed & 2)
    tape_signal(_led_value[6]);

  for (int a = 2;a < 8;a++)
    {
      if ((val & (1 << a)) == 0)
	{
	  selected_led = 5 - (a - 2);
	  break;
	}
    }
  if (selected_led < 0)
    return;

  _led_value[selected_led] = _led_value_latch;
  //draw_leds();
}

byte_t
PIO8_1::get_led_value(int index)
{
  if (index < 0)
    return 0;

  if (index > 6)
    return 0;

  return _led_value[index];
}

void
PIO8_1::reset(bool power_on)
{
  for (int a = 0;a < 7;a++)
    _led_value[a] = 0;
}

void
PIO8_1::tape_callback(byte_t val)
{
  //cout << "PIO8_1::tape_callback()" << endl;
}

void
PIO8_1::tape_signal(int val)
{
  int freq = 0;
  long long c;
  static long long diff;
  static long long cnt = 0;
  static int sync = 1000;

  c = z80->getCounter();

  if (val == 1)
    {
      //cout << "(" << dec << c - cnt << "/";
      diff = c - cnt;
      cnt = c;
      return;
    }
  else
    {
      //cout << (c - cnt) << ")";
      diff += c - cnt;
      cnt = c;
    }

  if (diff < 800)
    freq = 2;           // 2 kHz
  else if (diff > 1400)
    sync = 1000;        // pause
  else
    freq = 1;           // 1 kHz

  // SYNC
  if (sync > 0)
    {
      if (freq == 1)
	sync--;
      else
	sync = 1000;
      return;
    }

  if (sync == 0)
    {
      if (freq == 1)
	return;
      sync = -1;
    }


  //cout << "{" << dec << diff << "}";
  tape_bit(freq);
}

void
PIO8_1::tape_bit(int freq)
{
  static int cnt = 0;
  static int byte = 0;
  static int val = 0;
  static bool got1 = false;

  //cout << freq;

  if (freq == 2)
    {
      val++;
      if (got1)
	{
	  int bit = (val >= 0) ? 0 : 1;
	  // cout << " [" << bit << "] " << endl;
	  val = 0;
	  got1 = false;

	  byte >>= 1;
	  byte |= bit << 9;
	  cnt++;
	  if (cnt == 10)
	    {
	      tape_byte(byte);
	      cnt = 0;
	      byte = 0;
	    }
	}
    }
  else
    {
      val -= 2;
      got1 = true;
    }

  //cout << flush;
}

void
PIO8_1::tape_byte(int byte)
{
  byte = (byte >> 1) & 0xff;
  cout << hex << " |" << setw(2) << byte << "|" << endl;
}

PIO8_2::PIO8_2(void)
{
}

PIO8_2::~PIO8_2(void)
{
}

byte_t
PIO8_2::in(word_t addr)
{
  DBG(2, form("KCemu/PIO/8b/in",
              "PIO8_2::in(): addr = %04x\n",
              addr));

  switch (addr & 3)
    {
    case 0:
      return in_A_DATA();
    case 1:
      return in_B_DATA();
    case 2:
      return in_A_CTRL();
    case 3:
      return in_B_CTRL();
    }

  return 0; // shouldn't be reached
}

void
PIO8_2::out(word_t addr, byte_t val)
{
  DBG(2, form("KCemu/PIO/8b/out",
              "PIO8_2::out(): addr = %04x, val = %02x\n",
              addr, val));

  switch (addr & 3)
    {
    case 0:
      out_A_DATA(val);
      break;
    case 1:
      out_B_DATA(val);
      break;
    case 2:
      out_A_CTRL(val);
      break;
    case 3:
      out_B_CTRL(val);
      break;
    }
}

void
PIO8_2::change_A(byte_t changed, byte_t val)
{
}

void
PIO8_2::change_B(byte_t changed, byte_t val)
{
  if (changed & 0x02)
    {
      cout << "tape_signal()" << endl;
      tape->tape_signal();
    }
}
