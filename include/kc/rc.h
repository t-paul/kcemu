/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: rc.h,v 1.4 2002/10/31 01:46:33 torsten_paul Exp $
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

#ifndef __kc_rc_h
#define __kc_rc_h

#include <map.h>

struct rc_key_compare
{
  bool operator() (const char *s1, const char *s2) const
    {
      return strcmp(s1, s2) < 0;
    }
};

class RC
{
  private:
  typedef map<const char *, const char *, rc_key_compare> item_map_t;
  
  static RC *_instance;
  item_map_t _map;
  
public:
  typedef item_map_t::value_type rc_map_pair_t;

protected:
  RC(void);
  virtual ~RC(void);

  void load_file(const char *name);
  
public:
  static void init(void);
  static void done(void);
  static RC * instance(void);

  int get_int(const char *key, int def_val = -1);
  const char * get_string(const char *key, const char *def_val = 0);

  int get_int_i(int idx, const char *key, int def_val = -1);
  const char * get_string_i(int idx, const char *key, const char *def_val = 0);
};

#endif /* __kc_rc_h */
