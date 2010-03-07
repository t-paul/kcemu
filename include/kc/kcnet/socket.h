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

#ifndef __kc_kcnet_socket_h
#define __kc_kcnet_socket_h

#include "kc/kc.h"

using namespace std;

class SocketData {
private:
  int _len;
  int _idx;
  byte_t *_buf;

public:
  SocketData(int len);
  virtual ~SocketData(void);

  int length(void);

  void put_byte(byte_t val);
  void put_word(word_t val);
  void put_long(dword_t val);
  void put_text(const char *text);

  byte_t * get(void);
  byte_t get(int idx);

  void put(int idx, byte_t val);
};

class Socket {
public:
  virtual void set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3) = 0;
  virtual void set_port(word_t port) = 0;

  virtual bool open(void) = 0;
  virtual void close(void) = 0;
  virtual bool is_open(void) = 0;

  virtual void poll(void) = 0;
  virtual void send(SocketData *data) = 0;
  virtual SocketData * receive(void) = 0;
};

#endif /* __kc_kcnet_socket_h */
