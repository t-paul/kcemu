/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2003 Torsten Paul
 *
 *  $Id$
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

#include <sys/io.h>
#include <unistd.h>
#include <pthread.h>

#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/pio1.h"
#include "kc/mod_4131.h"

#include "libdbg/dbg.h"

using namespace std;

typedef struct data
{
  word_t port;
  byte_t value;
  long long counter;
  struct data *next;
} data_t;

//static pthread_t thread;
//static data_t *data = NULL;
//static data_t *data_last = NULL;
//static pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
//static FILE *f = NULL;

#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))

/*
static void *
thread_main(void *arg)
{
  data_t *ptr;
  long long old_counter = -1;
  long long tsc, old_tsc, diff, tics;

  cout << "THREAD START" << endl;

  while (true)
    {
      ptr = NULL;
      pthread_mutex_lock(&data_mutex);
      if (data != NULL)
	{
	  ptr = data;
	  data = data->next;
	}
      pthread_mutex_unlock(&data_mutex);
      if (ptr == NULL)
	continue;

      rdtscll(tsc);

      if (old_counter < 0)
	{
	  cout << (long long)ptr->counter << " - " << tsc << " - " << (int)ptr->value << endl;
	  outb(ptr->value, ptr->port);
	  rdtscll(tsc);
	  old_counter = ptr->counter;
	}
      else
	{
	  diff = ptr->counter - old_counter;
	  //cout << "diff: " << diff << endl;
	  //usec = 5 * diff / 2; tics = 800 * usec
	  tics = 2000 * diff;
	  //cout << (long long)tics << " - " << (int)ptr->value << endl;
	  old_counter = ptr->counter;

	  old_tsc = tsc + tics;
	  while (old_tsc > tsc)
	    rdtscll(tsc);

	  outb(ptr->value, ptr->port);
	  rdtscll(tsc);
	}
    }
}
*/

cairo_surface_t *ModuleXY4131::_surface = NULL;

ModuleXY4131::ModuleXY4131(ModuleXY4131 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  _ready = false;

  //if (iopl(3) != 0)
  //return;

  _ready = true;
  pio->register_callback_B_in(this);
  pio->register_callback_B_out(this);

  pio->set_B_EXT(0xff, 0x00);

  _width_cm = 210.0;
  _height_cm = 297.0;
  _line_width = 0.2;
  double mm_to_inch = 72.0 / 25.4;

  _surface = cairo_pdf_surface_create("/tmp/xy4131.pdf", _width_cm * mm_to_inch, _height_cm * mm_to_inch);
  cairo_status_t status = cairo_surface_status(_surface);
  if (status == CAIRO_STATUS_SUCCESS)
    {
      set_valid(true);
      _cr = cairo_create(_surface);
      cairo_surface_destroy(_surface);

      cairo_scale(_cr, mm_to_inch, mm_to_inch);
      cairo_set_line_width(_cr, _line_width);
      cairo_set_line_cap(_cr, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_join(_cr, CAIRO_LINE_JOIN_ROUND);
      cairo_set_source_rgb(_cr, 0, 0, 0);
    }
}

ModuleXY4131::ModuleXY4131(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  _cr = NULL;
  _ready = false;
  set_valid(true);
}

ModuleXY4131::~ModuleXY4131(void)
{
  if (_cr)
    {
      cairo_show_page(_cr);
      cairo_destroy(_cr);
    }

  _cr = NULL;
  _surface = NULL;
}

void
ModuleXY4131::m_out(word_t addr, byte_t val)
{
}

int
ModuleXY4131::callback_A_in(void)
{
  return -1;
}

void
ModuleXY4131::callback_A_out(byte_t val)
{
}

int
ModuleXY4131::callback_B_in(void)
{
  //inb(0x379);
  return -1;
}

void
ModuleXY4131::callback_B_out(byte_t val)
{
  double step;
  float usec;
  static byte_t old_val = 0x00;
  static long long counter = 0;

  static double x = 20;
  static double y = 20;

  /*
  data_t *ptr;
  pthread_mutex_lock(&data_mutex);

  if (data == NULL)
    {
      data = (data_t *)malloc(sizeof(data_t));
      data->port = 0x378;
      data->value = val;
      data->counter = z80->getCounter();
      data->next = NULL;
      data_last = data;
    }
  else
    {
      ptr = (data_t *)malloc(sizeof(data_t));
      ptr->port = 0x378;
      ptr->value = val;
      ptr->counter = z80->getCounter();
      ptr->next = NULL;
      data_last->next = ptr;
      data_last = ptr;
    }

  pthread_mutex_unlock(&data_mutex);
  */

  usec = (z80->getCounter() - counter) / 2.5;

  /*
   *    0,  0 - 100,  0           83/87/83/87/... => X V
   *  100,  0 - 100,100           81/85/81/85/... => Y V
   *  100,100 -   0,100           82/86/82/86/... => X R
   *    0,100 -   0,  0           80/84/80/84/... => Y R
   */

  if (((old_val & 0x80) == 0) && ((val & 0x80) == 0x80))
    {
      cairo_move_to(_cr, x - (_line_width / 2.0), y);
      cairo_rel_line_to(_cr, _line_width, 0);
      cairo_stroke(_cr);
    }

  if (((old_val & 4) == 0) && ((val & 4) == 4))
    {
      step = -0.1;
      if (val & 0x01)
        step = 0.1;

      if (val & 0x02)
        y += step;
      else
        x += step;

      if (x < 0)
        x = 0;
      if (y < 0)
        y = 0;
      if (x > _width_cm)
        x = _width_cm;
      if (y > _height_cm)
        y = _height_cm;

      if ((old_val & 4) != (val & 4))
        {
          if (val & 0x80)
            {
              cairo_line_to(_cr, x, y);
              cairo_stroke(_cr);
            }

          cairo_move_to(_cr, x, y);
        }
    }

  //printf(" [%.2f�s]\n", usec);

  old_val = val;
  counter = z80->getCounter();
}

ModuleInterface *
ModuleXY4131::clone(void)
{
  return new ModuleXY4131(*this);
}

void
ModuleXY4131::reset(bool power_on)
{
}
