/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tape.cc,v 1.1 2002/06/09 14:24:34 torsten_paul Exp $
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

#include "kc/system.h"

#ifdef HAVE_PARAGUI

#include "ui/sdl/tape.h"
#include "ui/sdl/ui_sdl.h"

class CMD_ui_tape_window_toggle : public CMD
{
private:
  TapeWindow *_w;

public:
  CMD_ui_tape_window_toggle(TapeWindow *w) : CMD("ui-tape-window-toggle")
    {
      _w = w;
      register_cmd("ui-tape-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->Show();
      UI_SDL::ui_visible(true);
    }
};

class CMD_ui_tape_load_selected : public CMD
{
private:
  TapeWindow *_w;
public:
  CMD_ui_tape_load_selected(TapeWindow *w) : CMD("ui-tape-load-selected")
    {
      _w = w;
      register_cmd("ui-tape-load-selected", 0);
      register_cmd("ui-tape-run-selected", 1);
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      if (args == NULL)
        args = new CMD_Args();
        
      args->set_string_arg("tape-filename",
                           _w->tapeGetName(_w->tapeGetSelected()));
                           
      switch (context)
        {
          case 0:
            CMD_EXEC_ARGS("tape-load", args);
            break;
          case 1:
            CMD_EXEC_ARGS("tape-run", args);
            break;
        }
    }
};

  
TapeWindow::TapeWindow(PG_Widget *widget, const PG_Rect &rect, const char *text)
  : PG_Window(widget, rect, text)
{
  cout << "SDL: TapeWindow::TapeWindow()" << endl;

  CMD *cmd;
  cmd = new CMD_ui_tape_window_toggle(this);
  cmd = new CMD_ui_tape_load_selected(this);
}

TapeWindow::~TapeWindow(void)
{
}
  
void
TapeWindow::tapeAttach(const char *name)
{
}

void
TapeWindow::tapeDetach(void)
{
}

void
TapeWindow::tapePower(bool power)
{
}

void
TapeWindow::tapeProgress(int val)
{
}

void
TapeWindow::tapeNext(void)
{
}

void
TapeWindow::tapeAddFile(const char *name, long load, long start, long size, unsigned char type)
{
}

void
TapeWindow::tapeRemoveFile(int idx)
{
}

int
TapeWindow::tapeGetSelected(void)
{

}

const char *
TapeWindow::tapeGetName(int idx)
{
  return "NAME";
}

void
TapeWindow::tapeAddArchive(const char *name)
{
}

#endif /* HAVE_PARAGUI */
