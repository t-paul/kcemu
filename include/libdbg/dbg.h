/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-1998 Torsten Paul
 *
 *  $Id: dbg.h,v 1.1 2000/05/21 17:37:30 tp Exp $
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

#ifndef __libdbg_dbg_h
#define __libdbg_dbg_h

#include <string.h>
#include <iostream.h>
#include <stdarg.h>

#include <map.h>
#include <vector.h>

struct str_cmp_func : public binary_function<const char *, const char *, bool>{
  bool operator()(const char *x, const char *y) const
    {
      return strcmp(x, y) < 0;
    }
};

class p_map : public map<const char *, p_map *, str_cmp_func>
{
private:
  char *_name;
  bool _allow_subkeys;

public:
  p_map(const char *name, bool allow_subkeys);
  virtual ~p_map(void);

  const char * get_name(void);
  void set_allow_subkeys(bool allow_subkeys);
  bool get_allow_subkeys(void);
};

class p_tree
{
private:
  p_map *_map;

public:
  p_tree(void);
  virtual ~p_tree(void);

  void add(const char *data, bool allow_subkeys);
  bool check_path(const char *data);

  void dump(ostream& os, p_map *map, int level) const;
  friend ostream& operator<< (ostream& os, const p_tree& t);
};
  
class DBG_class
{
 private:
  static DBG_class *_singleton;

  p_tree _tree;
  ostream *_o;

 protected:
  DBG_class(void);
  virtual ~DBG_class(void);

  void load_file(const char *filename);

 public:
  static DBG_class * instance(void);
  
  void add_path(const char *path, bool allow_subkeys = false);
  void print(const char *path, const char *msg);
  void println(const char *path, const char *msg);
  void form(const char *path, const char *format ...);
  bool check(const char *path);

  friend ostream& operator<< (ostream& os, const DBG_class &dbg);
};

#define DBGI() DBG_class::instance()

#define DBG_X(x,f,l) \
  do { \
    DBGI()->x; \
  } while (0)

#define DBG_0(x) DBG_X(x,__FILE__,__LINE__)

#if DBG_LEVEL > 0
#define DBG_1(x) DBG_X(x,__FILE__,__LINE__)
#else
#define DBG_1(x)
#endif

#if DBG_LEVEL > 1
#define DBG_2(x) DBG_X(x,__FILE__,__LINE__)
#else
#define DBG_2(x)
#endif

#if DBG_LEVEL > 2
#define DBG_3(x) DBG_X(x,__FILE__,__LINE__)
#else
#define DBG_3(x)
#endif

#define DBG(x, y) DBG_##x(y)
        
#define DBG_check(x)      (DBGI()->check(x))
#define DBG_add_path(x)   do { DBGI()->add_path(x);       } while (0)
#define DBG_add_path_t(x) do { DBGI()->add_path(x, true); } while (0)
#define DBG_print(x,y)    do { DBGI()->print(x,y);        } while (0)
#define DBG_println(x,y)  do { DBGI()->println(x,y);      } while (0)

#endif /* __libdbg_dbg_h */
