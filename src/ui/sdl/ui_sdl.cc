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

#include <ctype.h>
#include <stdlib.h>

#include <SDL_ttf.h>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"

#include "ui/hsv2rgb.h"

#include "ui/sdl/tape.h"
#include "ui/sdl/ui_sdl.h"

#include "ui/generic/tape.h"
#include "ui/generic/module.h"

#include "ui/sdlmtk/mtk.h"

#include "libdbg/dbg.h"

using namespace std;

class MTK_UICallback : public MTK_Callback
{
  void button_clicked(const char *command, MTK_Button *button)
  {
    CMD_EXEC(command);
  }
  void menubutton_clicked(const char *command, MTK_MenuButton *button)
  {
    CMD_EXEC(command);
  }
  void togglebutton_clicked(const char *command, MTK_ToggleButton *button)
  {
    CMD_EXEC(command);
  }
};

static MTK_UICallback __ui_callback;

class CMD_ui_toggle : public CMD
{
private:
  UI_SDL *_ui;
  
public:
  CMD_ui_toggle(UI_SDL *ui) : CMD("ui-toggle")
    {
      _ui = ui;
      register_cmd("ui-fullscreen-toggle", 0);
      register_cmd("ui-deactivate", 1);
      register_cmd("ui-speed-limit-toggle", 2);
      register_cmd("ui-reset", 3);
      register_cmd("ui-power-on", 4);
      register_cmd("ui-zoom-1", 5);
      register_cmd("ui-zoom-2", 6);
      register_cmd("ui-zoom-3", 7);
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      switch (context)
	{
	case 0:
	  _ui->sdl_fullscreen_toggle();
	  break;
	case 1:
	  _ui->ui_stop();
	  break;
	case 2:
	  _ui->speed_limit_toggle();
	  break;
	case 3:
	  CMD_EXEC("emu-reset");
	  _ui->ui_stop();
	  break;
	case 4:
	  CMD_EXEC("emu-power-on");
	  _ui->ui_stop();
	  break;
	case 5:
	case 6:
	case 7:
	  _ui->sdl_zoom(context - 4);
	  break;
	}
    }
};

UI_SDL::UI_SDL(void)
{
  _screen = 0;
  _shift = false;
  _control = false;
  _ui_active = false;
  _show_cursor = true;
  _speed_limit = true;
  memset(_colors, 0, sizeof(_colors));

  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  _rmask = 0xff000000;
  _gmask = 0x00ff0000;
  _bmask = 0x0000ff00;
  _amask = 0x000000ff;
#else
  _rmask = 0x000000ff;
  _gmask = 0x0000ff00;
  _bmask = 0x00ff0000;
  _amask = 0xff000000;
#endif

  _cmd_ui_toggle = new CMD_ui_toggle(this);
}

UI_SDL::~UI_SDL(void)
{
  delete _cmd_ui_toggle;
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
UI_SDL::ui_start(void)
{
  _ui_active = true;
  _show_cursor = true;
  SDL_ShowCursor(SDL_ENABLE);

  _background = SDL_CreateRGBSurface(SDL_HWSURFACE, _screen->w, _screen->h, 24, _rmask, _gmask, _bmask, 0);
  SDL_BlitSurface(_screen, NULL, _background, NULL);
  _ui = SDL_CreateRGBSurface(SDL_HWSURFACE, _screen->w, _screen->h, 24, _rmask, _gmask, _bmask, 0);
  SDL_SetAlpha(_ui, SDL_SRCALPHA, 200);

  MTK_Screen::get_instance()->render(_ui, true);
  MTK_Screen::get_instance()->blit(_screen, _background, _ui, true);
}

void
UI_SDL::ui_stop(void)
{
  _ui_active = false;

  SDL_BlitSurface(_background, NULL, _screen, NULL);
  SDL_UpdateRect(_screen, 0, 0, 0, 0);
  SDL_FreeSurface(_ui);
  SDL_FreeSurface(_background);

  _ui = NULL;
  _background = NULL;
}

void
UI_SDL::ui_update(bool clear_cache)
{
  if (MTK_Screen::get_instance()->render(_ui))
    MTK_Screen::get_instance()->blit(_screen, _background, _ui, clear_cache);
}

void
UI_SDL::render_tile_1(byte_t *dst, byte_t *src, int x, int y, int width, int height)
{
  int xx, yy;

  src += y * width + x;
  dst += 3 * (y * width + x);

  for (yy = 0;yy < 8;yy++)
    {
      for (xx = 0;xx < 8;xx++)
	{
	  byte_t val = *(src + xx);
	  dst[3 * xx    ] = _colors[val].b;
	  dst[3 * xx + 1] = _colors[val].g;
	  dst[3 * xx + 2] = _colors[val].r;
	}
      src += width;
      dst += width + width + width;
    }
}

void
UI_SDL::render_tile_2(byte_t *dst, byte_t *src, int x, int y, int width, int height)
{
  int w, xx, yy;

  w = 6 * width;
  src += y * width + x;
  dst += 12 * y * width + 6 * x;

  for (yy = 0;yy < 8;yy++)
    {
      for (xx = 0;xx < 8;xx++)
	{
	  byte_t val = *(src + xx);
	  dst[6 * xx    ] = _colors[val].b;
	  dst[6 * xx + 1] = _colors[val].g;
	  dst[6 * xx + 2] = _colors[val].r;
	  dst[6 * xx + 3] = _colors[val].b;
	  dst[6 * xx + 4] = _colors[val].g;
	  dst[6 * xx + 5] = _colors[val].r;

	  dst[6 * xx + w    ] = _colors[val].b;
	  dst[6 * xx + w + 1] = _colors[val].g;
	  dst[6 * xx + w + 2] = _colors[val].r;
	  dst[6 * xx + w + 3] = _colors[val].b;
	  dst[6 * xx + w + 4] = _colors[val].g;
	  dst[6 * xx + w + 5] = _colors[val].r;
	}
      src += width;
      dst += 12 * width;
    }
}

void
UI_SDL::render_tile_3(byte_t *dst, byte_t *src, int x, int y, int width, int height)
{
  int i, w, xx, yy;

  w = 9 * width;
  src += y * width + x;
  dst += 27 * y * width + 9 * x;

  for (yy = 0;yy < 8;yy++)
    {
      for (xx = 0;xx < 8;xx++)
	{
	  byte_t r, g, b;
	  byte_t val = *(src + xx);

	  r = _colors[val].r;
	  g = _colors[val].g;
	  b = _colors[val].b;

	  i = 9 * xx;
	  dst[i    ] = b;
	  dst[i + 1] = g;
	  dst[i + 2] = r;
	  dst[i + 3] = b;
	  dst[i + 4] = g;
	  dst[i + 5] = r;
	  dst[i + 6] = b;
	  dst[i + 7] = g;
	  dst[i + 8] = r;

	  i += w;
	  dst[i    ] = b;
	  dst[i + 1] = g;
	  dst[i + 2] = r;
	  dst[i + 3] = b;
	  dst[i + 4] = g;
	  dst[i + 5] = r;
	  dst[i + 6] = b;
	  dst[i + 7] = g;
	  dst[i + 8] = r;

	  r = (2 * r) / 3;
	  g = (2 * g) / 3;
	  b = (2 * b) / 3;

	  i += w;
	  dst[i    ] = b;
	  dst[i + 1] = g;
	  dst[i + 2] = r;
	  dst[i + 3] = b;
	  dst[i + 4] = g;
	  dst[i + 5] = r;
	  dst[i + 6] = b;
	  dst[i + 7] = g;
	  dst[i + 8] = r;
	}
      src += width;
      dst += 27 * width;
    }
}

void
UI_SDL::sdl_update(byte_t *bitmap, byte_t *dirty, int width, int height, bool clear_cache)
{
  int z, rect_update;

  if (_ui_active)
    {
      ui_update(clear_cache);
      return;
    }
  
  if (!sdl_lock())
    return;

  byte_t *pixels = (byte_t *)_screen->pixels;
  
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
	      _rect[rect_update].x = kcemu_ui_scale * x;
	      _rect[rect_update].y = kcemu_ui_scale * y;
	      _rect[rect_update].w = kcemu_ui_scale * 8;
	      _rect[rect_update].h = kcemu_ui_scale * 8;
	      rect_update++;
	      
	      switch (kcemu_ui_scale)
		{
		case 1:
		  render_tile_1(pixels, bitmap, x, y, width, height);
		  break;
		case 2:
		  render_tile_2(pixels, bitmap, x, y, width, height);
		  break;
		case 3:
		  render_tile_3(pixels, bitmap, x, y, width, height);
		default:
		  break;
		}
	    }
	}
    }
  sdl_unlock();

  if (rect_update == 0)
    return;

  SDL_UpdateRects(_screen, rect_update, _rect);
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
      DBG(2, form("KCemu/UI/update",
                  "counter = %lu, frame = %lu, timeframe = %lu\n",
                  (unsigned long)z80->getCounter(), frame, timeframe));
      frame = timeframe;
    }

  if (_speed_limit)
    {
      if (frame > (timeframe + 1))
	SDL_Delay(20 * (frame - timeframe - 1));
    }
  else
    {
      frame = timeframe;
    }
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
  memcpy(_colors, colors, numcolors * sizeof(SDL_Color));
  _colors[240].r = 0;
  _colors[240].g = 0;
  _colors[240].b = 0;
  _colors[241].r = 20;
  _colors[241].g = 20;
  _colors[241].b = 20;
  _colors[242].r = 50;
  _colors[242].g = 50;
  _colors[242].b = 50;
  _colors[243].r = 255;
  _colors[243].g = 255;
  _colors[243].b = 255;
  _colors[244].r = 255;
  _colors[244].g = 0;
  _colors[244].b = 0;
  _colors[245].r = 0;
  _colors[245].g = 255;
  _colors[245].b = 0;
}

void
UI_SDL::sdl_keyboard_handler(SDL_KeyboardEvent *event, bool press)
{
  int c;

  if (_show_cursor)
    {
      _show_cursor = false;
      SDL_ShowCursor(SDL_DISABLE);
    }

  switch (event->keysym.sym)
    {
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
      c = KC_KEY_SHIFT;
      _shift = event->type == SDL_KEYDOWN;
      break;
    case SDLK_RCTRL:
    case SDLK_LCTRL:
      c = KC_KEY_CONTROL;
      _control = event->type == SDL_KEYDOWN;
      break;
    case SDLK_RALT:
    case SDLK_LALT:   c = KC_KEY_ALT;     break;
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

    default:
      c = event->keysym.unicode & 0xff;
      break;
    }

  if (press)
    keyboard->keyPressed(c, event->keysym.scancode);
  else
    keyboard->keyReleased(c, event->keysym.scancode);
}

void
UI_SDL::sdl_zoom(int zoom)
{
  if (zoom < 1)
    zoom = 1;
  if (zoom > 3)
    zoom = 3;

  kcemu_ui_scale = zoom;
  if (_ui_active)
    ui_stop();

  sdl_resize();
  update(true, true);
}

void
UI_SDL::sdl_fullscreen_toggle(void)
{
  kcemu_ui_fullscreen = !kcemu_ui_fullscreen;

  if (SDL_WM_ToggleFullScreen(_screen))
    return;

  /*
   *  Argh, SDL_WM_ToggleFullScreen didn't work. This seems to
   *  happen with the windows drivers. We kludge around this
   *  by using the sdl_resize() function to allocate a new
   *  surface with the SDL_FULLSCREEN flag set and hope this
   *  will work out as expected.
   */
  sdl_resize();
  update(true, true);
}


void
UI_SDL::sdl_process_events(void)
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
    {
      if (_ui_active)
	MTK_Screen::get_instance()->handle_event(&event);

      switch(event.type)
	{
	case SDL_KEYDOWN:
	  sdl_keyboard_handler(&event.key, true);
	  break;
	case SDL_KEYUP:
	  sdl_keyboard_handler(&event.key, false);
	  break;
	case SDL_MOUSEMOTION:
	  if (!_show_cursor)
	    {
	      _show_cursor = true;
	      SDL_ShowCursor(SDL_ENABLE);
	    }
	  break;
	case SDL_MOUSEBUTTONUP:
	  if (event.button.button == SDL_BUTTON_RIGHT)
	    {
	      if (_ui_active)
		ui_stop();
	      else
		ui_start();
	    }
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
UI_SDL::sdl_resize(void)
{
  int flags = SDL_SWSURFACE | SDL_HWSURFACE;

  _sdl_width = get_width();
  _sdl_height = get_height();
  _rect = new SDL_Rect[(_sdl_width * _sdl_height) / 64];

  if (kcemu_ui_fullscreen)
    flags |= SDL_FULLSCREEN;

  if (_screen)
    SDL_FreeSurface(_screen);

  _screen = SDL_SetVideoMode(_sdl_width, _sdl_height, 24, flags);
  if (_screen == NULL)
    {
      printf("SDL_SetVideoMode(): Error!\n");
      exit(1);
    }

  allocate_colors(0.4, 0.5, 0.8, 0.6, 0.1, 0.9);
  SDL_SetColors(_screen, _colors, 0, 256);
  SDL_WM_SetCaption(get_title(), "KCemu");
  SDL_EnableUNICODE(1);

  _must_lock = SDL_MUSTLOCK(_screen);
}

void
UI_SDL::speed_limit_toggle(void)
{
  _speed_limit = !_speed_limit;
}

void
UI_SDL::show(void)
{
  sdl_resize();
}

void
UI_SDL::init(int *argc, char ***argv)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      printf("SDL_Init(): Error!\n");
      exit(1);
    }
  atexit(SDL_Quit);

  if (TTF_Init() < 0)
    {
      fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
      exit(2);
    }
  atexit(TTF_Quit);

  MTK_MenuButton *menu;
  MTK_HBox *box = new MTK_HBox(6);
  menu = new MTK_MenuButton("File");
  menu->add(new MTK_Button("Run"));
  menu->add(new MTK_Button("Load"));
  menu->add(new MTK_Button("Tape"));
  menu->add(new MTK_Button("Disk"));
  menu->add(new MTK_Button("Module"));
  menu->add(new MTK_Button("Audio"));
  menu->add(new MTK_Button("Quit", "emu-quit"));
  box->add(menu);

  menu = new MTK_MenuButton("View");
  menu->add(new MTK_Button("Zoom 1x", "ui-zoom-1"));
  menu->add(new MTK_Button("Zoom 2x", "ui-zoom-2"));
  menu->add(new MTK_Button("Zoom 3x", "ui-zoom-3"));
  menu->add(new MTK_ToggleButton("Fullscreen", "ui-fullscreen-toggle", kcemu_ui_fullscreen));
  box->add(menu);

  menu = new MTK_MenuButton("System");
  menu->add(new MTK_Button("Continue", "ui-deactivate"));
  menu->add(new MTK_Button("Reset", "ui-reset"));
  menu->add(new MTK_Button("Power On", "ui-power-on"));
  menu->add(new MTK_ToggleButton("No Speed Limit", "ui-speed-limit-toggle"));
  box->add(menu);
  
  menu = new MTK_MenuButton("Help");
  menu->add(new MTK_Button("About"));
  menu->add(new MTK_Button("Licence"));
  menu->add(new MTK_Button("No Warranty!"));
  box->add(menu);
  box->set_x(6);
  box->set_y(2);

  MTK_Screen::get_instance()->add(box);
  MTK_Screen::get_instance()->set_visible(true);

  MTK_Screen::get_instance()->set_callback(&__ui_callback);
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
