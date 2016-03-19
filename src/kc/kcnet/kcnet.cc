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

#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/time.h>

#include "kc/system.h"
#include "kc/prefs/prefs.h"
#include "kc/prefs/strlist.h"

#include "kc/kc.h"
#include "kc/pio.h"
#include "kc/z80.h"

#include "kc/kcnet/kcnet.h"
#include "kc/kcnet/tcp.h"
#include "kc/kcnet/udp.h"
#include "kc/kcnet/tftp.h"
#include "kc/kcnet/ipraw.h"

#include "cmd/cmd.h"

#include "libdbg/dbg.h"

KCNET::KCNET(void) : InterfaceCircuit("KC-NET"), Callback("KC-NET")
{
  _pio = NULL;
  _command = NULL;
  for (int a = 0;a < NR_OF_SOCKETS;a++)
    _socket[a] = NULL;

  for (int a = 0;a < 8;a++)
    _ip_addr[a] = 0;
  
  z80->register_ic(this);
  init(true, true);
}

KCNET::~KCNET(void)
{
  z80->unregister_ic(this);
}

void
KCNET::reti(void)
{
}

void
KCNET::irqreq(void)
{
}

word_t
KCNET::irqack(void)
{
  return IRQ_NOT_ACK;
}

void
KCNET::reset(bool power_on)
{
  init(power_on, true);
}

void
KCNET::init(bool power_on, bool set_config)
{
  _status = 0;
  _mem_ptr = 0;
  _portno = 3000;

  if (_command != NULL)
    delete _command;
  _command = NULL;

  for (int a = 0;a < NR_OF_SOCKETS;a++)
    {
      if (_socket[a] != NULL)
        {
          _socket[a]->close();
          delete _socket[a];
        }
      _socket[a] = NULL;
    }

  z80->addCallback(30000, this, NULL);
  
  if (power_on)
    {
      memset(_mem, 0, sizeof (_mem));

      // mac addr
      _mem[ADDR_SHAR0] = 0x00;
      _mem[ADDR_SHAR1] = 0x16;
      _mem[ADDR_SHAR2] = 0xaf;
      _mem[ADDR_SHAR3] = 0xbc;
      _mem[ADDR_SHAR4] = 0xde;
      _mem[ADDR_SHAR5] = 0x12;

      // just in case... see include/linux/etherdevice.h
      _mem[ADDR_SHAR5] &= 0xfe; /* clear multicast bit */
      _mem[ADDR_SHAR5] |= 0x02; /* set local assignment bit (IEEE802) */

      // default values
      _mem[ADDR_RTR0]   = 0x07;
      _mem[ADDR_RTR1]   = 0xd0;
      _mem[ADDR_RCR]    = 0x08;
      _mem[ADDR_RMSR]   = 0x55;
      _mem[ADDR_TMSR]   = 0x55;
      _mem[ADDR_PTIMER] = 0x28;
    }

  if (set_config)
    {
      byte_t ip0, ip1, ip2, ip3;


      if (get_ip_address_from_prefs("network_gateway", ip0, ip1, ip2, ip3))
        {
          _mem[ADDR_GAR0] = ip0;
          _mem[ADDR_GAR1] = ip1;
          _mem[ADDR_GAR2] = ip2;
          _mem[ADDR_GAR3] = ip3;
        }

      if (get_ip_address_from_prefs("network_netmask", ip0, ip1, ip2, ip3))
        {
          _mem[ADDR_SUBR0] = ip0;
          _mem[ADDR_SUBR1] = ip1;
          _mem[ADDR_SUBR2] = ip2;
          _mem[ADDR_SUBR3] = ip3;
        }

      if (get_ip_address_from_prefs("network_ip_address", ip0, ip1, ip2, ip3))
        {
          _mem[ADDR_SIPR0] = ip0;
          _mem[ADDR_SIPR1] = ip1;
          _mem[ADDR_SIPR2] = ip2;
          _mem[ADDR_SIPR3] = ip3;
        }

      if (get_ip_address_from_prefs("network_dns_server", ip0, ip1, ip2, ip3))
        {
          set_ip_address(0, (ip0 << 24) | (ip1 << 16) | (ip2 << 8) | ip3);
        }
    }
}

const bool
KCNET::get_ip_address_from_prefs(const char *key, byte_t &ip0, byte_t &ip1, byte_t &ip2, byte_t &ip3) const
{
  ip0 = ip1 = ip2 =  ip3 = 0;
  
  const char *addr = Preferences::instance()->get_string_value(key, NULL);
  if (addr == NULL)
    return false;

  StringList list(addr, '.');
  if (list.size() != 4)
    return false;

  int ip[4];
  int idx = 0;
  for (StringList::iterator it = list.begin();it != list.end();it++)
    {
      char *endptr;
      unsigned long n = strtoul((*it).c_str(), &endptr, 10);
      if (*endptr != 0)
        return false;

      if (n > 255)
        return false;

      ip[idx++] = n;
    }

  ip0 = ip[0];
  ip1 = ip[1];
  ip2 = ip[2];
  ip3 = ip[3];
  
  return true;
}

void
KCNET::register_pio(PIO *pio)
{
  _pio = pio;
}

void
KCNET::callback(void *data)
{
  z80->addCallback(30000, this, NULL);

  for (int a = 0;a < 4;a++)

  for (int idx = 0;idx < 4;idx++)
    {
      if (_socket[idx] == NULL)
        continue;

      word_t s = (idx << 8) + 0x0400;

      if (_socket[idx]->is_open())
        {
          _mem[s | 0x02] |= IR_CON;
          if (_mem[s | 0x03] == SOCK_INIT)
            {
              _mem[s | 0x03] = SOCK_ESTABLISHED;
              printf("TCP! ESTABLISHED\n");
              _mem[s | 0x20] = 4; // TX Free Size
              _mem[s | 0x21] = 0;
            }
        }
      else
        {
          continue;
        }

      _socket[idx]->poll();

      if (_mem[s | 0x26] | _mem[s | 0x27])
        {
          return;
        }

      SocketData *recv = _socket[idx]->receive();
      if (recv != NULL)
        {
          _mem[s | 0x02] |= IR_RECV;

          int data_len = recv->length();
          for (int a = 0;a < recv->length();a++)
            {
              word_t base = 0x6000 + idx * 0x0800;
              _mem[_recv_ptr[idx]++] = recv->get(a);
              if (_recv_ptr[idx] == base + 0x0800)
                _recv_ptr[idx] = base;
            }

          _mem[s | 0x26] = data_len >> 8;
          _mem[s | 0x27] = data_len;

          delete recv;
        }
    }
}

byte_t
KCNET::get_status(void)
{
  return _status;
}

void
KCNET::set_status(byte_t status)
{
  _status = status;
}

word_t
KCNET::get_pointer(void)
{
  return _mem_ptr;
}

void
KCNET::set_pointer(word_t ptr)
{
  _mem_ptr = ptr;
}

dword_t
KCNET::get_ip_address(unsigned int idx)
{
  if (idx < 8)
    return _ip_addr[idx];

  return 0;
}

void
KCNET::set_ip_address(unsigned int idx, dword_t ip_addr)
{
  if (idx < 8)
    _ip_addr[idx] = ip_addr;
}

byte_t
KCNET::mem_read(void)
{
  return mem_read(_mem_ptr++);
}

byte_t
KCNET::mem_read(word_t addr)
{
  return _mem[addr & 0x7fff];
}

void
KCNET::mem_write(byte_t val)
{
  mem_write(_mem_ptr++, val);
}

void
KCNET::mem_write(word_t addr, byte_t val)
{
  addr &= 0x7fff;

  _mem[addr] = val;

  if ((addr == 0) && (val & 0x80))
    {
      init(true, false);
      return;
    }

  if ((addr < 0x0400) || (addr >= 0x0800))
    return;

  // socket access
  int s = addr & 0xff00;
  int idx = (s - 0x400) >> 8;
  addr &= 0xff;

  if (addr == 0x01) // socket command
    {
      if (val == 0x01) // open
        {
          if ((_mem[s] & 0x8f) == 0x01) // ignore delayed ack (bit 5)
            _mem[s | 0x03] = SOCK_INIT;
          else if (_mem[s] == 0x02)
            _mem[s | 0x03] = SOCK_UDP;
          else if (_mem[s] == 0x03)
            _mem[s | 0x03] = SOCK_IPRAW;
          
          _mem[s | 0x01] = 0;

          _mem[s | 0x28] = 0x00;
          _mem[s | 0x29] = 0x00;

        }
      if (val == 0x10) // close
        {
          if (_socket[idx] != NULL)
            {
              _socket[idx]->close();
              delete _socket[idx];
              _mem[s | 0x24] = 0; // write pointer
              _mem[s | 0x25] = 0;
            }
          _socket[idx] = NULL;

          _mem[s | 0x01] = 0;
          _mem[s | 0x02] = 0;
          _mem[s | 0x03] = 0;
        }
      if (val == 0x04) // connect
        {
          if (_socket[idx] == NULL)
            {
              _recv_ptr[idx] = 0x6000 + idx * 0x0800;
              _old_read_ptr[idx] = 0x0000;
              _old_write_ptr[idx] = 0x0000;
              if (_mem[s | 0x03] == SOCK_INIT)
                {
                  word_t port = _mem[s | 0x10] << 8 | _mem[s | 0x11];
                  printf("TCP! connect\n");
                  _socket[idx] = new TCP();
                  _socket[idx]->set_ip_address(_mem[s | 0x0c], _mem[s | 0x0d], _mem[s | 0x0e], _mem[s | 0x0f]);
                  _socket[idx]->set_port(port);
                  _socket[idx]->open();
                }
            }
        }
      if (val == 0x20) // send
        {
          if (_socket[idx] == NULL)
            {
              _recv_ptr[idx] = 0x6000 + idx * 0x0800;
              _old_read_ptr[idx] = 0x0000;
              _old_write_ptr[idx] = 0x0000;
            }

          word_t port = _mem[s | 0x10] << 8 | _mem[s | 0x11];
          word_t write_ptr = _mem[s | 0x24] << 8 | _mem[s | 0x25];
          int len = write_ptr - _old_write_ptr[idx];

          if (_socket[idx] == NULL)
            {
              if (_mem[s | 0x03] == SOCK_UDP)
                {
                  printf("UDP! %04x / %04x (%d, %04xh)\n", _old_write_ptr[idx], write_ptr, len, len);
                  _socket[idx] = new UDP();
                  _socket[idx]->set_ip_address(_mem[s | 0x0c], _mem[s | 0x0d], _mem[s | 0x0e], _mem[s | 0x0f]);
                  _socket[idx]->set_port(port);
                  _socket[idx]->open();
                }
              if (_mem[s | 0x03] == SOCK_IPRAW)
                {
                  printf("IPRAW! %04x / %04x (%d, %04xh)\n", _old_write_ptr[idx], write_ptr, len, len);
                  _socket[idx] = new IPRAW();
                  _socket[idx]->set_ip_address(_mem[s | 0x0c], _mem[s | 0x0d], _mem[s | 0x0e], _mem[s | 0x0f]);
                  _socket[idx]->set_port(port);
                  _socket[idx]->open();
                }
            }
          else
            {
              _socket[idx]->set_ip_address(_mem[s | 0x0c], _mem[s | 0x0d], _mem[s | 0x0e], _mem[s | 0x0f]);
              _socket[idx]->set_port(port);
            }

          if (_socket[idx] != NULL)
            {
              SocketData *data = new SocketData(len);
              word_t base = 0x4000 + idx * 0x0800;
              for (int a = 0;a < len;a++)
                data->put_byte(_mem[base + _old_write_ptr[idx] + a]);
              _socket[idx]->send(data);
              delete data;
            }

          _old_write_ptr[idx] = write_ptr;
          _mem[s | 0x02] = IR_SEND_OK | IR_CON;

          _mem[s | 0x01] = 0;
        }
      if (val == 0x40) // recv
        {
          word_t read_ptr = _mem[s | 0x22] << 8 | _mem[s | 0x23];
          int len = read_ptr - _old_read_ptr[idx];
          printf("RECV! %04x / %04x (%d, %04xh)\n", _old_read_ptr[idx], read_ptr, len, len);

          _mem[s | 0x02] = IR_CON;
          _mem[s | 0x01] = 0;
          _mem[s | 0x26] = 0x00;
          _mem[s | 0x27] = 0x00;

          _old_read_ptr[idx] = read_ptr;
        }
    }
}

const char *
KCNET::mem_info(void)
{
  return mem_info(_mem_ptr);
}

const char *
KCNET::mem_info(word_t addr)
{
  addr &= 0x7fff;
  
  if ((addr >= 0x0400) && (addr < 0x0800))
    {
      switch (addr & 0x00ff)
        {
        case 0x0000: return "Socket x Mode (Sx_MR)";
        case 0x0001: return "Socket x Command (Sx_CR)";
        case 0x0002: return "Socket x Interrupt (Sx_IR)";
        case 0x0003: return "Socket x Status (Sx_SR)";
        case 0x0004: return "Socket x Source Port";
        case 0x0006: return "Socket x Destination Hardware Address";
        case 0x000c: return "Socket x Destination IP Address";
        case 0x0010: return "Socket x Destination Port";
        case 0x0012: return "Socket x Maximum Segment Size";
        case 0x0014: return "Socket x Protocol in IP Raw mode";
        case 0x0015: return "Socket x IP TOS (Sx_TOS)";
        case 0x0016: return "Socket x IP TTL (Sx_TTL)";
        case 0x0020: return "Socket x TX Free Size";
        case 0x0022: return "Socket x TX Read Pointer";
        case 0x0024: return "Socket x TX Write Pointer";
        case 0x0026: return "Socket x RX Received Size";
        case 0x0028: return "Socket x RX Read Pointer";
        }
    }
  
  switch (addr)
    {
    case 0x0000: return "Mode (MR)";
    case 0x0001: return "Gateway Address";
    case 0x0005: return "Subnet mask Address";
    case 0x0009: return "Source Hardware Address";
    case 0x000f: return "Source IP Address";
    case 0x0015: return "Interrupt (IR)";
    case 0x0016: return "Interrupt Mask (IMR)";
    case 0x0017: return "Retry Time";
    case 0x0019: return "Retry Count (RCR)";
    case 0x001a: return "RX Memory Size (RMSR)";
    case 0x001b: return "TX Memory Size (TMSR)";
    case 0x001c: return "Authentication Type in PPPoE";
    case 0x0028: return "PPP LCP Request Timer (PTIMER)";
    case 0x0029: return "PPP LCP Magic number (PMAGIC)";
    case 0x002a: return "Unreachable IP Address";
    case 0x002e: return "Unreachable Port";
    default:
      return "";
    }
}

byte_t
KCNET::read(void)
{
  if (_command)
    return _command->read();

  printf("KCNET::read(): NO ACTIVE COMMAND\n");
  return 0x00;
}

void
KCNET::write(byte_t val)
{
  if (_command && _command->is_active())
    {
      //printf("WRITE %02x\n", val);
      _command->write(val);
    }
  else
    {
      //printf("NEW COMMAND: %02x\n", val);
      _command = KCNET_CMD::get_command(this, (kcnet_command_t)val);
      if (_command)
        _command->init();
    }
}

int
KCNET::callback_A_in(void)
{
  return -1;
}

int
KCNET::callback_B_in(void)
{
  return -1;
}

void
KCNET::callback_A_out(byte_t val)
{
}

void
KCNET::callback_B_out(byte_t val)
{
}

KCNET_CMD::KCNET_CMD(KCNET *kcnet, const char *name, int args)
{
  _idx = 0;
  _args = args;
  _name = strdup(name);
  _kcnet = kcnet;
  
  if (_args > 0)
    {
      _buf = new byte_t[args];
      set_phase(KCNET_CMD_PHASE_ARGS);
    }
  else
    {
      _buf = NULL;
      set_phase(KCNET_CMD_PHASE_EXECUTE);
    }

  _result_len = 0;
  _result_buf = NULL;
}

KCNET_CMD::~KCNET_CMD(void)
{
  free(_name);

  if (_buf != NULL)
    delete[] _buf;

  if (_result_buf != NULL)
    delete[] _result_buf;
}

void
KCNET_CMD::init(void)
{
  if (get_phase() == KCNET_CMD_PHASE_EXECUTE)
    execute();
}

const char *
KCNET_CMD::get_name()
{
  return _name;
}

KCNET_CMD::kcnet_cmd_phase_t
KCNET_CMD::get_phase(void)
{
  return _phase;
}

void
KCNET_CMD::set_phase(kcnet_cmd_phase_t phase)
{
  //printf("KCNET_CMD::set_phase(): %d\n", phase);
  _phase = phase;
  if (phase == KCNET_CMD_PHASE_READ_DATA)
    _kcnet->set_status(0x80);
}

bool
KCNET_CMD::is_active(void)
{
  return get_phase() != KCNET_CMD_PHASE_FINISHED;
}

byte_t
KCNET_CMD::read(void)
{
  return data_read();
}

byte_t
KCNET_CMD::data_read(void)
{
  if (_result_idx < _result_len)
    {
      byte_t val = _result_buf[_result_idx++];
      if (_result_idx == _result_len)
        set_phase(KCNET_CMD_PHASE_FINISHED);
      DBG(2, form("KCemu/KCNET_CMD/result",
                  "KCNET_CMD::data_read(): %02xh (%3d / '%c')\n",
                  val, val, isprint(val) ? val : '.'));
      return val;
    }

  DBG(1, form("KCemu/KCNET_CMD/result",
              "KCNET_CMD::data_read(): ILLEGAL READ\n"));
  return 0x00;
}

void
KCNET_CMD::write(byte_t val)
{
  if (get_phase() == KCNET_CMD_PHASE_WRITE_DATA)
    {
      data_write(val);
    }
  else
    {
      if (_idx < _args)
        {
          _buf[_idx++] = val;
        }
      else
        {
          printf("IGNORE WRITE! (cmd = %s, phase = %d, val = %02xh (%3d)\n", get_name(), get_phase(), val, val);
          return;
        }

      if (_idx == _args)
        {
          set_phase(KCNET_CMD_PHASE_EXECUTE);
          execute();
        }
    }
}

void
KCNET_CMD::data_write(byte_t val)
{
}

byte_t
KCNET_CMD::get_data(int idx)
{
  if (idx >= _args)
    {
      printf("ILLEGAL ARGUMENT READ, return 0!\n");
      return 0;
    }
  return _buf[idx];
}

void
KCNET_CMD::set_result(byte_t val)
{
  byte_t *buf = new byte_t[1];
  buf[0] = val;
  set_result(buf, 1);
}

void
KCNET_CMD::set_result(byte_t val1, byte_t val2)
{
  byte_t *buf = new byte_t[2];
  buf[0] = val1;
  buf[1] = val2;
  set_result(buf, 2);
}

void
KCNET_CMD::set_result(byte_t *buf, int len)
{
  _result_idx = 0;
  _result_buf = buf;
  _result_len = len;
  set_phase(KCNET_CMD_PHASE_READ_DATA);
}

KCNET_CMD *
KCNET_CMD::get_command(KCNET *kcnet, kcnet_command_t command)
{
  switch (command)
    {
    case CMD_WRITE_BYTES:
      return new KCNET_CMD_WRITE_BYTES(kcnet);
    case CMD_READ_BYTES:
      return new KCNET_CMD_READ_BYTES(kcnet);
    case CMD_SET_POINTER:
      return new KCNET_CMD_SET_POINTER(kcnet);
    case CMD_READ_TIMER:
      return new KCNET_CMD_READ_TIMER(kcnet);
    case CMD_WRITE_BYTE:
      return new KCNET_CMD_WRITE_BYTE(kcnet);
    case CMD_READ_BYTE:
      return new KCNET_CMD_READ_BYTE(kcnet);
    case CMD_WRITE_IPADDR:
      return new KCNET_CMD_WRITE_IPADDR(kcnet);
    case CMD_READ_IPADDR:
      return new KCNET_CMD_READ_IPADDR(kcnet);
    case CMD_READ_PORTNO:
      return new KCNET_CMD_READ_PORTNO(kcnet);
    case CMD_READ_SW_VERSION:
      return new KCNET_CMD_READ_SW_VERSION(kcnet);
    case CMD_READ_HW_VERSION:
      return new KCNET_CMD_READ_HW_VERSION(kcnet);
    case CMD_READ_LINK:
      return new KCNET_CMD_READ_LINK(kcnet);
    case CMD_READ_ID:
      return new KCNET_CMD_READ_ID(kcnet);
    case CMD_READ_ERROR:
      return new KCNET_CMD_READ_ERROR(kcnet);
    default:
      return NULL;
    }
}

KCNET_CMD_WRITE_BYTES::KCNET_CMD_WRITE_BYTES(KCNET *kcnet) : KCNET_CMD(kcnet, "WRITE_BYTES", 2)
{
}

void
KCNET_CMD_WRITE_BYTES::execute(void)
{
  _idx = 0;
  _counter = get_data(0) | (get_data(1) << 8);
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_WRITE_BYTES",
              "KCNET_CMD_WRITE_BYTES:     addr: %04xh - writing %3d (%04xh) bytes            | %s\n",
              _kcnet->get_pointer(), _counter, _counter, _kcnet->mem_info()));
  set_phase(KCNET_CMD_PHASE_WRITE_DATA);
}

void
KCNET_CMD_WRITE_BYTES::data_write(byte_t val)
{
  if (_idx < _counter)
    {
      DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_WRITE_BYTES",
                  "KCNET_CMD_WRITE_BYTES:     addr: %04xh - %02xh (%3d / '%c') - index = %02xh (%3d)  | %s\n",
                  _kcnet->get_pointer(), val, val, isprint(val) ? val : '.', _idx, _idx, _kcnet->mem_info()));
      _kcnet->mem_write(val);
      _idx++;

      if (_idx == _counter)
        set_phase(KCNET_CMD_PHASE_FINISHED);

      return;
    }

  DBG(0, form("KCemu/KCNET_CMD/KCNET_CMD_WRITE_BYTES",
              "KCNET_CMD_WRITE_BYTES:     ILLEGAL WRITE\n"));
}

KCNET_CMD_READ_BYTES::KCNET_CMD_READ_BYTES(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_BYTES", 2)
{
}

void
KCNET_CMD_READ_BYTES::execute(void)
{
  _idx = 0;
  _counter = get_data(0) | (get_data(1) << 8);
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_BYTES",
              "KCNET_CMD_READ_BYTES:      addr: %04xh - reading %3d (%04xh) bytes            | %s\n",
              _kcnet->get_pointer(), _counter, _counter, _kcnet->mem_info()));
  set_phase(KCNET_CMD_PHASE_READ_DATA);
}

byte_t
KCNET_CMD_READ_BYTES::data_read(void)
{
  if (_idx < _counter)
    {
      byte_t val = _kcnet->mem_read();

      DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_BYTES",
                  "KCNET_CMD_READ_BYTES:      addr: %04xh - %02xh (%3d / '%c') - index = %02xh (%3d)  | %s\n",
                  _kcnet->get_pointer(),
                  val, val, isprint(val) ? val : '.',
                  _idx, _idx,
                  _kcnet->mem_info(_kcnet->get_pointer())));

      _idx++;

      if (_idx == _counter)
        set_phase(KCNET_CMD_PHASE_FINISHED);

      return val;
    }

  DBG(0, form("KCemu/KCNET_CMD/KCNET_CMD_READ_BYTES",
              "KCNET_CMD_READ_BYTES:      ILLEGAL WRITE\n"));
  return 0x00;
}

KCNET_CMD_SET_POINTER::KCNET_CMD_SET_POINTER(KCNET *kcnet) : KCNET_CMD(kcnet, "SET_POINTER", 2)
{
}

void
KCNET_CMD_SET_POINTER::execute(void)
{
  word_t ptr = get_data(0) | (get_data(1) << 8);
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_SET_POINTER",
              "KCNET_CMD_SET_POINTER:     addr: %04xh                                        | %s \n",
              ptr, _kcnet->mem_info(ptr)));
  _kcnet->set_pointer(ptr);
  set_phase(KCNET_CMD_PHASE_FINISHED);
}

KCNET_CMD_READ_TIMER::KCNET_CMD_READ_TIMER(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_TIMER", 0)
{
}

void
KCNET_CMD_READ_TIMER::execute(void)
{
  static word_t val = 0;
  
  struct timeval tv;
  if (gettimeofday(&tv, NULL) == 0)
    {
      val = (tv.tv_sec % 60) * 1000;
      val += tv.tv_usec / 1000;
    }
  else
    {
      val++;
    }

  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_TIMER",
              "KCNET_CMD_READ_TIMER:      val:  %04xh\n",
              val));
  
  set_result(val & 0xff, val >> 8);
}

KCNET_CMD_WRITE_BYTE::KCNET_CMD_WRITE_BYTE(KCNET *kcnet) : KCNET_CMD(kcnet, "WRITE_BYTE", 3)
{
}

void
KCNET_CMD_WRITE_BYTE::execute(void)
{
  word_t ptr = get_data(0) | (get_data(1) << 8);
  byte_t val = get_data(2);
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_WRITE_BYTE",
              "KCNET_CMD_WRITE_BYTE:      addr: %04xh - %02xh (%3d / '%c')                      | %s\n",
              ptr, val, val, isprint(val) ? val : '.', _kcnet->mem_info(ptr)));
  _kcnet->mem_write(ptr, val);
  set_phase(KCNET_CMD_PHASE_FINISHED);
}

KCNET_CMD_READ_BYTE::KCNET_CMD_READ_BYTE(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_BYTE", 2)
{
}

void
KCNET_CMD_READ_BYTE::execute(void)
{
  word_t ptr = get_data(0) | (get_data(1) << 8);
  byte_t val = _kcnet->mem_read(ptr);
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_BYTE",
              "KCNET_CMD_READ_BYTE:       addr: %04xh - %02xh (%3d / '%c')                      | %s\n",
              ptr, val, val, isprint(val) ? val : '.', _kcnet->mem_info(ptr)));
  set_result(val);
}

KCNET_CMD_WRITE_IPADDR::KCNET_CMD_WRITE_IPADDR(KCNET *kcnet) : KCNET_CMD(kcnet, "WRITE_IPADDR", 5)
{
}

void
KCNET_CMD_WRITE_IPADDR::execute(void)
{
  unsigned int idx = get_data(0);
  if (idx < 8)
    {
      DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_WRITE_IPADDR",
                  "KCNET_CMD_WRITE_IPADDR:    ip:   %d.%d.%d.%d (index = %d)\n",
                  get_data(1), get_data(2), get_data(3), get_data(4), idx));
      dword_t addr = (get_data(1) << 24) | (get_data(2) << 16) | (get_data(3) << 8) | get_data(4);
      _kcnet->set_ip_address(idx, addr);
    }
  else
    {
      DBG(1, form("KCemu/KCNET_CMD/KCNET_CMD_WRITE_IPADDR",
                  "KCNET_CMD_WRITE_IPADDR:    ip:   %d.%d.%d.%d (index = %d) IGNORED, INVALID INDEX!\n",
                  get_data(1), get_data(2), get_data(3), get_data(4), idx));
    }

  set_phase(KCNET_CMD_PHASE_FINISHED);
}

KCNET_CMD_READ_IPADDR::KCNET_CMD_READ_IPADDR(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_IPADDR", 1)
{
}

void
KCNET_CMD_READ_IPADDR::execute(void)
{
  byte_t *buf = new byte_t[4];
  unsigned int idx = get_data(0);
  if (idx < 8)
    {
      dword_t addr = _kcnet->get_ip_address(idx);
      buf[0] = addr >> 24;
      buf[1] = addr >> 16;
      buf[2] = addr >> 8;
      buf[3] = addr;
      DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_IPADDR",
                  "KCNET_CMD_READ_IPADDR:     ip:   %d.%d.%d.%d (index = %d)\n",
                  buf[0], buf[1], buf[2], buf[3], idx));
    }
  else
    {
      buf[0] = buf[1] = buf[2] = buf[3] = 0;
      DBG(1, form("KCemu/KCNET_CMD/KCNET_CMD_READ_IPADDR",
                  "KCNET_CMD_READ_IPADDR:     ip:   %d.%d.%d.%d (index = %d) INVALID INDEX!\n",
                  buf[0], buf[1], buf[2], buf[3], idx));
    }

  set_result(buf, 4);
}

KCNET_CMD_READ_PORTNO::KCNET_CMD_READ_PORTNO(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_PORTNO", 0)
{
}

void
KCNET_CMD_READ_PORTNO::execute(void)
{
  word_t portno = 3000;
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_PORTNO",
              "KCNET_CMD_READ_PORTNO:     port: %04xh (%d)\n",
              portno, portno));
  set_result(portno & 0xff, portno >> 8);
}

KCNET_CMD_READ_SW_VERSION::KCNET_CMD_READ_SW_VERSION(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_SW_VERSION", 0)
{
}

void
KCNET_CMD_READ_SW_VERSION::execute(void)
{
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_SW_VERSION",
              "KCNET_CMD_READ_SW_VERSION: ver:  %d.%d\n",
              KCNET::KCNET_SW_VERSION_MAJOR, KCNET::KCNET_SW_VERSION_MINOR));
  set_result(KCNET::KCNET_SW_VERSION_MINOR, KCNET::KCNET_SW_VERSION_MAJOR);
}

KCNET_CMD_READ_HW_VERSION::KCNET_CMD_READ_HW_VERSION(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_HW_VERSION", 0)
{
}

void
KCNET_CMD_READ_HW_VERSION::execute(void)
{
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_HW_VERSION",
              "KCNET_CMD_READ_HW_VERSION: ver:  %d.%d\n",
              KCNET::KCNET_HW_VERSION_MAJOR, KCNET::KCNET_HW_VERSION_MINOR));
  set_result(KCNET::KCNET_HW_VERSION_MINOR, KCNET::KCNET_HW_VERSION_MAJOR);
}

KCNET_CMD_READ_LINK::KCNET_CMD_READ_LINK(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_LINK", 0)
{
}

void
KCNET_CMD_READ_LINK::execute(void)
{
  byte_t val = 1;
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_LINK",
              "KCNET_CMD_READ_LINK:       link: %02xh\n",
              val));
  set_result(val);
}

KCNET_CMD_READ_ID::KCNET_CMD_READ_ID(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_ID", 0)
{
}

void
KCNET_CMD_READ_ID::execute(void)
{
  byte_t *buf = new byte_t[6];
  buf[5] = 'K';
  buf[4] = 'C';
  buf[3] = 'e';
  buf[2] = 'm';
  buf[1] = 'u';
  buf[0] = 0;
  DBG(2, form("KCemu/KCNET_CMD/KCNET_CMD_READ_ID",
              "KCNET_CMD_READ_ID:         id:   '%s'\n",
              buf));
  set_result(buf, 6);
}

KCNET_CMD_READ_ERROR::KCNET_CMD_READ_ERROR(KCNET *kcnet) : KCNET_CMD(kcnet, "READ_ERROR", 0)
{
}

void
KCNET_CMD_READ_ERROR::execute(void)
{
}
