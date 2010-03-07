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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "kc/prefs/dirlist.h"

DirectoryListEntry::DirectoryListEntry(string dir, string filename) : _dir(dir), _filename(filename)
{
    _path = _dir + "/" + _filename;
}

DirectoryListEntry::~DirectoryListEntry(void)
{
}

const char *
DirectoryListEntry::get_dir(void) const {
    return _dir.c_str();
}

const char *
DirectoryListEntry::get_path(void) const {
    return _path.c_str();
}

const char *
DirectoryListEntry::get_filename(void) const {
    return _filename.c_str();
}

DirectoryList::DirectoryList(string basedir) : _basedir(basedir) {
    DIR *dir;
    struct stat statbuf;
    struct dirent *entry;
    
    dir = opendir(_basedir.c_str());
    if (dir == NULL)
        return;

    while (true) {
        entry = readdir(dir);
        if (entry == NULL)
            break;
        
        string filename = _basedir + "/" + entry->d_name;
        
        if (stat(filename.c_str(), &statbuf) != 0)
            continue;
        
        if (!S_ISREG(statbuf.st_mode))
            continue;
        
        push_back(new DirectoryListEntry(_basedir, entry->d_name));
    }
    
    closedir(dir);
}

DirectoryList::~DirectoryList(void) {
}
