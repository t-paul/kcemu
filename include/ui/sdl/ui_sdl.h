/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2003 Torsten Paul
 *
 *  $Id: ui_sdl.h,v 1.2 2002/10/31 01:16:25 torsten_paul Exp $
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

#ifndef __ui_sdl_ui_sdl_h
#define __ui_sdl_ui_sdl_h

#include <SDL/SDL.h>

#include "ui/ui.h"

class UI_SDL : public UI
{
 private:
  bool _shift;
  bool _control;
  bool _ui_active;
  bool _show_cursor;
  int _sdl_width;
  int _sdl_height;
  bool _must_lock;
  bool _speed_limit;
  SDL_Color _colors[256];
  Uint32 _rmask, _gmask, _bmask, _amask;

  CMD *_cmd_ui_toggle;

 protected:
  SDL_Rect *_rect;
  SDL_Surface *_ui;
  SDL_Surface *_screen;
  SDL_Surface *_background;

 protected:
  void render_tile_1(byte_t *dst, byte_t *src, int x, int y, int width, int height);
  void render_tile_2(byte_t *dst, byte_t *src, int x, int y, int width, int height);
  void render_tile_3(byte_t *dst, byte_t *src, int x, int y, int width, int height);

 public:
  UI_SDL(void);
  virtual ~UI_SDL(void);

  bool sdl_lock(void);
  void sdl_sync(void);
  void sdl_unlock(void);
  void sdl_resize(void);
  void sdl_zoom(int zoom);
  void sdl_fullscreen_toggle(void);
  void sdl_update(byte_t *_bitmap, byte_t *_dirty, int width, int height, bool clear_cache);

  void ui_start(void);
  void ui_stop(void);
  void ui_update(bool clear_cache);
  void speed_limit_toggle(void);

  void hsv_to_sdl_color(double h, double s, double v, SDL_Color *col);
  void sdl_set_colors(SDL_Color *colors, int numcolors);
  void sdl_process_events(void);
  void sdl_keyboard_handler(SDL_KeyboardEvent *event, bool press);

  virtual void show(void);
  virtual void init(int *argc, char ***argv);

  virtual int get_width(void) = 0;
  virtual int get_height(void) = 0;
  virtual const char * get_title(void) = 0;

  virtual void allocate_colors(double saturation_fg,
			       double saturation_bg,
			       double brightness_fg,
			       double brightness_bg,
			       double black_level,
			       double white_level) = 0;

  /*
   *  interface handling
   */
  virtual UI_ModuleInterface * getModuleInterface(void);
  virtual TapeInterface  * getTapeInterface(void);
  virtual DebugInterface * getDebugInterface(void);
};

#endif /* __ui_sdl_ui_sdl_h */
