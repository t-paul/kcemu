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
static FILE *f = NULL;

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

ModuleXY4131::ModuleXY4131(ModuleXY4131 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type())
{
  ready = false;

  //if (iopl(3) != 0)
  //return;

  ready = true;
  pio->register_callback_B_in(this);
  pio->register_callback_B_out(this);

  f = fopen("/tmp/plotter.log", "wb+");
  fprintf(f, "%%!PS-Adobe-3.0\n"
	  "%%%%Creator: KCemu\n"
	  "%%%%CreationDate: D:20030114204616\n"
	  "%%%%LanguageLevel: 2\n"
	  "%%%%Pages: 1\n"
	  "%%%%BoundingBox: 0 0 596 842\n"
	  "%%%%PageOrder: Ascend\n"
	  "%%%%Title: Plotter Output\n"
	  "%%%%EndComments\n"
	  "%%%%Page: New document 1 1\n"
	  "/m { moveto } def\n"
	  "/l { lineto } def\n"
	  "/p { currentpoint currentlinewidth 2 div sub lineto\n"
          "     currentpoint currentlinewidth add lineto } def\n"
	  "0.1 setlinewidth\n"
	  "0 0 0 setrgbcolor\n");

  // pthread_create(&thread, NULL, thread_main, NULL);

  set_valid(true);
}

ModuleXY4131::ModuleXY4131(const char *name) :
  ModuleInterface(name, 0, KC_MODULE_KC_85_1)
{
  ready = false;
}

ModuleXY4131::~ModuleXY4131(void)
{
  if (f)
    {
      fprintf(f, "stroke\n"
	      "showpage\n"
	      "%%%%Trailer\n"
	      "%%%%EOF\n");
      fclose(f);
      f = NULL;
    }
}

void
ModuleXY4131::m_out(word_t addr, byte_t val)
{
}

void
ModuleXY4131::callback_A_in(void)
{
}

void
ModuleXY4131::callback_A_out(byte_t val)
{
}

void
ModuleXY4131::callback_B_in(void)
{
  //inb(0x379);
}

void
ModuleXY4131::callback_B_out(byte_t val)
{
  int step;
  float usec;
  static byte_t old_val = 0x00;
  static long long counter = 0;

  static int x = 0;
  static int y = 0;

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
    fprintf(f, "p\n");

  if (((old_val & 4) == 0) && ((val & 4) == 4))
    {
      step = -1;
      if (val & 0x01)
	step = 1;
      
      if (val & 0x02)
	y += step;
      else
	x += step;
      
      if (x < 0)
	x = 0;
      if (y < 0)
	y = 0;
      if (x > (596 * 3))
	x = 596 * 3;
      if (y > (842 * 3))
	y = 842 * 3;
      
      if ((old_val & 4) != (val & 4))
	fprintf(f, "%.2f %.2f %s\n",
		(x / 3.0),
		842 - (y / 3.0),
		val & 0x80 ? "l" : "m");
    }

  //printf(" [%.2fµs]\n", usec);

  old_val = val;
  counter = z80->getCounter();
}

ModuleInterface *
ModuleXY4131::clone(void)
{
  return new ModuleXY4131(*this);
}
