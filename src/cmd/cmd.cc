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

#include <string.h>
#include <stdlib.h>

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

/*
 *  class CMD
 */

CMD::CMD(const char *name)
{
  _name = strdup(name);
}

CMD::~CMD(void)
{
  free(_name);
}

const char *
CMD::get_name(void)
{
  return _name;
}

void
CMD::register_cmd(const char *list_name, CMD_Context context)
{
  DBG(0, form("KCemu/CMD/register",
	      "CMD: registering command '%s', context = %d\n",
	      list_name, context));
  CMD_Repository::instance()->add_cmd(this, list_name, context);
};

void
CMD::unregister_cmd(const char *list_name, CMD_Context context)
{
  DBG(0, form("KCemu/CMD/unregister",
	      "CMD: unregistering command '%s', context = %d\n",
	      list_name, context));
  CMD_Repository::instance()->remove_cmd(this, list_name, context);
};

void
CMD::execute(CMD_Args *args, CMD_Context context)
{
  DBG(0, form("KCemu/CMD/warning",
              "*** Warning: CMD without execute() function called!\n"
              "***          command is: '%s' [%p]\n"
              "***          context is: %d [0x%08x]\n",
              get_name(), this, context, context));
}

/*
 *  class CMD_Repository
 */

CMD_Repository * CMD_Repository::_singleton = 0;

CMD_Repository::CMD_Repository(void)
{
  _cmd_llist = new cmd_llist_t;
}

CMD_Repository::~CMD_Repository(void)
{
  delete _cmd_llist;
}

CMD_Repository *
CMD_Repository::instance(void)
{
  if (_singleton == 0)
    _singleton = new CMD_Repository();
  return _singleton;
}

void
CMD_Repository::add_cmd(CMD *cmd,
                        const char *list_name,
                        CMD_Context context)
{
  CMD_List *list;
  CMD_Repository::iterator it;

  list = 0;
  for (it = _cmd_llist->begin();it != _cmd_llist->end();it++)
    {
      if (strcmp((*it)->get_name(), list_name) == 0)
        {
          list = (*it);
          break;
        }
    }
  if (list == 0)
    {
      list = new CMD_List(list_name);
      _cmd_llist->push_back(list);
    }
  list->add_cmd(cmd, context);
}

void
CMD_Repository::remove_cmd(CMD *cmd,
                           const char *list_name,
                           CMD_Context context)
{
  CMD_List *list;
  CMD_Repository::iterator it;

  list = 0;
  for (it = _cmd_llist->begin();it != _cmd_llist->end();it++)
    {
      if (strcmp((*it)->get_name(), list_name) == 0)
        {
          list = (*it);
          break;
        }
    }
  if (list)
    list->remove_cmd(cmd, context);
}

void
CMD_Repository::execute(const char *list_name, CMD_Args *args)
{
  CMD_Repository::iterator it;

  for (it = _cmd_llist->begin(); it != _cmd_llist->end();it++)
    {
      if (strcmp((*it)->get_name(), list_name) == 0)
        {
          DBG(0, form("KCemu/CMD/execute",
                      "CMD: executing command '%s'\n",
                      list_name));
          (*it)->execute(args);
          return;
        }
    }
  DBG(0, form("KCemu/CMD/warning",
              "*** Warning: unknown command list called!\n"
              "***          command list name: '%s'\n",
              list_name));
}

void
CMD_Repository::dump(void)
{
  CMD_Repository::iterator it;

  DBG(0, form("KCemu/CMD_Repository",
              "*** CMD_Repository dump ***************************\n"));
  for (it = _cmd_llist->begin(); it != _cmd_llist->end();it++)
    {
      DBG(0, form("KCemu/CMD_Repository",
                  "+ List: %s\n", (*it)->get_name()));
      (*it)->dump();
    }
  DBG(0, form("KCemu/CMD_Repository",
              "*** CMD_Repository ********************************\n"));
}
