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

#ifndef __kc_kcnet_ipraw_h
#define __kc_kcnet_ipraw_h

#include "kc/kc.h"

#include "kc/kcnet/socket.h"

using namespace std;

class IPRAW : public Socket {
private:
  word_t _port;
  byte_t _ip0, _ip1, _ip2, _ip3;

  int _socket;
  SocketData *_data;
  SocketData *_send_data;

protected:
  void close_socket(void);
  word_t checksum(SocketData *data, int offset);
  
public:
  IPRAW(void);
  virtual ~IPRAW(void);

  virtual void set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3);
  virtual void set_port(word_t port);

  virtual bool open(void);
  virtual void close(void);
  virtual bool is_open(void);

  virtual void poll(void);
  virtual void send(SocketData *data);
  virtual SocketData * receive(void);
};

#endif /* __kc_kcnet_ipraw_h */
