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

#include "ui/generic/tape.h"

DummyTapeHandler::DummyTapeHandler(void)
{
}

DummyTapeHandler::~DummyTapeHandler(void)
{
}
  
void
DummyTapeHandler::tapeAttach(const char *name)
{
}

void
DummyTapeHandler::tapeDetach(void)
{
}

void
DummyTapeHandler::tapePower(bool power)
{
}

void
DummyTapeHandler::tapeProgress(int val)
{
}

void
DummyTapeHandler::tapeNext(void)
{
}

void
DummyTapeHandler::tapeAddFile(const char *name, long load, long start, long size, unsigned char type)
{
}

void
DummyTapeHandler::tapeRemoveFile(int idx)
{
}

int
DummyTapeHandler::tapeGetSelected(void)
{
  return 0;
}

const char *
DummyTapeHandler::tapeGetName(int idx)
{
  return "<none>";
}

void
DummyTapeHandler::tapeAddArchive(const char *name)
{
}
