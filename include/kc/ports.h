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

#ifndef __kc_ports_h
#define __kc_ports_h

#include <list>

#include "kc/system.h"

class PortInterface
{
public:
  PortInterface(void) {}
  virtual ~PortInterface() {}

  virtual byte_t in(word_t addr) = 0;
  virtual void out(word_t addr, byte_t val) = 0;
};

class NullPort : public PortInterface
{
private:
  char *_name;

public:
  NullPort(const char *name);
  virtual ~NullPort(void);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
};

class PortGroup
{
private:
  PortInterface *_p;
  int _prio;
  bool _active;
  byte_t _start;
  word_t _len;
  char *_name;
  
public:
  PortGroup(const char *name, PortInterface *p,
            byte_t start, word_t len, int prio);
  virtual ~PortGroup(void);
  virtual int get_prio(void) { return _prio; }
  virtual byte_t get_start(void) { return _start; }
  virtual word_t get_len(void) { return _len; }
  virtual void set_active(bool active) { _active = active; }
  virtual bool is_active(void) { return _active; }
  virtual const char * get_name(void) { return _name; }
  virtual PortInterface * get_port_if(void) { return _p; }
};

class Ports
{
public:
  enum {
    NR_PORTS = 256,
  };

private:
  typedef std::list<PortGroup *> port_list_t;

  port_list_t _port_list[NR_PORTS];
  PortInterface *_port_ptr[NR_PORTS];

protected:
  virtual void reload_prt_ptr(void);

public:
  Ports(void);
  virtual ~Ports(void);
  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);
    
  virtual void insert(port_list_t *l, PortGroup *group);
  virtual PortGroup * register_ports(const char *name,
                                     byte_t start, word_t len,
                                     PortInterface *p, int prio);
  virtual void unregister_ports(PortGroup *p);
  virtual void info(void);
};

#endif /* __kc_ports_h */
