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

#ifndef __kc_roms_h
#define __kc_roms_h

#include <map>
#include <string>

using namespace std;

class RomRegistryEntry
{
private:
  const char *_id;
  const char *_name;
  const char *_filename;
  const char *_directory;
  const int _size;

public:
  RomRegistryEntry(const char *name, const char *id, const char *filename, const char *directory, int size) : _id(id), _name(name), _filename(filename), _directory(directory), _size(size)
  {
  }
};

class RomRegistry
{
private:
  static const RomRegistry * _self;

  map<string, RomRegistryEntry *> _entries;
  
protected:
  void add_rom(const char *name, const char *id, const char *filename, const char *directory, int size);

public:
  RomRegistry(void);
  virtual ~RomRegistry(void);

  static const RomRegistry * instance();
};

#endif /* __kc_roms_h */
