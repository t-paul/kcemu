/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
 *
 *  $Id$
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __kc_vdip_h
#define __kc_vdip_h

#include <string>

#include "kc/kc.h"
#include "kc/cb.h"
#include "kc/pio.h"

#include "kc/prefs/strlist.h"

using namespace std;

class VDIP : public Callback, public PIOCallbackInterface
{
private:
  enum {
    CMD_DIR = 0x01,
    CMD_CD  = 0x02,
    CMD_CLF = 0x0a,
    CMD_RDF = 0x0b,
    CMD_OPR = 0x0e,
    CMD_SCS = 0x10,
    CMD_ECS = 0x11,
  };

  typedef enum {
    ERR_COMMAND_FAILED,
    ERR_INVALID,
    ERR_BAD_COMMAND,
  } vdip_error_t;
  
  int  _cbval;
  PIO *_pio;
  bool _input;
  bool _reset;
  int _output;
  int _input_data;
  string _input_buffer;
  string _output_buffer;
  byte_t _pio_ext;
  bool _short_command_set;
  FILE *_file;
  StringList *_cwd;

private:
  void set_pio_ext_b(byte_t val);

public:
  VDIP(void);
  virtual ~VDIP(void);

  virtual void register_pio(PIO *pio);
  
  virtual void callback(void *data);

  virtual void reset(void);

  /** latch byte to read, called when RD# goes H->L */
  virtual void latch_byte(void);

  /** return the byte latched on latch_byte() */
  virtual byte_t read_byte(void);

  /** byte was read, called when RD# goes L->H */
  virtual void read_end(void);
 
  /** write byte, called when WR# goes L->H */
  virtual void write_byte(byte_t val);

  /** byte was written, called when WR# goes H->L */
  virtual void write_end(void);

  virtual void send_prompt(void);
  virtual void send_error(vdip_error_t error);
  virtual void send_char(int c);
  virtual void send_dword(dword_t val);
  virtual void send_string(const char *text);

  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

#endif /* __kc_vdip_h */
