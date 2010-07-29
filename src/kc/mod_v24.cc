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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/un.h>
#include <sys/signal.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/z80.h"
#include "kc/mod_v24.h"
#include "kc/prefs/prefs.h"

#include "libdbg/dbg.h"

using namespace std;

static ModuleV24 *self;

ModuleV24::ModuleV24(ModuleV24 &tmpl) :
  ModuleInterface(tmpl.get_name(), tmpl.get_id(), tmpl.get_type()),
  InterfaceCircuit("ModuleV24")
{
  _reg[A] = 0;
  _reg[B] = 0;
  _data_in[A] = 0;
  _data_in[B] = 0;
  _reg_rd[A][0] = 0xff;
  _reg_rd[A][1] = 0xff;
  _reg_rd[A][2] = 0xff;
  _reg_rd[B][0] = 0xff;
  _reg_rd[B][1] = 0xff;
  _reg_rd[B][2] = 0xff;

  _irq_active[A] = 0;
  _irq_active[B] = 0;
  _irq_pending[A] = 0;
  _irq_pending[B] = 0;

  _fd_in[A] = 0;
  _fd_in[B] = 0;
  _fd_out[A] = 0;
  _fd_out[B] = 0;
  _in_buf_ptr = 0;
  _socket_name = 0;
}

ModuleV24::ModuleV24(const char *name, byte_t id) :
  ModuleInterface(name, id, KC_MODULE_KC_85_3),
  InterfaceCircuit("ModuleV24")
{
  _fd_in[A] = 0;
  _fd_in[B] = 0;
  _fd_out[A] = 0;
  _fd_out[B] = 0;
  _in_buf_ptr = 0;
  _socket_name = 0;
}

ModuleV24::~ModuleV24(void)
{
  close_device();
}

byte_t
ModuleV24::in(word_t addr)
{
  byte_t val;
  int port = addr & 0xff;

  val = 0xff;
  switch (port)
    {
    case 0x80:
      return get_id();
      break;
    case 0x08:
      /* SIO channel A - data */
      val = _data_in[A];
      DBG(2, form("KCemu/ModuleV24/in/data/A",
                  "%04x: in_data: <- sio A 08h <- %0xh\n",
                  z80->getPC(), val));
      break;
    case 0x09:
      /* SIO channel B - data */
      val = _data_in[B];
      DBG(2, form("KCemu/ModuleV24/in/data/B",
                  "%04x: in_data: <- sio B 08h <- %0xh\n",
                  z80->getPC(), val));
      break;
    case 0x0a:
      /* SIO channel A - control */
      val = in_reg(A);
      break;
    case 0x0b:
      /* SIO channel B - control */
      val = in_reg(B);
      break;
    case 0x0c:
      val = 0xff;
      DBG(2, form("KCemu/ModuleV24/in",
                  "%04xh: in_reg:  <- ctc 0 0ch <- %0xh\n",
                  z80->getPC(), val));
      break;
    case 0x0d:
      val = 0xff;
      DBG(2, form("KCemu/ModuleV24/in",
                  "%04x: in_reg:  <- ctc 1 0dh <- %0xh\n",
                  z80->getPC(), val));
      break;
    case 0x0e:
      val = 0xff;
      DBG(2, form("KCemu/ModuleV24/in",
                  "%04x: in_reg:  <- ctc 2 0eh <- %0xh\n",
                  z80->getPC(), val));
      break;
    case 0x0f:
      val = 0xff;
      DBG(2, form("KCemu/ModuleV24/in",
                  "%04x: in_reg:  <- ctc 3 0eh <- %0xh\n",
                  z80->getPC(), val));
      break;
    }
  
  DBG(3, form("KCemu/ModuleV24/in",
              "%04x: %s: %04x, %02x\n",
              z80->getPC(), __PRETTY_FUNCTION__, addr, val));
  return val;
}

void
ModuleV24::out(word_t addr, byte_t val)
{
  int port = addr & 0xff;

  DBG(3, form("KCemu/ModuleV24/out",
              "%s: %04x, %02x\n",
              __PRETTY_FUNCTION__, addr, val));
  switch (port)
    {
    case 0x80:
      if (((_val & 1) ^ (val & 1)) != 1) return;
      if (val & 1)
        {
          open_device();
          _portg = ports->register_ports(get_name(), 8, 6, this, (addr >> 8));
        }
      else
        close_device();
      
      _val = val;
      break;
    case 0x08:
      /* SIO channel A - data */
      DBG(2, form("KCemu/ModuleV24/out/data/A",
                  "out_data: sio A 08h: %0xh (io_type = %d)\n",
                  val, _io_type));
      if (_fd_out[A] && (_io_type != IO_NONE)) 
        {
          if (_io_type == IO_FILE)
            write(_fd_out[A], &val, 1);
          else
            send(_fd_out[A], &val, 1, 0);
        }
      break;
    case 0x09:
      /* SIO channel B - data */
      DBG(2, form("KCemu/ModuleV24/out/data/B",
                  "out_data: sio B 09h: %0xh (io_type = %d)\n",
                  val, _io_type));
      if (_fd_out[B] && (_io_type != IO_NONE))
        {
          if (_io_type == IO_FILE)
            write(_fd_out[B], &val, 1);
          else
            send(_fd_out[B], &val, 1, 0);
        }
      break;
    case 0x0a:
      /* SIO channel A - control */
      out_reg(A, val);
      break;
    case 0x0b:
      /* SIO channel B - control */
      out_reg(B, val);
      break;
    case 0x0c:
      DBG(2, form("KCemu/ModuleV24/out/ctc/0",
                  "out_reg: -> ctc 0 0ch -> %0xh\n",
                  val));
      break;
    case 0x0d:
      DBG(2, form("KCemu/ModuleV24/out/ctc/1",
                  "out_reg: -> ctc 1 0dh -> %0xh\n",
                  val));
      break;
    case 0x0e:
      DBG(2, form("KCemu/ModuleV24/out/ctc/2",
                  "out_reg: -> ctc 2 0eh -> %0xh\n",
                  val));
      break;
    case 0x0f:
      DBG(2, form("KCemu/ModuleV24/out/ctc/3",
                  "out_reg: -> ctc 3 0eh -> %0xh\n",
                  val));
      break;
    }
}

byte_t
ModuleV24::in_reg(int c)
{
  DBG(2, form("KCemu/ModuleV24/in_reg/raw",
              "in_reg:  <- [%d] %02xh <- %d\n",
              c, _reg_rd[c][_reg[c]], _reg[c]));
  return _reg_rd[c][_reg[c]];
}

void
ModuleV24::out_reg(int c, byte_t val)
{
  static const char *wr0_0[] = {
    "",
    " next write to register 1\n",
    " next write to register 2\n",
    " next write to register 3\n",
    " next write to register 4\n",
    " next write to register 5\n",
    " next write to register 6\n",
    " next write to register 7\n",
  };
  static const char *wr0_1[] = {
    "",
    " send break (SDLC)\n",
    " reset external and status interrupts\n",
    " reset channel\n",
    " reset receive interrupt on first character\n",
    " reset send interrupt\n",
    " reset error condition\n",
    " interrupt return (channel A)\n",
  };
  static const char *wr0_2[] = {
    "",
    " reset receiver CRC\n",
    " reset sender CRC\n",
    " reset CRC/SYNC status memory\n",
  };
    
  DBG(2, form("KCemu/ModuleV24/out_reg/raw",
              "out_reg: -> [%d] %02xh -> %d\n",
              c, val, _reg[c]));
  _reg_wr[c][_reg[c]] = val;
  switch (_reg[c])
    {
    case 0:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr0",
                  "wr0: [%d] %02xh -> %d\n%s%s%s",
                  c, val, _reg[c],
                  wr0_0[val & 7],
                  wr0_1[(val >> 3) & 7],
                  wr0_2[(val >> 6) & 3]));
          break;
    case 1:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr1",
                  "wr1: [%d] %02xh -> %d\n",
                  c, val, _reg[c]));
      break;
    case 2:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr2",
                  "wr2: [%d] %02xh -> %d\n new interrupt vector: %02x\n",
                  c, val, _reg[c], val));
      break;
    case 3:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr3",
                  "wr3: [%d] %02xh -> %d\n",
                  c, val, _reg[c]));
      break;
    case 4:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr4",
                  "wr4: [%d] %02xh -> %d\n",
                  c, val, _reg[c]));
      break;
    case 5:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr5",
                  "wr5: [%d] %02xh -> %d\n",
                  c, val, _reg[c]));
      break;
    case 6:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr6",
                  "wr6: [%d] %02xh -> %d\n",
                  c, val, _reg[c]));
      break;
    case 7:
      DBG(1, form("KCemu/ModuleV24/out_reg/reg/wr7",
                  "wr7: [%d] %02xh -> %d\n",
                  c, val, _reg[c]));
      break;
    }
  if (_reg[c] == 0)
    _reg[c] = val & 7;
  else
    _reg[c] = 0;
}

ModuleInterface *
ModuleV24::clone(void)
{
  return new ModuleV24(*this);
}

void
ModuleV24::reset(bool power_on)
{
  _irq_active[A] = 0;
  _irq_active[B] = 0;
}

void
ModuleV24::reti(void)
{
  _irq_active[B] = 0;
  _irq_pending[B] = 0;

  if (!_in_buf_ptr)
    return;
  
//  if (z80->triggerIrq(_reg_wr[B][2]))
//    {
//      _irq_active[B] = 1;
//      _data_in[B] = *_in_buf_ptr++;
//      if (*_in_buf_ptr == 0) _in_buf_ptr = 0;
//      z80->handleIrq(_reg_wr[B][2]);
//      return;
//    }
//  else
//    _irq_pending[B] = 1;
}

void
ModuleV24::push_data(char *buf, int len)
{
  if (_in_buf_ptr)
    {
      cerr << "data ignored! ***" << endl;
      return;
    }

  if (_irq_active[B])
    {
      cout << "irq still active!" << endl;
      return;
    }

  strcpy((char *)_in_buf, buf);
  _in_buf_ptr = _in_buf;

//  if (z80->triggerIrq(_reg_wr[B][2]))
//    {
//      _irq_active[B] = 1;
//      _data_in[B] = *_in_buf_ptr++;
//      if (*_in_buf_ptr == 0) _in_buf_ptr = 0;
//      z80->handleIrq(_reg_wr[B][2]);
//      return;
//    }
//  else
//    _irq_pending[B] = 1;
}

void
ModuleV24::signal_handler_IO_read(int status)
{
  int a;
  
  static char buf[INBUF_LEN];

  DBG(1, form("KCemu/ModuleV24/signal",
              "signal_handler_IO_read\n"));
  a = read(self->_fd_in[B], buf, INBUF_LEN - 1);
  buf[a] = '\0';
  DBG(2, form("KCemu/ModuleV24/signal/data",
              "got %d bytes: '%s'\n",
              a, buf));
  self->push_data(&buf[0], a);
}

void
ModuleV24::signal_handler_IO_recv(int status)
{
  int a;
  
  static char buf[INBUF_LEN];

  DBG(1, form("KCemu/ModuleV24/signal",
              "signal_handler_IO_recv\n"));
  a = recv(self->_fd_in[B], buf, INBUF_LEN, 0);
  buf[a] = '\0';
  DBG(2, form("KCemu/ModuleV24/signal/data",
              "got %d bytes: '%s'\n",
              a, buf));
  self->push_data(&buf[0], a);
}

void
ModuleV24::set_signal_handler(int fd, void (*sig_func)(int))
{
  struct sigaction saio;

  DBG(1, form("KCemu/ModuleV24/signal",
              "setting signal handler for fd = %d\n", fd));

  memset(&saio, 0, sizeof(saio));

  if (fcntl(fd, F_SETOWN, getpid()) < 0)
    cerr << "can't set owner on filedescriptor " << fd << " to " << getpid() << endl;
  if (fcntl(fd, F_SETFL, FASYNC) < 0)
    cerr << "can't set FASYNC flag on filedescriptor " << fd << endl;

  self = this;
  saio.sa_handler = sig_func;
  sigemptyset(&saio.sa_mask);
  saio.sa_flags = 0;

  sigaction(SIGIO, &saio, (struct sigaction *)NULL);
}

bool
ModuleV24::open_device_serial(int dev, const char *dev_name)
{
  int baudrate = Preferences::instance()->get_int_value("v24_baudrate", 2400);

  _fd_in[dev] = open(dev_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (_fd_in[dev] == 0)
    return false;
  _fd_out[dev] = _fd_in[dev];

  set_signal_handler(_fd_in[dev], signal_handler_IO_read);

  tcgetattr(_fd_in[dev], &_tio_old[dev]);
  bzero(&_tio_new[dev], sizeof(_tio_new[dev]));
  
  /*
   *  BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
   *  CRTSCTS : output hardware flow control (only used if the cable has
   *            all necessary lines. See sect. 7 of Serial-HOWTO)
   *  CS8     : 8n1 (8bit,no parity,1 stopbit)
   *  CLOCAL  : local connection, no modem contol
   *  CREAD   : enable receiving characters
   */
  _tio_new[dev].c_cflag = CRTSCTS | CS8 | CLOCAL | CREAD;
  switch (baudrate)
    {
    case 50:    _tio_new[dev].c_cflag |= B50   ; break;
    case 75:    _tio_new[dev].c_cflag |= B75   ; break;
    case 110:   _tio_new[dev].c_cflag |= B110  ; break;
    case 134:   _tio_new[dev].c_cflag |= B134  ; break;
    case 150:   _tio_new[dev].c_cflag |= B150  ; break;
    case 200:   _tio_new[dev].c_cflag |= B200  ; break;
    case 300:   _tio_new[dev].c_cflag |= B300  ; break;
    case 600:   _tio_new[dev].c_cflag |= B600  ; break;
    case 1200:  _tio_new[dev].c_cflag |= B1200 ; break;
    case 1800:  _tio_new[dev].c_cflag |= B1800 ; break;
    case 2400:  _tio_new[dev].c_cflag |= B2400 ; break;
    case 4800:  _tio_new[dev].c_cflag |= B4800 ; break;
    case 9600:  _tio_new[dev].c_cflag |= B9600 ; break;
    case 19200: _tio_new[dev].c_cflag |= B19200; break;
    case 38400: _tio_new[dev].c_cflag |= B38400; break;
    default:
      baudrate = 9600;
      _tio_new[dev].c_cflag |= B9600 ;
      break;
    }
  
  DBG(1, form("KCemu/ModuleV24/open",
              "ModuleV24::open_device_serial(): device = '%s', baudrate = %d\n",
              dev_name, baudrate));


  /*
   *  IGNPAR  : ignore bytes with parity errors
   *  ICRNL   : map CR to NL (otherwise a CR input on the other computer
   *            will not terminate input)
   *  otherwise make device raw (no other input processing)
   */
  _tio_new[dev].c_iflag = IGNPAR | ICRNL;
  _tio_new[dev].c_iflag = IGNPAR;

  /*
   *  Raw output.
   */
  _tio_new[dev].c_oflag = 0;
  
  /*
   *  ICANON  : enable canonical input
   *  disable all echo functionality, and don't send signals to calling program
   */
  _tio_new[dev].c_lflag = ICANON;
  _tio_new[dev].c_lflag = 0;

  _tio_new[dev].c_cc[VMIN]=1;
  _tio_new[dev].c_cc[VTIME]=0;

  tcflush(_fd_in[dev], TCIFLUSH);
  tcsetattr(_fd_in[dev], TCSANOW, &_tio_new[dev]);

  return true;
}

void
ModuleV24::fifo_server(int fd)
{
  int ret, len, fd_read, fd_read_w, fd_write, fd_write_r;
  struct pollfd pollfds[1];
  char buf[1025];

  DBG(1, form("KCemu/ModuleV24/server",
              "ModuleV24::fifo_server() fd = %d\n",
              fd));

  if (unlink("/tmp/KCemu-FIFO-in") != 0)
    perror("unlink");
  if (unlink("/tmp/KCemu-FIFO-out") != 0)
    perror("unlink");

  if (mkfifo("/tmp/KCemu-FIFO-in", 0600) != 0)
    {
      perror("can't create FIFO");
      exit(1);
    }
  if (mkfifo("/tmp/KCemu-FIFO-out", 0600) != 0)
    {
      perror("can't create FIFO");
      exit(1);
    }

  fd_read = open("/tmp/KCemu-FIFO-in", O_RDONLY | O_NONBLOCK);
  if (fd_read < 0)
    {
      perror("open (read)");
      exit(1);
    }
  /*
   *  open the fifo for writing too to keep it open if the
   *  external writer closes it's side of the fifo
   */
  fd_read_w = open("/tmp/KCemu-FIFO-in", O_WRONLY | O_NONBLOCK);

  fd_write_r = open("/tmp/KCemu-FIFO-out", O_RDONLY | O_NONBLOCK);
  fd_write = open("/tmp/KCemu-FIFO-out", O_WRONLY | O_NONBLOCK);
  if (fd_write < 0)
    {
      perror("open (write)");
      exit(1);
    }

  while (242)
    {
      pollfds[0].fd = fd_read;
      pollfds[0].events = POLLIN;
      pollfds[0].revents = 0;
      pollfds[1].fd = fd;
      pollfds[1].events = POLLIN;
      pollfds[1].revents = 0;
      if ((ret = poll(pollfds, 2, -1)) < 0)
        exit(2);
      
      if ((pollfds[0].revents & POLLIN) == POLLIN)
        {
          len = read(fd_read, buf, 1024);
          if (len == 0)
            {
              cerr << "ModuleV24::fifo_server(): read error" << endl;
              exit(1);
            }
          send(fd, buf, len, 0);
        }
      if ((pollfds[1].revents & POLLIN) == POLLIN)
        {
          len = read(fd, buf, 1024);
          write(fd_write, buf, len);
        }
    }
}

void
ModuleV24::socket_server(int fd)
{
  int s, c, len;
  char buf[1024];
  unsigned int cli_addr_len;
  struct sockaddr_un unix_addr, cli_addr;

  _socket_name = tempnam("/tmp", "KCemu");
  if (_socket_name == NULL)
    exit(1);
  _socket_name = strdup("/tmp/KCemu-in");

  s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (s < 0)
    exit(2);
  
  bzero((char *)&unix_addr, sizeof(unix_addr));
  unix_addr.sun_family = AF_UNIX;
  strcpy(unix_addr.sun_path, _socket_name);
  len = strlen(unix_addr.sun_path) + sizeof(unix_addr.sun_family);

  unlink(_socket_name);
  if (bind(s, (struct sockaddr *)&unix_addr, len) != 0)
    exit(3);

  if (listen(s, 5) != 0)
    exit(4);
  
  while (242)
    {
      if ((c = accept(s, (struct sockaddr *)&cli_addr, &cli_addr_len)) < 0)
        continue;
      
      while (242)
        {
          len = recv(c, buf, 1024, 0);
          if (len == 0)
            break;
          send(fd, buf, len, 0);
        }
    }
}

bool
ModuleV24::open_device_socket_or_fifo(io_type_t io_type)
{
  int fd[2];

  /*
   *  we use a pair of sockets for to communicate with the
   *  child process, fd[0] is used by the server, fd[1] by
   *  the child process
   */
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) != 0)
    return false;

  _pid = fork();

  if (_pid < 0)
    return false;

  if (_pid > 0)
    {
      _fd_in[A] = fd[0];
      _fd_out[A] = fd[0];
      set_signal_handler(_fd_in[A], signal_handler_IO_recv);
      return true;
    }

  if (io_type == IO_FIFO)
    fifo_server(fd[1]);
  else
    socket_server(fd[1]);
  return true;
}

void
ModuleV24::open_device(void)
{
  bool ret;
  const char *s;
  const char *dev_name;

  dev_name = Preferences::instance()->get_string_value("v24_device", "/dev/ttyS1");

  _io_type = IO_NONE;
  if (strcmp(dev_name, "SOCKET") == 0)
    {
      _io_type = IO_SOCKET;
      ret = open_device_socket_or_fifo(IO_SOCKET);
    }
  else if (strcmp(dev_name, "FIFO") == 0)
    {
      _io_type = IO_FIFO;
      ret = open_device_socket_or_fifo(IO_FIFO);
    }
  else
    {
      _io_type = IO_FIFO;
      ret = open_device_serial(B, dev_name);
    }

  if (!ret)
    return;

  z80->register_ic(this);

  s = "\r\nKCemu v" KCEMU_VERSION " - V24 module active\r\n\r\n";
  write(_fd_out[B], s, strlen(s));
}

void
ModuleV24::close_device(void)
{
  if (_socket_name != 0)
    {
      unlink(_socket_name);
      free(_socket_name);
    }

  if (_fd_in[A] != 0)
    {
      tcsetattr(_fd_in[A] , TCSANOW, &_tio_old[A]);
      close(_fd_in[A]);
      close(_fd_out[A]);
    }

  z80->unregister_ic(this);
}

void
ModuleV24::m_out(word_t addr, byte_t val)
{
}
