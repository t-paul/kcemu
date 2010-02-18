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
#include "kc/gdc.h"
#include "kc/vis.h"

#include "ui/ui.h"
#include "ui/generic/ui_9.h"

UI_9::UI_9(void)
{
  _dirty = NULL;
  _bitmap = NULL;
  _pix_cache = NULL;
  _col_cache = NULL;

  generic_set_mode(UI_GENERIC_MODE_LORES);
  
  init();
}

UI_9::~UI_9(void)
{
  dispose();
}

void
UI_9::init(void)
{
  int a;

  _max_ptr = get_real_width() * get_real_height();

  _dirty_size = (get_real_width() * get_real_height()) / 64;
  _dirty = new byte_t[_dirty_size];

  _bitmap = new byte_t[get_real_width() * get_real_height()];
  _pix_cache = new byte_t[_dirty_size];
  _col_cache = new byte_t[_dirty_size];

  for (a = 0;a < _dirty_size;a++)
    _dirty[a] = 1;

  for (a = 0;a < _dirty_size;a++)
    {
      _pix_cache[a] = 0;
      _col_cache[a] = 0;
    }
}

void
UI_9::dispose(void)
{
  if (_dirty)
    delete[] _dirty;
  if (_bitmap)
    delete[] _bitmap;
  if (_pix_cache)
    delete[] _pix_cache;
  if (_col_cache)
    delete[] _col_cache;
}

void
UI_9::set_pixel(int x, int y, int color)
{
  int yaddr = y * get_real_width();

  if (yaddr >= _max_ptr)
    return;

  if (_bitmap[yaddr + x] == color)
    return;

  _bitmap[yaddr + x] = color;
  _dirty[(y / 8) * (get_real_width() / 8) + x / 8] = 0xff;
}

void
UI_9::set_char(int x, int y, int c, int fg, int bg, int width, int lines)
{
  byte_t *chr = vis->get_memory();
  int xx, yy, pix, col_fg, col_bg, addr;

  addr = width * y + x;

  c *= 8;
  x *= 8;
  y *= lines;
  y += 20;

  // move smaller screen a little bit to
  // the center of the window
  if (lines < 10)
    y += (10 - lines) * 12;
  
  for (yy = 0;yy < 8;yy++)
    {
      pix = chr[c];

      if (gdc->get_cursor(addr, yy))
	{
	  col_fg = bg;
	  col_bg = fg; // not truncated to 3 bit!
	}
      else
	{
	  col_fg = fg;
	  col_bg = bg;
	}

      for (xx = 0;xx < 8;xx++)
	{
	  if (pix & (1 << xx))
	    set_pixel(x + xx + 32, y, col_fg);
	  else
	    set_pixel(x + xx + 32, y, col_bg);
	}

      y++;
      c++;
    }

  // fill space between lines (if any) with the background color
  for (yy = 8;yy < lines;yy++)
    {
      for (xx = 0;xx < 8;xx++)
	set_pixel(x + xx + 32, y, bg);
      y++;
    }
}

void
UI_9::generic_update_border(int border, int lines)
{
  int x, y, top;

  lines++;
  top = 20 + (10 - lines) * 12;

  for (x = 0;x < get_real_width();x++)
    for (y = 0;y < top;y++)
      {
	set_pixel(x, y, border);
	set_pixel(x, y + top + 25 * lines, border);
      }

  for (x = 0;x < 32;x++)
    for (y = 0;y < 25 * lines;y++)
      {
	set_pixel(x, top + y, border);
	set_pixel(get_real_width() - x - 1, top + y, border);
      }
}

void
UI_9::generic_update_text(int width, int height, int lines, bool clear_cache)
{
  int x, y, z, p, c, fg, bg;

  long offset = gdc->get_pram_SAD(0);

  z = 0;
  for (y = 0;y < height;y++)
    {
      if ((lines * y) == gdc->get_pram_LEN(0))
	offset = gdc->get_pram_SAD(1) - z; // compensate value of loop variable!

      for (x = 0;x < width;x++)
	{
	  int changed = _dirty[z];

	  p = gdc->get_mem(z + offset);
	  c = gdc->get_col(z + offset);

	  if (p != _pix_cache[z])
	    {
	      changed++;
	      _pix_cache[z] = p;
	    }

	  if (c != _col_cache[z])
	    {
	      changed++;
	      _col_cache[z] = c;
	    }

	  if (vis->get_changed(p))
	    changed++;

	  if (gdc->get_cursor(z + offset))
	    {
	      changed++;
	      _pix_cache[z] = 255; // invalidate cache on cursor position to force
	      _col_cache[z] = 255; // update there when cursor position changes
	    }

	  z++;

	  if (!changed)
	    continue;

	  fg = c & 15;
	  bg = (c >> 5) & 7;

	  set_char(x, y, p, fg, bg, width, lines);
	}
    }

  vis->reset_changed();
}

void
UI_9::generic_update_graphic_2(bool clear_cache)
{
  int a, x, y, col;

  long offset = gdc->get_pram_SAD(0);

  for (y = 0;y < 200;y++)
    for (x = 0;x < 320;x++)
      {
	a = x & 7;
	col = (gdc->get_mem(offset + 40 * y + x / 8) >> a) & 1;
	col += 2 * ((gdc->get_col(offset + 40 * y + x / 8) >> a) & 1);
	set_pixel(x + 32, y + 44, vis->get_color(col));
      }
}

void
UI_9::generic_update_graphic_3(bool clear_cache)
{
  int a, x, y, col;

  long offset = gdc->get_pram_SAD(0);

  for (y = 0;y < 200;y++)
    for (x = 0;x < 640;x++)
      {
	a = x & 7;
	col = (gdc->get_mem(offset + 80 * y + x / 8) >> a) & 1;
	col += 2 * ((gdc->get_col(offset + 80 * y + x / 8) >> a) & 1);
	set_pixel(x + 32, y + 44, vis->get_color(col));
      }
}

void
UI_9::generic_update_graphic_5(bool clear_cache)
{
  int a, x, y, mem, col, val;

  long offset = gdc->get_pram_SAD(0);

  for (y = 0;y < 200;y++)
    for (x = 0;x < 80;x++)
      {
	mem = gdc->get_mem(offset + 80 * y + x);
	col = gdc->get_col(offset + 80 * y + x);

	for (a = 0;a < 4;a++)
	  {
	    val = (mem >> a) & 1;
	    val += (mem >> (a + 3)) & 2;
	    val += ((4 * col) >> a) & 4;
	    val += (col >> (a + 1)) & 8;
	    set_pixel(4 * x + a + 32, y + 44, val);
	  }
      }
}

void
UI_9::generic_update(Scanline *scanline, MemAccess *memaccess, bool clear_cache)
{
  int mode;
  int border;
  int screen_on;
  int nr_of_lines;
  static int old_mode = -1;
  static int old_border = -1;
  static int old_screen_on = -1;
  static int old_nr_of_lines = -1;

  screen_on = gdc->get_screen_on();
  if (screen_on != old_screen_on)
    {
      old_screen_on = screen_on;
      if (!screen_on)
	{
	  memset(_bitmap, 0, get_real_width() * get_real_height());
	  memset(get_dirty_buffer(), 0xff, get_dirty_buffer_size());
	  return;
	}
    }

  mode = vis->get_mode();
  border = vis->get_border();
  nr_of_lines = gdc->get_nr_of_lines();

  if (mode != old_mode)
    {
      // call to ui->set_mode below will trigger new update, we need to
      // make sure that we do not get into a loop.
      old_mode = mode;
      clear_cache = true;
      switch (mode)
	{
	case 0:
	case 2:
	case 5:
	  ui->set_mode(UI_GENERIC_MODE_LORES);
	  break;
	case 1:
	case 3:
	  ui->set_mode(UI_GENERIC_MODE_HIRES);
	  break;
	}
    }

  if (border != old_border)
    clear_cache = true;

  if (nr_of_lines != old_nr_of_lines)
    clear_cache = true;

  if (vis->is_color_palette_changed())
    {
      clear_cache = true;
      vis->reset_color_palette_changed();
    }

  old_border = border;
  old_nr_of_lines = nr_of_lines;

  if (clear_cache)
    {
      memset(_bitmap, 0, get_real_width() * get_real_height());
      memset(get_dirty_buffer(), 0xff, get_dirty_buffer_size());
      generic_update_border(border, nr_of_lines);
    }

  switch (mode)
    {
    case 0:
      generic_update_text(40, 25, nr_of_lines + 1, clear_cache);
      break;
    case 1:
      generic_update_text(80, 25, nr_of_lines + 1, clear_cache);
      break;
    case 2:
      generic_update_graphic_2(clear_cache);
      break;
    case 3:
      generic_update_graphic_3(clear_cache);
      break;
    case 5:
      generic_update_graphic_5(clear_cache);
      break;
    }
}

void
UI_9::generic_signal_v_retrace(bool value)
{
  gdc->v_retrace(value);
}

int
UI_9::generic_get_mode(void)
{
  return _mode;
}

void
UI_9::generic_set_mode(int mode)
{
  _mode = mode;

  switch (_mode)
    {
    case UI_GENERIC_MODE_LORES:
      set_real_screen_size(384, 288);
      break;
    case UI_GENERIC_MODE_HIRES:
      set_real_screen_size(704, 288);
      break;
    }

  dispose();
  init();
}
