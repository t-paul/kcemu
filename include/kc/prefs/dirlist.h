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
