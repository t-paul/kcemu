/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_beos.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include <Window.h>
#include <Bitmap.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Application.h>

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

#include "ui/hsv2rgb.h"
#include "ui/beos/tape.h"
#include "ui/beos/module.h"
#include "ui/beos/ui_beos.h"

App::App(const char *signature) : BApplication(signature)
{
}

App::~App(void)
{
}

void App::MessageReceived(BMessage *message)
{
  const char *cmd;
  UI_Window *target;

  if (message->what != 'BEKC')
    return;

  if (message->FindPointer("target", (void **)&target) == B_OK)
    {
      target->ui_message(message);
      return;
    }

  if (message->FindString("cmd", &cmd) == B_OK)
    CMD_EXEC(cmd);
}

UI_Window::UI_Window(BRect rect, const char *name) : BWindow(rect, name, B_TITLED_WINDOW, 0)
{
}

UI_Window::~UI_Window(void)
{
}

void
UI_Window::toggle(void)
{
  thread_info info;
  
  if (get_thread_info(find_thread(NULL), &info) == B_OK)
    cout << "current thread: '" << info.name << "'" << endl;

  if (Lock())
    {
      if (IsHidden())
        Show();
      else
        Hide();

      Unlock();
    }
}

void
UI_Window::ui_message(BMessage *message)
{
}

void
UI_Window::MessageReceived(BMessage *message)
{
  if (message->what != 'BEKC')
    return;

  be_app->PostMessage(message);
}

MainWindow::MainWindow(BRect rect, const char *name) : UI_Window(rect, name)
{
}

MainWindow::~MainWindow(void)
{
}

void
MainWindow::Quit(void)
{
  CMD_EXEC("emu-quit");
  BWindow::Quit();
}

Canvas::Canvas(BRect rect) : BView(rect, "Drawing Area", B_FOLLOW_NONE, 0)
{
}

Canvas::~Canvas(void)
{
}

void
Canvas::AttachedToWindow(void)
{
	MakeFocus();
}

void
Canvas::KeyDownOrUp(const char *bytes, int32 numBytes, bool press)
{
  int32 c, key_code;
  
  BMessage *msg = Window()->CurrentMessage();
  if (msg) {
    if (msg->FindInt32("key", &key_code) < B_OK)
      key_code = 0;
  }
  
  switch (*bytes) {
    case B_FUNCTION_KEY:
      switch (key_code) {
        case B_F1_KEY:  c = KC_KEY_F1;  break;
        case B_F2_KEY:  c = KC_KEY_F2;  break;
        case B_F3_KEY:  c = KC_KEY_F3;  break;
        case B_F4_KEY:  c = KC_KEY_F4;  break;
        case B_F5_KEY:  c = KC_KEY_F5;  break;
        case B_F6_KEY:  c = KC_KEY_F6;  break;
        case B_F7_KEY:  c = KC_KEY_F7;  break;
        case B_F8_KEY:  c = KC_KEY_F8;  break;
        case B_F9_KEY:  c = KC_KEY_F9;  break;
        case B_F10_KEY: c = KC_KEY_F10; break;
        case B_F11_KEY: c = KC_KEY_F11; break;
        case B_F12_KEY: c = KC_KEY_F12; break;
        default:        c = 0;          break;
      }
    case B_RETURN:      c = 0x0d;         break;
    case B_BACKSPACE:
    case B_DELETE:      c = KC_KEY_DEL;   break;
    case B_UP_ARROW:    c = KC_KEY_UP;    break;
    case B_DOWN_ARROW:  c = KC_KEY_DOWN;  break;
    case B_LEFT_ARROW:  c = KC_KEY_LEFT;  break;
    case B_RIGHT_ARROW: c = KC_KEY_RIGHT; break;
    default:
      c =  *bytes;
      break;
  }
  
  if (press)
    keyboard->keyPressed(c, key_code);
  else
    keyboard->keyReleased(c, key_code);
}

void
Canvas::KeyDown(const char *bytes, int32 numBytes)
{
  /*
   *  ignore key repeat, this is handled by the keyboard
   *  class itself
   */
  if (_down)
    return;

  _down = true;
  KeyDownOrUp(bytes, numBytes, true);
}

void
Canvas::KeyUp(const char *bytes, int32 numBytes)
{
  KeyDownOrUp(bytes, numBytes, false);
  _down = false;
}

UI_BeOS::UI_BeOS(void)
{
}

UI_BeOS::~UI_BeOS(void)
{
}

void
UI_BeOS::set_color_map(color_t *color_map, int ncolors)
{
   _color_map = color_map;
}

/*
void
UI_BeOS1::put_pixels(byte_t *bits, int x, int y, byte_t val, color_t *fg, color_t *bg)
{
  int a;
  int idx = y * _bitmap->BytesPerRow() + 4 * x;

  for (a = 0;a < 8;a++)
    {
      if (val & 1)
        {
          bits[idx++] = fg->blue;
          bits[idx++] = fg->green;
          bits[idx++] = fg->red;
        }
      else
        {
          bits[idx++] = bg->blue;
          bits[idx++] = bg->green;
          bits[idx++] = bg->red;
        }
      idx++;
      val >>= 1;
    }
}
*/


void
UI_BeOS::copy_bitmap(byte_t *bitmap, int width, int height)
{
  int a, x, y, idx, yadd;
  byte_t *bits = (byte_t *)_bbitmap->Bits();

  a = 0;
  idx = 0;
  yadd = _bbitmap->BytesPerRow();
  switch (kcemu_ui_scale)
    {
    case 1:
      for (y = 0;y < height;y++)
        {
          for (x = 0;x < width;x++)
            {
              bits[idx + 4 * x    ] = _color_map[bitmap[a]].blue;
              bits[idx + 4 * x + 1] = _color_map[bitmap[a]].green;
              bits[idx + 4 * x + 2] = _color_map[bitmap[a]].red;
              a++;
            }
          idx += yadd;
        }
      break;
    case 2:
      byte_t *bit2 = &bits[yadd];
      for (y = 0;y < height;y++)
        {
          for (x = 0;x < width;x++)
            {
              bits[idx + 8 * x    ] = _color_map[bitmap[a]].blue;
              bits[idx + 8 * x + 1] = _color_map[bitmap[a]].green;
              bits[idx + 8 * x + 2] = _color_map[bitmap[a]].red;
              bits[idx + 8 * x + 4] = _color_map[bitmap[a]].blue;
              bits[idx + 8 * x + 5] = _color_map[bitmap[a]].green;
              bits[idx + 8 * x + 6] = _color_map[bitmap[a]].red;
              bit2[idx + 8 * x    ] = _color_map[bitmap[a]].blue;
              bit2[idx + 8 * x + 1] = _color_map[bitmap[a]].green;
              bit2[idx + 8 * x + 2] = _color_map[bitmap[a]].red;
              bit2[idx + 8 * x + 4] = _color_map[bitmap[a]].blue;
              bit2[idx + 8 * x + 5] = _color_map[bitmap[a]].green;
              bit2[idx + 8 * x + 6] = _color_map[bitmap[a]].red;
              a++;
            }
          idx += yadd;
          idx += yadd;
        }
      break;
    }
  BWindow *w = _canvas->Window();
  if (w->Lock())
    {
      _canvas->DrawBitmap(_bbitmap);
      w->Unlock();
    }
}

void
UI_BeOS::sync(void)
{
  unsigned long timeframe;

  static bigtime_t base;
  static bool first = true;
  static unsigned long frame = 25;

  if (first)
    {
      first = false;
      base = (system_time() / 20000) - 26;
    }

  timeframe = (system_time() / 20000) - base;
  frame++;

  if (frame < (timeframe - 20))
    {
      cout << "update: frame = " << frame << ", timeframe = " << timeframe << endl;
      frame = timeframe;
    }

  if (frame > (timeframe + 1))
    snooze(20000 * (frame - timeframe - 1));
}

void
UI_BeOS::processEvents(void)
{
}

void
UI_BeOS::memWrite(int addr, char val)
{
}

void
UI_BeOS::callback(void *data)
{
}

void
UI_BeOS::flash(bool enable)
{
}

void
UI_BeOS::init(int *argc, char ***argv)
{
  _ui_semaphor = create_sem(0, "UI Thread Semaphor");
  int32 tid = spawn_thread(UI_BeOS::ui_main_static, "UI Thread", B_NORMAL_PRIORITY, this);
  resume_thread(tid);
  acquire_sem(_ui_semaphor); // wait for ui thread to start up
  delete_sem(_ui_semaphor);
}

void
UI_BeOS::setStatus(const char *msg)
{
}

void
UI_BeOS::errorInfo(const char *msg)
{
}

void
UI_BeOS::hsv_to_beos_color(double h, double s, double v, color_t *col)
{
  int r, g, b;

  hsv2rgb(h, s, v, &r, &g, &b);
  col->red = r;
  col->green = g;
  col->blue = b;
}

void
UI_BeOS::rgb_to_beos_color(int r, int g, int b, color_t *col)
{
  col->red = r;
  col->green = g;
  col->blue = b;
}

int32
UI_BeOS::ui_main_static(void *ui)
{
	UI_BeOS *self = (UI_BeOS *)ui;
	self->ui_main();
	return 0;
}

BMessage *
UI_BeOS::get_message(const char *string)
{
    BMessage *message = new BMessage('BEKC');
    message->AddString("cmd", string);
    return message;
}

void
UI_BeOS::ui_main(void)
{
	/*
	 *  main of the user interface thread
	 */
    BRect rect(0, 0, get_width(), get_height());

    new App("application/x-vnd.kcemu");

    _bbitmap = new BBitmap(BRect(0, 0, get_width(), get_height()), B_RGB32);

	_main_window = new MainWindow(BRect(20, 60, get_width() + 19, get_height() + 59), get_title());
	_main_window->SetFlags(B_NOT_RESIZABLE);
    _tape_window = new TapeWindow(BRect(420, 60, 700, 400), "BeKC: Tape");

	BMenuBar *menubar = new BMenuBar(BRect(0, 0, get_width(), 10), "MenuBar");
    _main_window->AddChild(menubar);

    BMenu *menu = new BMenu("File");
    menubar->AddItem(menu);
    menu->AddItem(new BMenuItem("Run...", get_message("kc-image-run")));
    menu->AddItem(new BMenuItem("Load...", get_message("kc-image-load")));
    menu->AddItem(new BMenuItem("Tape...", get_message("ui-tape-window-toggle")));
    menu->AddItem(new BMenuItem("Disk...", get_message("ui-disk-window-toggle")));
    menu->AddItem(new BMenuItem("Module...", get_message("ui-module-window-toggle")));
    menu->AddSeparatorItem();
    menu->AddItem(new BMenuItem("Reset", get_message("emu-reset")));
    menu->AddItem(new BMenuItem("Power On", get_message("emu-power-on")));
    menu->AddSeparatorItem();
    menu->AddItem(new BMenuItem("Quit Emulator", get_message("emu-quit")));

    menu = new BMenu("View");
    menubar->AddItem(menu);
    menu->AddItem(new BMenuItem("Debugger", get_message("ui-debug-window-toggle")));
    menu->AddItem(new BMenuItem("Info", get_message("ui-info-window-toggle")));
    menu->AddItem(new BMenuItem("Menubar", get_message("ui-menu-bar-toggle")));
    menu->AddItem(new BMenuItem("Statusbar", get_message("ui-status-bar-toggle")));
    
    menu = new BMenu("Configuration");
    menubar->AddItem(menu);
    menu->AddItem(new BMenuItem("Colors", get_message("ui-color-window-toggle")));
    
    menu = new BMenu("Help");
    menubar->AddItem(menu);
    menu->AddItem(new BMenuItem("About KCemu", get_message("ui-about-window-toggle")));
    menu->AddSeparatorItem();
    menu->AddItem(new BMenuItem("KCemu Licence", get_message("ui-copying-window-toggle")));
    menu->AddItem(new BMenuItem("No Warranty!", get_message("ui-warranty-window-toggle")));

    rect.OffsetBy(menubar->Bounds().LeftBottom());
    _canvas = new Canvas(rect);
	_main_window->AddChild(_canvas);
	_main_window->ResizeTo(rect.right, rect.bottom);

	allocate_colors(0.4, 0.5, 0.8, 0.6, 0.1, 0.9);

	release_sem(_ui_semaphor);
	_main_window->Show();
	be_app->Run();
}