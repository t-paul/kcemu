/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
 *
 *  $Id$
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

#include "kc/prefs/strlist.h"

StringList::StringList(string str, const char split_char)
{
    string::size_type lastPos = str.find_first_not_of(split_char, 0);
    string::size_type pos     = str.find_first_of(split_char, lastPos);

    while ((string::npos != pos) || (string::npos != lastPos))
    {
        string substr = str.substr(lastPos, pos - lastPos);
        push_back(substr);
        lastPos = str.find_first_not_of(split_char, pos);
        pos = str.find_first_of(split_char, lastPos);
    }    
}

StringList::~StringList(void)
{
}

string
StringList::join(const char join_char)
{
  string ret;

  for (StringList::iterator it = begin();it != end();it++)
    {
      if (ret.length() > 0)
        ret += join_char;

      ret += *it;
    }

  return ret;
}
