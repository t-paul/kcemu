/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_sdl.cc,v 1.2 2002/10/31 01:16:25 torsten_paul Exp $
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

#include <stdlib.h>

#include "kc/system.h"

#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"

#include "ui/hsv2rgb.h"

#include "ui/sdl/tape.h"
#include "ui/sdl/ui_sdl.h"

#include "ui/generic/tape.h"
#include "ui/generic/module.h"

using namespace std;

UI_SDL::UI_SDL(void)
{
}

UI_SDL::~UI_SDL(void)
{
}

bool
UI_SDL::sdl_lock(void)
{
  if (_must_lock)
    return SDL_LockSurface(_screen);

  return true;
}

void
UI_SDL::sdl_unlock(void)
{
  if (_must_lock)
    SDL_UnlockSurface(_screen);
}

void
UI_SDL::render_tile_2(byte_t *dst, byte_t *src, int x, int y)
{
  int xx, yy;

  src += y * 320 + x;
  dst += y * 1280 + 2 * x;

  for (yy = 0;yy < 8;yy++)
    {
      for (xx = 0;xx < 8;xx++)
	{
	  byte_t val = *(src + xx);
	  dst[xx * 2      ] = val;
	  dst[xx * 2 +   1] = val;
	  dst[xx * 2 + 640] = val;
	  dst[xx * 2 + 641] = val;
	}
      src += 320;
      dst += 1280;
    }
}

void
UI_SDL::sdl_update(byte_t *bitmap, byte_t *dirty, int width, int height)
{
  byte_t *src, *dst;
  int z, rect_update;
  
  if (!sdl_lock())
    return;

  byte_t *pixels = (byte_t *)_screen->pixels;

  switch (kcemu_ui_scale)
    {
    case 1:
      rect_update = -1;
      memcpy(pixels, bitmap, width * height);
      sdl_unlock();
      break;
    case 2:
      src = bitmap;
      dst = pixels;

      z = -1;
      rect_update = 0;
      for (int y = 0;y < height;y += 8)
	{
	  for (int x = 0;x < width;x += 8)
	    {
	      z++;
	      if (dirty[z])
		{
		  dirty[z] = 0;
		  rect_update++;
		  _rect[rect_update].x = 2 * x;
		  _rect[rect_update].y = 2 * y;
		  render_tile_2(dst, src, x, y);
		}
	    }
	}
      sdl_unlock();
      break;
    default:
      sdl_unlock();
      break;
    }

  if (rect_update == 0)
    return;

  if (rect_update > 0)
    SDL_UpdateRects(_screen, rect_update + 1, _rect);
  else
    SDL_UpdateRect(_screen, 0, 0, 0, 0);
}

void
UI_SDL::sdl_sync(void)
{
  unsigned long timeframe;

  static Uint32 base;
  static bool first = true;
  static unsigned long frame = 25;

  if (first)
    {
      first = false;
      base = (SDL_GetTicks() / 20) - 26;
    }

  timeframe = (SDL_GetTicks() / 20) - base;
  frame++;
  
  if (frame < (timeframe - 20))
    {
      cout << "update: frame = " << frame << ", timeframe = " << timeframe << endl;
      frame = timeframe;
    }

  if (frame > (timeframe + 1))
    SDL_Delay(20 * (frame - timeframe - 1));
}

void
UI_SDL::sdl_init(int width, int height, const char *title)
{
  int flags = SDL_SWSURFACE | SDL_HWSURFACE;

  _width = width;
  _height = height;

  _rect = new SDL_Rect[24 * 80]; // FIXME:
  for (int a = 0;a < 24 * 80;a++)
    {
      _rect[a].x = 0;
      _rect[a].y = 0;
      _rect[a].w = 16;
      _rect[a].h = 16;
    }

  memset(_colors, 0, sizeof(_colors));

  _screen = 0;
  _fullscreen = true;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      printf("SDL_Init(): Error!\n");
      exit(1);
    }
  atexit(SDL_Quit);

  _screen = SDL_SetVideoMode(_width, _height, 8, flags);
  if (_screen == NULL)
    {
      printf("SDL_SetVideoMode(): Error!\n");
      exit(1);
    }
  SDL_SetColors(_screen, _colors, 0, 256);
  _must_lock = SDL_MUSTLOCK(_screen);

  SDL_WM_SetCaption(title, "KCemu");
}

void
UI_SDL::hsv_to_sdl_color(double h, double s, double v, SDL_Color *col)
{
  int r, g, b;

  hsv2rgb(h, s, v, &r, &g, &b);
  col->r = r;
  col->g = g;
  col->b = b;
}

void
UI_SDL::sdl_set_colors(SDL_Color *colors, int numcolors)
{
  SDL_SetColors(_screen, colors, 0, numcolors);
  memcpy(_colors, colors, numcolors * sizeof(SDL_Color));
}

void
UI_SDL::sdl_keyboard_handler(SDL_KeyboardEvent *event, bool press)
{
  int c;

  switch (event->keysym.sym)
    {
    case SDLK_RALT:
    case SDLK_LALT:   c = KC_KEY_ALT;     break;
    case SDLK_RCTRL:
    case SDLK_LCTRL:  c = KC_KEY_CONTROL; break;
    case SDLK_RSHIFT:
    case SDLK_LSHIFT: c = KC_KEY_SHIFT;   break;
    case SDLK_UP:     c = KC_KEY_UP;      break;
    case SDLK_DOWN:   c = KC_KEY_DOWN;    break;
    case SDLK_LEFT:   c = KC_KEY_LEFT;    break;
    case SDLK_RIGHT:  c = KC_KEY_RIGHT;   break;
    case SDLK_F1:     c = KC_KEY_F1;      break;
    case SDLK_F2:     c = KC_KEY_F2;      break;
    case SDLK_F3:     c = KC_KEY_F3;      break;
    case SDLK_F4:     c = KC_KEY_F4;      break;
    case SDLK_F5:     c = KC_KEY_F5;      break;
    case SDLK_F6:     c = KC_KEY_F6;      break;
    case SDLK_F7:     c = KC_KEY_F7;      break;
    case SDLK_F8:     c = KC_KEY_F8;      break;
    case SDLK_F9:     c = KC_KEY_F9;      break;
    case SDLK_F10:    c = KC_KEY_F10;     break;
    case SDLK_F11:    c = KC_KEY_F11;     break;
    case SDLK_F12:    c = KC_KEY_F12;     break;
    case SDLK_F13:    c = KC_KEY_F13;     break;
    case SDLK_F14:    c = KC_KEY_F14;     break;
    case SDLK_F15:    c = KC_KEY_F15;     break;

    case SDLK_PAUSE:
      if (!press)
	SDL_WM_ToggleFullScreen(_screen);
      return;

    case SDLK_PRINT:
      CMD_EXEC("emu-quit");
      return;

    default:
      c = event->keysym.sym & 0xff;
      break;
    }

  if (press)
    keyboard->keyPressed(c, event->keysym.scancode);
  else
    keyboard->keyReleased(c, event->keysym.scancode);
}

void
UI_SDL::sdl_process_events(void)
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
    {
      switch(event.type)
	{
	case SDL_KEYDOWN:
	  sdl_keyboard_handler(&event.key, true);
	  break;
	case SDL_KEYUP:
	  sdl_keyboard_handler(&event.key, false);
	  break;
	case SDL_QUIT:
	  CMD_EXEC("emu-quit");
	  break;
	default:
	  break;
	}
    }
}

void
UI_SDL::init(int *argc, char ***argv)
{
}

UI_ModuleInterface *
UI_SDL::getModuleInterface(void)
{
  static UI_ModuleInterface *i = new DummyModuleHandler();
  return i;
}

TapeInterface *
UI_SDL:: getTapeInterface(void)
{
  static TapeInterface *i = new DummyTapeHandler();
  return i;
}

DebugInterface *
UI_SDL::getDebugInterface(void)
{
  return NULL;
}
