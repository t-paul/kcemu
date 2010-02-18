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

#include "kc/system.h"

#include "kc/cb_list.h"

CallbackListEntry *CallbackListEntry::_free_list = 0;

CallbackListEntry::CallbackListEntry(unsigned long long value,
				     Callback *cb,
				     void *data,
				     CallbackListEntry *next)
{
  _cb = cb;
  _data = data;
  _next = next;
  _value = value;
}

CallbackListEntry::~CallbackListEntry(void)
{
}

void
CallbackListEntry::alloc_entries(void)
{
  for (int a = 0;a < CALLBACK_LIST_ALLOC_SIZE;a++)
    _free_list = ::new CallbackListEntry(0, 0, 0, _free_list);
}

void
CallbackListEntry::free_entries(void)
{
  for (CallbackListEntry *ptr = _free_list;ptr != NULL;)
    {
      CallbackListEntry *tmp = ptr;
      ptr = ptr->_next;
      delete tmp;
    }
  _free_list = 0;
}

void *
CallbackListEntry::operator new(size_t size)
{
  CallbackListEntry *ptr = _free_list;

  if (ptr == NULL)
    {
      alloc_entries();
      return ::new CallbackListEntry(0, 0, 0, 0);
    }

  _free_list = ptr->_next;
  ptr->_next = 0; // don't leak internal list references
  return ptr;
}

void
CallbackListEntry::operator delete(void *p)
{
  CallbackListEntry *ptr = (CallbackListEntry *)p;
  ptr->_next = _free_list;
  _free_list = ptr;
}

CallbackList::CallbackList()
{
  _list = 0;
}

CallbackList::~CallbackList(void)
{
  clear();
  CallbackListEntry::free_entries();
}

void
CallbackList::clear(void)
{
  CallbackListEntry *ptr;
  
  while (_list)
    {
      ptr = _list;
      _list = ptr->_next;
      delete ptr;
    }
}

void
CallbackList::add_callback(unsigned long long value,
			   Callback *cb,
			   void *data)
{
  CallbackListEntry *ptr, *next;

  if (_list == 0)
    {
      /*
       *  easy, empty list
       */
      _list = new CallbackListEntry(value, cb, data, 0);
    }
  else
    {
      ptr = _list;
      if (_list->_value > value)
        {
          /*
           *  add front
           */
          _list = new CallbackListEntry(value, cb, data, ptr);
        }
      else
        {
          /*
           *  insert sorted
           */
          while (242)
            {
              if (ptr->_next && ptr->_next->_value > value)
                {
                  next = ptr->_next;
                  ptr->_next = new CallbackListEntry(value, cb, data, next);
                  break;
                }
              else
                {
		  if (ptr->_next == 0)
		    {
		      ptr->_next = new CallbackListEntry(value, cb, data, 0);
		      break;
		    }
                }
              ptr = ptr->_next;
            }
        }
    }
}

void
CallbackList::run_callbacks(unsigned long long value)
{
  void *data;
  Callback *cb;
  CallbackListEntry *ptr;

  while (_list && _list->_value <= value)
    {
      ptr = _list;
      _list = ptr->_next;
      cb = ptr->_cb;
      data = ptr->_data;
      delete ptr;
      if (cb)
	cb->callback(data);
    }
}

void
CallbackList::remove_callback_listener(Callback *cb)
{
  CallbackListEntry *ptr = _list;

  for (ptr = _list;ptr != NULL;ptr = ptr->_next)
    if (ptr->_cb == cb)
      ptr->_cb = NULL;
}

/*
################################################################################
################################################################################
################################################################################
*/

#if 0
void _print_callback_list(const char *why, unsigned long long clock,
			  callback_list *list, const char *txt)
{
  int a;
  callback_list *ptr;

  a = 0;
  for (ptr = list;ptr != NULL;ptr = ptr->_next) a++;
  cerr.form("paranoia_check (%s):\n", why);
  cerr.form("\tfunction = %s\n", txt);
  cerr.form("\tpc       = %04x\n", z80->getPC());
  cerr.form("\tlength   = %10d\n", a);
  cerr.form("\tdiff     = %10Ld\n", list->_counter - clock);
  cerr.form("\tcounter  = %10Ld\n", clock);
  for (ptr = list;ptr != NULL;ptr = ptr->_next)
    {
      cerr.form("\tcounter  = %10Ld, cb = %8p [%s], data = %8p\n",
		ptr->_counter, ptr->_cb, ptr->_cb->get_name(), ptr->_data);
    }
}

void _callback_paranoia_check(unsigned long long clock, callback_list *list,
			      char *txt)
{
  unsigned long long last = 0;
  callback_list *tmp;
  callback_list *ptr = list;

  while (ptr != NULL)
    {
      if (ptr->_counter < clock)
	{
	  _print_callback_list("ptr->counter < clock", clock, list, txt);
	  return;
	}
      if (ptr->_counter < last)
        {
          _print_callback_list("ptr->counter < last", clock, list, txt);
	  return;
	}
      
      tmp = list;
      while (tmp != ptr)
        {
          if (tmp->_counter > ptr->_counter)
            {
              _print_callback_list("tmp->counter > ptr->counter",
				   clock, list, txt);
              return;
            }
          tmp = tmp->_next;
        }
      
      last = ptr->_counter;
      ptr = ptr->_next;
    }
}
#endif

