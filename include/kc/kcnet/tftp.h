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

#ifndef __kc_kcnet_tftp_h
#define __kc_kcnet_tftp_h

#include <string>

#include "kc/kc.h"

#include "kc/kcnet/socket.h"

using namespace std;

class TFTP : public Socket {
private:
  // opcodes, see RFC 1350
  typedef enum {
    TFTP_RRQ   = 1,
    TFTP_WRQ   = 2,
    TFTP_DATA  = 3,
    TFTP_ACK   = 4,
    TFTP_ERROR = 5,
  } tftp_opcode_t;

  // error codes, see RFC 1350
  typedef enum {
    TFTP_ERR_UNDEFINED           = 0,
    TFTP_ERR_FILE_NOT_FOUND      = 1,
    TFTP_ERR_ACCESS_VIOLATION    = 2,
    TFTP_ERR_DISK_FULL           = 3,
    TFTP_ERR_ILLEGAL_OPERATION   = 4,
    TFTP_ERR_UNKNOWN_TRANSFER_ID = 5,
    TFTP_ERR_FILE_ALREADY_EXISTS = 6,
    TFTP_ERR_NO_SUCH_USER        = 7,
  } tftp_error_t;

private:
  int _block;
  string _path;
  SocketData *_send_data;

protected:
  virtual void init_read(SocketData *data);

public:
  TFTP(void);
  virtual ~TFTP(void);

  virtual void set_ip_address(byte_t a0, byte_t a1, byte_t a2, byte_t a3);
  virtual void set_port(word_t port);

  virtual bool open(void);
  virtual void close(void);
  virtual bool is_open(void);

  virtual void poll(void);
  virtual void send(SocketData *data);
  virtual SocketData * receive(void);
};

#endif /* __kc_kcnet_tftp_h */
