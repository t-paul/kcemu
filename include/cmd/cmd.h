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

#ifndef __cmd_cmd_h
#define __cmd_cmd_h

#include <list>
#include <stdio.h>

#include "cmd/cmdargs.h"

struct force_link
{
    struct force_link *f;
    class CMD *t;
};

#define __force_link(s) \
  static s __##s; \
  static struct force_link force_##s = { (struct force_link *)&force_##s, (CMD *)&__##s };

class CMD
{
 private:
  char *_name;

 public:
  CMD(const char *name);
  virtual ~CMD(void);
  
  virtual const char * get_name(void);
  virtual void register_cmd(const char *list_name,
                            CMD_Context context = 0);
  virtual void unregister_cmd(const char *list_name,
                              CMD_Context context = 0);
  virtual void execute(CMD_Args *args,
                       CMD_Context context);
};

class CMD_Repository
{
 private:
  static CMD_Repository *_singleton;
  typedef std::list<CMD_List *>cmd_llist_t;

 public:
  typedef cmd_llist_t::iterator iterator;

 private:
  cmd_llist_t *_cmd_llist;

 protected:
  CMD_Repository(void);
  virtual ~CMD_Repository(void);

 public:
  static CMD_Repository * instance(void);

  virtual void add_cmd(CMD *cmd,
                       const char *list_name,
                       CMD_Context context);
  virtual void remove_cmd(CMD *cmd,
                          const char *list_name,
                          CMD_Context context);
  virtual void execute(const char *list_name,
                       CMD_Args *args = 0);

  virtual void dump(void);
};

#define CMD_EXEC(list) \
  CMD_Repository::instance()->execute(list);
#define CMD_EXEC_ARGS(list,args) \
  CMD_Repository::instance()->execute(list,args);

#endif /* __cmd_cmd_h */
