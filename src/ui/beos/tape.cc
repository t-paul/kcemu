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

#include <ListView.h>
#include <ScrollView.h>

#include "ui/beos/tape.h"
#include "ui/beos/ui_beos.h"

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
      _w->toggle();
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

TapeWindow::TapeWindow(BRect rect, const char *name) : UI_Window(rect, name)
{
  BRect rect(Bounds());

  rect.right -= B_V_SCROLL_BAR_WIDTH;
  _list = new BListView(rect, "TapeList", B_SINGLE_SELECTION_LIST, B_FOLLOW_ALL_SIDES);
    _list->SetInvocationMessage(UI_BeOS::get_message("ui-tape-run-selected"));
  _scroll_view = new BScrollView("ScrollView", _list, B_FOLLOW_ALL_SIDES, 0, false, true);
    AddChild(_scroll_view);

    CMD *cmd;
  cmd = new CMD_ui_tape_window_toggle(this);
    cmd = new CMD_ui_tape_load_selected(this);
}

TapeWindow::~TapeWindow(void)
{
}
  
bool
TapeWindow::QuitRequested(void)
{
  CMD_EXEC("ui-tape-window-toggle");
  return false;
}

void
TapeWindow::ui_message(BMessage *message)
{
  const char *cmd;

  if (message->what != 'BEKC')
    return;

  if (message->FindString("cmd", &cmd) != B_OK)
    return;

  if (!Lock())
    return;

  if (strcmp("tapeAttach", cmd) == 0)
    {
		_list->MakeEmpty(); // FIXME: fix memory leak
    }
  else if (strcmp("tapeAddFile", cmd) == 0)
    {
      const char *name;
      if (message->FindString("name", &name) == B_OK)
        _list->AddItem(new BStringItem(name));
    }

		Unlock();
}

void
TapeWindow::tapeAttach(const char *name)
{
  static BMessage *message = 0;

  if (message == 0)
    {
      message = new BMessage('BEKC');
      message->AddPointer("target", this);
      message->AddString("cmd", "tapeAttach");
      message->AddString("name", "");
	}

  message->ReplaceString("name", name);
  be_app->PostMessage(message);
}

void
TapeWindow::tapeDetach(void)
{
	cout << "tapeDetach(): " << endl;
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
  static BMessage *message = 0;

  if (message == 0)
    {
      message = new BMessage('BEKC');
      message->AddPointer("target", this);
      message->AddString("cmd", "tapeAddFile");
      message->AddString("name", "");
  }

  message->ReplaceString("name", name);

  be_app->PostMessage(message);
}

void
TapeWindow::tapeRemoveFile(int idx)
{
}

int
TapeWindow::tapeGetSelected(void)
{
	return _list->CurrentSelection(0);
}

const char *
TapeWindow::tapeGetName(int idx)
{
    BStringItem *item = (BStringItem *)_list->ItemAt(idx);
    return item->Text();
}

void
TapeWindow::tapeAddArchive(const char *name)
{
}
