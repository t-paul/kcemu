/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tape.h,v 1.1 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __ui_beos_tape_h
#define __ui_beos_tape_h

#include <Window.h>

#include "kc/system.h"

#include "ui/tape.h"

#include "ui/beos/ui_beos.h"

class TapeWindow : public UI_Window, public TapeInterface
{
 private:
  BListView *_list;
  BScrollView *_scroll_view;
  
 protected:
  virtual void ui_message(BMessage *message);
  
 public:
  TapeWindow(BRect rect, const char *name);
  virtual ~TapeWindow(void);

  virtual bool QuitRequested(void);
  
  /*
   *  TapeInterface
   */
  virtual void tapeAttach(const char *name);
  virtual void tapeDetach(void);
  virtual void tapePower(bool power);
  virtual void tapeProgress(int val);
  virtual void tapeNext(void);
  virtual void tapeAddFile(const char *name, long load,
                           long start, long size,
			               unsigned char type);
  virtual void tapeRemoveFile(int idx);
  virtual int  tapeGetSelected(void);
  virtual const char * tapeGetName(int idx);
  virtual void tapeAddArchive(const char *name);
};

#endif /* __ui_beos_tape_h */
