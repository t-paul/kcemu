/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: ui_beos.h,v 1.1 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __ui_beos_ui_beos_h
#define __ui_beos_ui_beos_h

#include <OS.h>
#include <View.h>
#include <Window.h>
#include <Application.h>

#include "ui/ui.h"
#include "ui/errorl.h"
#include "ui/statusl.h"
#include "ui/commands.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

class TapeWindow;

typedef struct {
  byte_t red;
  byte_t green;
  byte_t blue;
} color_t;

class App : public BApplication
{
 public:
  App(const char *signature);
  virtual ~App(void);

  virtual void MessageReceived(BMessage *message);
};

class UI_Window : public BWindow
{
 public:
  UI_Window(BRect rect, const char *name);
  virtual ~UI_Window(void);

  virtual void toggle(void);
  virtual void ui_message(BMessage *message);
  virtual void MessageReceived(BMessage *message);
};

class MainWindow : public UI_Window
{
 public:
  MainWindow(BRect rect, const char *name);
  virtual ~MainWindow(void);

  virtual void Quit(void);
};

class Canvas : public BView
{
 private:
  bool _down;
 
 protected:
  void KeyDownOrUp(const char *bytes, int32 numBytes, bool press);

 public:
  Canvas(BRect rect);
  virtual ~Canvas(void);

  virtual void AttachedToWindow(void);
  virtual void KeyDown(const char *bytes, int32 numBytes);
  virtual void KeyUp(const char *bytes, int32 numBytes);
};

class UI_BeOS
{
  private:
    BBitmap *_bbitmap;
    sem_id _ui_semaphor;
    color_t *_color_map;

  protected:    
    BView *_canvas;
    MainWindow *_main_window;
    TapeWindow *_tape_window;

  protected:
    void sync(void);
    void ui_main(void);
    void set_color_map(color_t *color_map, int ncolors);
    void rgb_to_beos_color(int r, int g, int b, color_t *col);
    void hsv_to_beos_color(double h, double s, double v, color_t *col);
    static int32 ui_main_static(void *ui);

  protected:
    void copy_bitmap(byte_t *bitmap, int width, int height);
    
  public:
    UI_BeOS(void);
    virtual ~UI_BeOS(void);
    virtual void processEvents(void);

    static BMessage * get_message(const char *string);

    virtual void allocate_colors(double saturation_fg,
                                 double saturation_bg,
                                 double brightness_fg,
                                 double brightness_bg,
                                 double black_level,
                                 double white_level) = 0;

    virtual void update(bool full_update = false, bool clear_cache = false) = 0;
    virtual void memWrite(int addr, char val) = 0;
    virtual void callback(void *data) = 0;
    virtual void flash(bool enable) = 0;

    virtual void init(int *argc, char ***argv);
    virtual const char * get_title(void) = 0;
    virtual int get_width(void) = 0;
    virtual int get_height(void) = 0;

    /*
     *  StatusListener
     */
    void setStatus(const char *msg);

    /*
     *  ErrorListener
     */
    void errorInfo(const char *msg);
};

#endif /* __ui_beos_ui_h */
