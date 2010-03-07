/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define DBG_LEVEL 3
#include "libdbg/dbg.h"

using namespace std;

int
main(void)
{
  DBG_add_path("test/path/s1");
  cout << *DBGI();
  DBG_add_path("test/path/s2");
  cout << *DBGI();

  DBG_clear();

  DBG_add_path("test/path2/s3");
  cout << *DBGI();

  DBG(0, form("test/path/s1", "%s -> %d\n", "hallo", 3));
  DBG(1, form("test/path/s4", "%s -> %d\n", "hallo", 3));
}
