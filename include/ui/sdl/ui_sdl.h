/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_sdl.h,v 1.1 2002/06/09 14:24:33 torsten_paul Exp $
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

class UI_SDL
{
 private:
  int _width;
  int _height;
  bool _must_lock;
  bool _fullscreen;
  SDL_Color _colors[256];

 protected:
  SDL_Rect *_rect;
  SDL_Surface *_screen;

 protected:
  void render_tile_2(byte_t *dst, byte_t *src, int x, int y);

 public:
  UI_SDL(void);
  virtual ~UI_SDL(void);

  bool sdl_lock(void);
  void sdl_sync(void);
  void sdl_unlock(void);
  void sdl_update(byte_t *_bitmap, byte_t *_dirty, int width, int height);
  void sdl_init(int width, int height, const char *title);
  void hsv_to_sdl_color(double h, double s, double v, SDL_Color *col);
  void sdl_set_colors(SDL_Color *colors, int numcolors);
  void sdl_process_events(void);
  void sdl_keyboard_handler(SDL_KeyboardEvent *event, bool press);
};

#endif /* __ui_sdl_ui_sdl_h */
