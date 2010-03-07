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

#include <stdio.h>
#include <stdlib.h>

#include "kc/system.h"

#include "cmd/cmd.h"
#include "cmd/cmdargs.h"

#include "libdbg/dbg.h"

using namespace std;

/*
 *  class CMD_ListEntry
 */

CMD_ListEntry::CMD_ListEntry(CMD *cmd,
                             CMD_Context context,
                             const char *name)
{
  _cmd = cmd;
  _context = context;
  _name = 0;
  if (name)
    _name = strdup(name);
}

CMD_ListEntry::~CMD_ListEntry(void)
{
  if (_name)
    free(_name);
}

CMD *
CMD_ListEntry::get_cmd(void)
{
  return _cmd;
}

CMD_Context
CMD_ListEntry::get_context(void)
{
  return _context;
}

const char *
CMD_ListEntry::get_name(void)
{
  return _name;
}

/*
 *  class CMD_List
 */

CMD_List::CMD_List(const char *name)
{
  _name = strdup(name);
  _cmd_list = new cmd_list_t;
}

CMD_List::~CMD_List(void)
{
  /* FIXME: free cmds here! */
  delete _cmd_list;
  free(_name);
}

const char *
CMD_List::get_name(void)
{
  return _name;
}

void
CMD_List::add_cmd(CMD *cmd, CMD_Context context)
{
  _cmd_list->push_back(new CMD_ListEntry(cmd, context));
}

void
CMD_List::remove_cmd(CMD *cmd, CMD_Context context)
{
  /* FIXME: */
}

void
CMD_List::execute(CMD_Args *args)
{
  CMD_List::iterator it;
  
  for (it = _cmd_list->begin(); it != _cmd_list->end();it++)
    {
      (*it)->get_cmd()->execute(args, (*it)->get_context());
    }
}

void
CMD_List::dump(void)
{
  CMD_List::iterator it;
  
  for (it = _cmd_list->begin(); it != _cmd_list->end();it++)
    DBG(0, form("KCemu/CMD_Repository",
                "  - %-40s [%016p] {%d 0x%x}\n",
                (*it)->get_cmd()->get_name(),
                (*it)->get_cmd(),
                (*it)->get_context()));
}

/*
 *  class CMD_Arg
 */

CMD_Arg::CMD_Arg(const char *name, CMD_Arg_Type type)
{
  _name = strdup(name);
  _type = type;
  _value_set = false;
}

CMD_Arg::~CMD_Arg(void)
{
  if (_value_set && (_type == CMD_ARG_STRING))
    free(_val.v_string);
}

const char *
CMD_Arg::get_name(void)
{
  return _name;
}

CMD_Arg_Type
CMD_Arg::get_type(void)
{
  return _type;
}

void
CMD_Arg::set_long_arg(long value)
{
  char buf[50];

  switch (_type)
    {
    case CMD_ARG_LONG:
      _val.v_long = value;
      break;
    case CMD_ARG_STRING:
      sprintf(buf, "%ld", value);
      _val.v_string = strdup(buf);
      break;
    }
  _value_set = true;
}

void
CMD_Arg::set_string_arg(const char *value)
{
  switch (_type)
    {
    case CMD_ARG_LONG:
      _val.v_long = strtol(value, NULL, 0);
      break;
    case CMD_ARG_STRING:
      _val.v_string = strdup(value);
      break;
    }
  _value_set = true;
}

void
CMD_Arg::set_pointer_arg(void *value)
{
  switch (_type)
    {
    case CMD_ARG_LONG:
      _val.v_long = (long)value;
      break;
    case CMD_ARG_STRING:
      _val.v_string = (char *)value;
      break;
    }
  _value_set = true;
}

long
CMD_Arg::get_long_arg(void)
{
  switch (_type)
    {
    case CMD_ARG_LONG:
      return _val.v_long;
    case CMD_ARG_STRING:
      return strtol(_val.v_string, NULL, 0);
    }
  return 0;
}

const char *
CMD_Arg::get_string_arg(void)
{
  switch (_type)
    {
    case CMD_ARG_LONG:
      cerr << "CMD_Arg: accessing LONG arg as STRING" << endl;
      break;
    case CMD_ARG_STRING:
      return _val.v_string;
    }
  return "";
}

void *
CMD_Arg::get_pointer_arg(void)
{
  switch (_type)
    {
    case CMD_ARG_LONG:
      return (void *)_val.v_long;
      break;
    case CMD_ARG_STRING:
      return _val.v_string;
    }
  return 0;
}
    
/*
 *  class CMD_Args
 */

CMD_Args::CMD_Args(void)
{
  _user_data = 0;
}

CMD_Args::~CMD_Args(void)
{
  CMD_Args::arg_iterator it;

  for (it = _arg_list.begin();it != _arg_list.end();it++)
    delete (*it);
  _arg_list.erase(_arg_list.begin(), _arg_list.end());
}

CMD_Arg *
CMD_Args::lookup(const char *name)
{
  CMD_Args::arg_iterator it;

  for (it = _arg_list.begin();it != _arg_list.end();it++)
    {
      if (strcmp((*it)->get_name(), name) == 0)
        {
          return (*it);
        }
    }

  return 0;
}

CMD_Args *
CMD_Args::set_long_arg(const char *name, long value)
{
  CMD_Arg *arg;

  arg = lookup(name);
  if (arg == 0)
    {
      arg = new CMD_Arg(name, CMD_ARG_LONG);
      _arg_list.push_back(arg);
    }
  arg->set_long_arg(value);
  notify_change_listeners();
  return this;
}

CMD_Args *
CMD_Args::set_string_arg(const char *name, const char *value)
{
  CMD_Arg *arg;

  arg = lookup(name);
  if (arg == 0)
    {
      arg = new CMD_Arg(name, CMD_ARG_STRING);
      _arg_list.push_back(arg);
    }
  arg->set_string_arg(value);
  notify_change_listeners();
  return this;
}

CMD_Args *
CMD_Args::set_pointer_arg(const char *name, void *value)
{
  CMD_Arg *arg;

  arg = lookup(name);
  if (arg == 0)
    {
      arg = new CMD_Arg(name, CMD_ARG_STRING);
      _arg_list.push_back(arg);
    }
  arg->set_pointer_arg(value);
  notify_change_listeners();
  return this;
}

CMD_Args *
CMD_Args::add_change_listener(CMD_Change_Listener *listener)
{
  _cl_list.push_back(listener);
  return this;
}

long
CMD_Args::get_long_arg(const char *name)
{
  CMD_Arg *arg;

  arg = lookup(name);
  if (arg == 0)
    return 0;
  return arg->get_long_arg();
}

const char *
CMD_Args::get_string_arg(const char *name)
{
  CMD_Arg *arg;

  arg = lookup(name);
  if (arg == 0)
    return 0;
  return arg->get_string_arg();
}

void *
CMD_Args::get_pointer_arg(const char *name)
{
  CMD_Arg *arg;

  arg = lookup(name);
  if (arg == 0)
    return 0;
  return arg->get_pointer_arg();
}

bool
CMD_Args::has_arg(const char *name)
{
  if (lookup(name) == 0)
    return false;
  return true;
}

void
CMD_Args::set_user_data(void *data)
{
  _user_data = data;
}

void *
CMD_Args::get_user_data(void)
{
  return _user_data;
}


void
CMD_Args::notify_change_listeners(void)
{
  CMD_Args::cl_iterator it;

  for (it = _cl_list.begin();it != _cl_list.end();it++)
    {
      (*it)->cmd_args_changed();
    }
}

void
CMD_Args::add_callback(const char *name, CMD *cmd, CMD_Context context)
{
  DBG(2, form("KCemu/CMD_Args/callback/add",
	      "CMD_Args: adding callback '%s' for cmd '%s'\n",
	      name, cmd->get_name()));
  _cb_list.push_back(new CMD_ListEntry(cmd, context, name));
}

void
CMD_Args::remove_callback(const char *name, CMD *cmd, CMD_Context context)
{
  DBG(0, form("KCemu/warning",
	      "CMD_Args::remove_callback(): *** NOT IMPLEMENTED ***\n"));
  DBG(2, form("KCemu/CMD_Args/callback/remove",
	      "CMD_Args: removing callback '%s' for cmd '%s'\n",
	      name, cmd->get_name()));

  /* FIXME: */
}

void
CMD_Args::call_callbacks(const char *name)
{
  bool found;
  CMD *cmd;
  CMD_Args::cb_iterator it;

  found = false;
  for (it = _cb_list.begin();it != _cb_list.end();it++)
    {
      if (strcmp((*it)->get_name(), name) == 0)
        {
	  found = true;
          cmd = (*it)->get_cmd();
	  DBG(2, form("KCemu/CMD_Args/callback/call",
		      "CMD_Args: running callback '%s' on cmd '%s' [%d]\n",
		      name, cmd->get_name(), (*it)->get_context()));
          cmd->execute(this, (*it)->get_context());
        }
    }

  if (!found)
    {
      DBG(2, form("KCemu/CMD_Args/callback/call",
		  "CMD_Args: no cmd registered on callback '%s'\n",
		  name));
    }
}

void
CMD_Args::dump(const char *text)
{
  CMD_Args::arg_iterator it;

  printf("CMD_Args: -=<[%s]>=-\n", text);
  for (it = _arg_list.begin();it != _arg_list.end();it++)
    {
      switch ((*it)->get_type())
        {
        case CMD_ARG_LONG:
          printf("CMD_Args: %-30s I %ld\n", (*it)->get_name(), (*it)->get_long_arg());
          break;
        case CMD_ARG_STRING:
          printf("CMD_Args: %-30s S '%s'\n", (*it)->get_name(), (*it)->get_string_arg());
          break;
        default:
          printf("CMD_Args: %-30s ?", (*it)->get_name());
          break;
        }
    }
}
