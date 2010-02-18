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

#ifndef __kc_cb_list_h
#define __kc_cb_list_h

#include "kc/cb.h"

class CallbackListEntry
{
 private:
  enum {
    CALLBACK_LIST_ALLOC_SIZE = 1000
  };

 protected:
  /**
   *  time value of the callback entry
   */
  unsigned long long _value;

  /**
   *  the Callback that will be run
   */
  Callback *_cb;
  
  /**
   *  arbitrary data that can be set by the
   *  routine registering a callback
   */
  void *_data;

  /**
   *  single linked list
   */
  CallbackListEntry *_next;

  static CallbackListEntry *_free_list;

  static void alloc_entries(void);
  static void free_entries(void);

 public:
  CallbackListEntry(unsigned long long value,
		    Callback *cb,
		    void *data,
		    CallbackListEntry *next);
  virtual ~CallbackListEntry(void);

  void * operator new(size_t size);
  void operator delete(void *p);

  friend class CallbackList;
};

class CallbackList
{
 private:
  CallbackListEntry *_list;

 public:
  CallbackList(void);
  virtual ~CallbackList(void);

  void clear();
  void add_callback(unsigned long long offset, Callback *cb, void *data);
  void run_callbacks(unsigned long long value);
  void remove_callback_listener(Callback *cb);
};

#endif /* __kc_cb_list_h */
