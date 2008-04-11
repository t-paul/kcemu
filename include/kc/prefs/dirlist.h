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

#ifndef __kc_prefs_dirlist_h
#define __kc_prefs_dirlist_h

#include <list>
#include <string>

using namespace std;

class DirectoryListEntry
{
private:
    string _dir;
    string _filename;
    string _path;

public:
    DirectoryListEntry(string dir, string filename);
    virtual ~DirectoryListEntry(void);
    
    const char *get_dir(void) const;
    const char *get_path(void) const;
    const char *get_filename(void) const;
};

class DirectoryList : public list<DirectoryListEntry *>
{
    string _basedir;
    
public:
    DirectoryList(string basedir);
    virtual ~DirectoryList(void);
};

#endif /* __kc_prefs_dirlist_h */
