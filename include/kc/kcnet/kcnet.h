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

#ifndef __kc_kcnet_kcnet_h
#define __kc_kcnet_kcnet_h

#include <string>

#include "kc/kc.h"
#include "kc/cb.h"
#include "kc/pio.h"

#include "kc/kcnet/socket.h"

using namespace std;

typedef enum
{
  SOCK_CLOSED      = 0x00,
  SOCK_INIT        = 0x13,
  SOCK_LISTEN      = 0x14,
  SOCK_ESTABLISHED = 0x17,
  SOCK_CLOSE_WAIT  = 0x1c,
  SOCK_UDP         = 0x22,
  SOCK_IPRAW       = 0x32,
  SOCK_MACRAW      = 0x42,
  SOCK_PPPOE       = 0x5f,

  SOCK_SYNSENT     = 0x15,
  SOCK_SYNRECV     = 0x16,
  SOCK_FIN_WAIT    = 0x18,
  SOCK_CLOSING     = 0x1a,
  SOCK_TIME_WAIT   = 0x1b,
  SOCK_LAST_ACK    = 0x1d,
  SOCK_ARP1        = 0x11,
  SOCK_ARP2        = 0x21,
  SOCK_ARP3        = 0x31,
} socket_status_t;

typedef enum
{
  CMD_WRITE_BYTES = 0,
  CMD_READ_BYTES  = 1,
  CMD_SET_POINTER = 2,
  CMD_READ_TIMER  = 3,
  CMD_WRITE_BYTE  = 4,
  CMD_READ_BYTE   = 5,
  CMD_WRITE_IPADDR = 6,
  CMD_READ_IPADDR  = 7,
  CMD_READ_PORTNO  = 8,
  CMD_READ_SW_VERSION = 9,
  CMD_READ_HW_VERSION = 10,
  CMD_READ_LINK       = 11,
  CMD_READ_ID         = 12,
  CMD_READ_ERROR      = 13,
} kcnet_command_t;

typedef enum
{
  IR_SEND_OK = 0x10,
  IR_TIMEOUT = 0x08,
  IR_RECV    = 0x04,
  IR_DISCON  = 0x02,
  IR_CON     = 0x01,
} ir_reg_t;

class KCNET;

class KCNET_CMD {
public:
  typedef enum {
    KCNET_CMD_PHASE_ARGS       = 1,
    KCNET_CMD_PHASE_EXECUTE    = 2,
    KCNET_CMD_PHASE_WRITE_DATA = 3,
    KCNET_CMD_PHASE_READ_DATA  = 4,
    KCNET_CMD_PHASE_FINISHED   = 5,
  } kcnet_cmd_phase_t;

private:
  int _idx;
  int _args;
  char *_name;
  byte_t *_buf;
  int _result_len;
  int _result_idx;
  byte_t *_result_buf;
  kcnet_cmd_phase_t _phase;

protected:
  KCNET *_kcnet;
  virtual void execute(void) = 0;

  const char * get_name(void);

  kcnet_cmd_phase_t get_phase(void);
  void set_phase(kcnet_cmd_phase_t phase);

public:
  KCNET_CMD(KCNET *kcnet, const char *name, int args);
  virtual ~KCNET_CMD(void);

  virtual void init(void);

  virtual bool is_active(void);

  virtual byte_t read(void);
  virtual void write(byte_t val);
  virtual byte_t get_data(int idx);

  virtual byte_t data_read(void);
  virtual void data_write(byte_t val);

  virtual void set_result(byte_t val);
  virtual void set_result(byte_t val1, byte_t val2);
  virtual void set_result(byte_t *buf, int len);

  static KCNET_CMD * get_command(KCNET *kcnet, kcnet_command_t command);
};

class KCNET_CMD_WRITE_BYTES : public KCNET_CMD {
private:
  int _idx;
  word_t _counter;
protected:
  void execute(void);
public:
  KCNET_CMD_WRITE_BYTES(KCNET *kcnet);

  void data_write(byte_t val);
};

class KCNET_CMD_READ_BYTES : public KCNET_CMD {
private:
  int _idx;
  word_t _counter;
protected:
  void execute(void);
public:
  KCNET_CMD_READ_BYTES(KCNET *kcnet);

  byte_t data_read(void);
};

class KCNET_CMD_SET_POINTER : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_SET_POINTER(KCNET *kcnet);
};

class KCNET_CMD_READ_TIMER : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_TIMER(KCNET *kcnet);
};

class KCNET_CMD_WRITE_BYTE : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_WRITE_BYTE(KCNET *kcnet);
};

class KCNET_CMD_READ_BYTE : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_BYTE(KCNET *kcnet);
};

class KCNET_CMD_WRITE_IPADDR : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_WRITE_IPADDR(KCNET *kcnet);
};

class KCNET_CMD_READ_IPADDR : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_IPADDR(KCNET *kcnet);
};

class KCNET_CMD_READ_PORTNO : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_PORTNO(KCNET *kcnet);
};

class KCNET_CMD_READ_SW_VERSION : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_SW_VERSION(KCNET *kcnet);
};

class KCNET_CMD_READ_HW_VERSION : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_HW_VERSION(KCNET *kcnet);
};

class KCNET_CMD_READ_LINK : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_LINK(KCNET *kcnet);
};

class KCNET_CMD_READ_ID : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_ID(KCNET *kcnet);
};

class KCNET_CMD_READ_ERROR : public KCNET_CMD {
protected:
  void execute(void);
public:
  KCNET_CMD_READ_ERROR(KCNET *kcnet);
};

class KCNET : public InterfaceCircuit, public Callback, public PIOCallbackInterface
{
public:
  enum {
    KCNET_SW_VERSION_MAJOR = 1,
    KCNET_SW_VERSION_MINOR = 2,

    KCNET_HW_VERSION_MAJOR = 1,
    KCNET_HW_VERSION_MINOR = 2,

    NR_OF_SOCKETS = 4,
  };

  enum {
    ADDR_MR     = 0x0000,
    ADDR_GAR0   = 0x0001,
    ADDR_GAR1   = 0x0002,
    ADDR_GAR2   = 0x0003,
    ADDR_GAR3   = 0x0004,
    ADDR_SUBR0  = 0x0005,
    ADDR_SUBR1  = 0x0006,
    ADDR_SUBR2  = 0x0007,
    ADDR_SUBR3  = 0x0008,
    ADDR_SHAR0  = 0x0009,
    ADDR_SHAR1  = 0x000a,
    ADDR_SHAR2  = 0x000b,
    ADDR_SHAR3  = 0x000c,
    ADDR_SHAR4  = 0x000d,
    ADDR_SHAR5  = 0x000e,
    ADDR_SIPR0  = 0x000f,
    ADDR_SIPR1  = 0x0010,
    ADDR_SIPR2  = 0x0011,
    ADDR_SIPR3  = 0x0012,
    ADDR_IR     = 0x0015,
    ADDR_IMR    = 0x0016,
    ADDR_RTR0   = 0x0017,
    ADDR_RTR1   = 0x0018,
    ADDR_RCR    = 0x0019,
    ADDR_RMSR   = 0x001a,
    ADDR_TMSR   = 0x001b,
    ADDR_PATR0  = 0x001c,
    ADDR_PATR1  = 0x001d,
    ADDR_PTIMER = 0x0028,
    ADDR_PMAGIC = 0x0029,
    ADDR_UIPR0  = 0x002a,
    ADDR_UIPR1  = 0x002b,
    ADDR_UIPR2  = 0x002c,
    ADDR_UIPR3  = 0x002d,
    ADDR_UPORT0 = 0x002e,
    ADDR_UPORT1 = 0x002f,
  };

private:
  PIO *_pio;
  byte_t _mem[0x8000];
  word_t _mem_ptr;
  byte_t _status;
  int _portno;
  KCNET_CMD *_command;
  dword_t _ip_addr[8];
  Socket *_socket[4];

  word_t _recv_ptr[4];
  word_t _old_read_ptr[4];
  word_t _old_write_ptr[4];

protected:
  const bool get_ip_address_from_prefs(const char *key, byte_t &ip0, byte_t &ip1, byte_t &ip2, byte_t &ip3) const;

public:
  KCNET(void);
  virtual ~KCNET(void);

  virtual void init(bool power_on, bool set_config);

  virtual byte_t get_status(void);
  virtual void set_status(byte_t status);

  virtual byte_t read(void);
  virtual void write(byte_t byte);

  virtual word_t get_pointer(void);
  virtual void set_pointer(word_t ptr);

  virtual dword_t get_ip_address(unsigned int idx);
  virtual void set_ip_address(unsigned int idx, dword_t ip_addr);
  
  virtual byte_t mem_read(void);
  virtual byte_t mem_read(word_t addr);

  virtual void mem_write(byte_t val);
  virtual void mem_write(word_t addr, byte_t val);

  virtual const char * mem_info(void);
  virtual const char * mem_info(word_t addr);

  virtual void callback(void *data);

  virtual void register_pio(PIO *pio);

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void);
  virtual word_t irqack(void);
  virtual void reset(bool power_on = false);

  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

#endif /* __kc_kcnet_kcnet_h */
