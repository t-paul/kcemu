/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: ui_gtk4.h,v 1.7 2000/05/21 17:38:49 tp Exp $
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

#ifndef __ui_ui_gtk4_h
#define __ui_ui_gtk4_h

#include "ui/gtk/ui_gtk.h"

class UI_Gtk4 : public UI_Gtk
{
 private:
  bool       _flash_enabled;
  int        _flash_idx;
  int        _flash_count;
  word_t     _flash_vec[48];
  word_t    *_flash_v1;
  word_t    *_flash_v2;
  word_t    *_flash_v3;
  long long  _flash_time;
  long long  _frame_time;
  bool       _auto_skip;
  int        _cur_auto_skip;
  int        _max_auto_skip;

 protected:
  void handle_flash(void);
  void reset_flash(bool enable);
  void render_tile(int x, int y);

 protected:
  virtual void allocate_colors(void);
  
 public:
  UI_Gtk4(void);
  virtual ~UI_Gtk4(void);
  virtual void update(bool force_update);
  virtual void memWrite(int addr, char val);
  virtual void callback(void *data);
  virtual void flash(bool enable);

  virtual const char * get_title(void);
  virtual int get_width(void) { return 320; }
  virtual int get_height(void) { return 256; }

  virtual void reset(bool power_on = false);
};

#endif /* __ui_ui_gtk4_h */
