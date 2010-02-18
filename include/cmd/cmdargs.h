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

#ifndef __cmd_cmdargs_h
#define __cmd_cmdargs_h

#include <list>

class CMD_Change_Listener
{
 public:
  CMD_Change_Listener(void) {}
  virtual ~CMD_Change_Listener(void) {}

  virtual void cmd_args_changed(void) = 0;
};

typedef unsigned int CMD_Context;

typedef enum {
  CMD_ARG_LONG, CMD_ARG_STRING,
} CMD_Arg_Type;

class CMD;
class CMD_Args;

class CMD_ListEntry
{
 private:
  CMD *_cmd;
  CMD_Context _context;
  char *_name;

 public:
  CMD_ListEntry(CMD *cmd, CMD_Context context, const char *name = 0);
  virtual ~CMD_ListEntry(void);
  
  CMD * get_cmd(void);
  CMD_Context get_context(void);
  const char * get_name(void);
};

class CMD_List
{
 private:
  typedef std::list<CMD_ListEntry *>cmd_list_t;
  
 public:
  typedef cmd_list_t::iterator iterator;

 private:
  char *_name;
  cmd_list_t *_cmd_list;

 public:
  CMD_List(const char *name);
  virtual ~CMD_List(void);
  
  virtual const char * get_name();
  virtual void add_cmd(CMD *cmd, CMD_Context context);
  virtual void remove_cmd(CMD *cmd, CMD_Context context);
  virtual void execute(CMD_Args *args = 0);

  virtual void dump(void);
};

class CMD_Arg
{
 protected:
  char *_name;
  CMD_Arg_Type _type;
  union {
    long  v_long;
    char *v_string;
  } _val;
  bool _value_set;

 public:
  CMD_Arg(const char *name, CMD_Arg_Type type);
  virtual ~CMD_Arg(void);

  CMD_Arg_Type get_type(void);
  const char * get_name(void);
  virtual void set_long_arg(long value);
  virtual void set_string_arg(const char *value);
  virtual void set_pointer_arg(void *ptr);
  virtual long get_long_arg(void);
  virtual const char * get_string_arg(void);
  virtual void * get_pointer_arg(void);
};

class CMD_Args {
 private:
  typedef std::list<CMD_Arg *> arg_list_t;
  typedef std::list<CMD_Change_Listener *> cl_list_t;
  typedef std::list<CMD_ListEntry *> cb_list_t;
  
 public:
  typedef cl_list_t::iterator cl_iterator;
  typedef cb_list_t::iterator cb_iterator;
  typedef arg_list_t::iterator arg_iterator;

 private:
  void *_user_data;
  cl_list_t _cl_list;
  cb_list_t _cb_list;
  arg_list_t _arg_list;

 public:
  CMD_Args(void);
  virtual ~CMD_Args(void);

 protected:
  virtual void notify_change_listeners(void);

 public:
  CMD_Arg * lookup(const char *name);
  virtual CMD_Args * set_long_arg(const char *name, long value);
  virtual CMD_Args * set_string_arg(const char *name, const char *value);
  virtual CMD_Args * set_pointer_arg(const char *name, void *value);
  virtual CMD_Args * add_change_listener(CMD_Change_Listener *listener);
  virtual long get_long_arg(const char *name);
  virtual const char * get_string_arg(const char *name);
  virtual void * get_pointer_arg(const char *name);
  virtual bool has_arg(const char *name);
  virtual void add_callback(const char *name,
                            CMD *cmd,
                            CMD_Context context);
  virtual void remove_callback(const char *name,
                               CMD *cmd,
                               CMD_Context context);
  virtual void call_callbacks(const char *name);

  virtual void set_user_data(void *data);
  virtual void * get_user_data(void);

  virtual void dump(const char *text);
};

class CMD_Caller
{
 private:
  CMD_Args *_args;
  
 public:
  CMD_Caller(CMD_Args *args = 0) { _args = args; }
  virtual ~CMD_Caller(void) {}
  
  virtual CMD_Args * cmd_caller_get_args(void) { return _args; }
  virtual void cmd_caller_set_args(CMD_Args *args) { _args = args; }
};

#endif /* __cmd_cmdargs_h */
