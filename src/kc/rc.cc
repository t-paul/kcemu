/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: rc.cc,v 1.2 2000/05/21 16:54:37 tp Exp $
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream.h>

#include <algo.h>

#include "kc/config.h"
#include "kc/system.h"

#include "kc/kc.h"
#include "kc/rc.h"

#include "libdbg/dbg.h"

RC *RC::_instance = 0;

static void print(RC::rc_map_pair_t p)
{
  DBG(0, form("KCemu/RC",
              "- %-40s <=> %s\n",
              p.first, p.second));
}
                  
RC::RC(void)
{
  char *filename, *tmp;
  
  /*
   *  inserting into the map rejects new items that have a
   *  key that is already present
   *  so we need to read the personal file first
   */
  tmp = getenv("HOME");
  if (tmp)
    {
      filename = new char[strlen(tmp) + 10];
      strcpy(filename, tmp);
      strcat(filename, "/.kcemurc");
      load_file(filename);
      delete filename;
    }
  else
    cerr << "Warning: HOME not set! can't locate file `.kcemurc'" << endl;

  filename = new char [strlen(kcemu_datadir) + 10];
  strcpy(filename, kcemu_datadir);
  strcat(filename, "/.kcemurc");
  load_file(filename);
  delete filename;

  DBG(0, form("KCemu/RC",
              "--- dumping ressource database ---\n\n"));
  for_each(_map.begin(), _map.end(), print);
  DBG(0, form("KCemu/RC",
              "\n----------------------------------\n"));
}

void
RC::load_file(const char *filename)
{
  int c;
  ifstream is;
  char *line, *name, *arg, *tmp;
  
  is.open(filename);
  if (!is) return;
  while (242)
    {
      is.gets(&line);
      if (!line) break;

      name = line;
      while ((*name == ' ') || (*name == '\t')) name++;
      if (*name != '#')
        {
          arg = strchr(name, ':');
          if (arg && (arg != name))
            {
              tmp = arg - 1;
              *arg++ = '\0';
              while ((*tmp == ' ') || (*tmp == '\t')) *tmp-- = '\0';
              while ((*arg == ' ') || (*arg == '\t')) arg++;
              tmp = strchr(arg, '\0');
              if (tmp && (tmp != arg))
                {
                  tmp--;
                  while ((*tmp == ' ') || (*tmp == '\t')) *tmp-- = '\0';
                  _map.insert(rc_map_pair_t(strdup(name), strdup(arg)));
                }
            }
        }
      delete line;
    }
}

void
RC::init(void)
{
  if (_instance == 0)
    _instance = new RC();
}

RC *
RC::instance(void)
{
  if (_instance == 0)
    {
      cerr.form("RC::instance(void): RC::init() not called!\n");
      exit(1);
    }
  return _instance;
}

int
RC::get_int(const char *key, int def_val)
{
  long val;
  char *endptr;
  const char *ptr;
  item_map_t::iterator it;
  
  if (!key) return def_val;
  it = _map.find(key);
  if (it != _map.end())
    {
      ptr = (*it).second;
      val = strtol(ptr, &endptr, 0);
      if ((*ptr != '\0') && (*endptr == '\0'))
        return val;
    }
  return def_val;
}

const char *
RC::get_string(const char *key, const char *def_val)
{
  item_map_t::iterator it;

  if (!key) return def_val;
  it = _map.find(key);
  if (it != _map.end())
    return (*it).second;
  return def_val;  
}

int
RC::get_int_i(int idx, const char *key, int def_val)
{
  char *tmp;
  int val;

  if (!key) return def_val;
  tmp = new char[strlen(key) + 10];
  sprintf(tmp, "%s_%08X", key, idx);
  val = get_int(tmp, def_val);
  delete tmp;
  return val;
}

const char *
RC::get_string_i(int idx, const char *key, const char *def_val)
{
  char *tmp;
  const char *val;

  if (!key) return def_val;
  tmp = new char[strlen(key) + 10];
  sprintf(tmp, "%s_%08X", key, idx);
  val = get_string(tmp, def_val);
  delete tmp;
  return val;
}
