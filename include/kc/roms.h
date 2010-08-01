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
#include <list>
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
  RomRegistryEntry(const char *id, const char *name, const char *filename, const char *directory, int size) : _id(id), _name(name), _filename(filename), _directory(directory), _size(size)
  {
  }

  const char * get_id() const;
  const char * get_name() const;
  const char * get_filename() const;
  const char * get_directory() const;
  int get_size() const;
};

typedef list<const RomRegistryEntry *> rom_registry_entry_list_t;

class RomRegistry
{
public:
  typedef map<string, const RomRegistryEntry *> rom_entry_map_t;
private:
  static const RomRegistry * _instance;

  rom_entry_map_t _entries;
  
protected:
  void add_rom(const char *name, const char *id, const char *filename, const char *directory, int size);

public:
  RomRegistry(void);
  virtual ~RomRegistry(void);

  const list<const RomRegistryEntry *> * get_missing(const string directory) const;
  bool save_rom(const RomRegistryEntry *entry, byte_t *buf, unsigned int size) const;

  bool is_missing(const RomRegistryEntry *entry) const;
  int count_missing_roms(void) const;
  int count_missing_roms(const string directory) const;
  const RomRegistryEntry * get_rom(const char *id) const;

  static const RomRegistry * instance();
};

#endif /* __kc_roms_h */
