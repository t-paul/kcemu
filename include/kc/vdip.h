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

#ifndef __kc_vdip_h
#define __kc_vdip_h

#include <string>

#include "kc/kc.h"
#include "kc/cb.h"
#include "kc/pio.h"

#include "kc/prefs/strlist.h"

using namespace std;

class VDIP;

typedef enum
{
  CMD_DIR  = 0x01,
  CMD_CD   = 0x02,
  CMD_RD   = 0x04,
  CMD_DLD  = 0x05,
  CMD_MKD  = 0x06,
  CMD_DLF  = 0x07,
  CMD_WRF  = 0x08,
  CMD_OPW  = 0x09,
  CMD_CLF  = 0x0a,
  CMD_RDF  = 0x0b,
  CMD_REN  = 0x0c,
  CMD_OPR  = 0x0e,
  CMD_IDD  = 0x0f,
  CMD_SCS  = 0x10,
  CMD_ECS  = 0x11,
  CMD_FS   = 0x12,
  CMD_FWV  = 0x13,
  CMD_SEK  = 0x28,
  CMD_DSN  = 0x2d,
  CMD_DVL  = 0x2e,
  CMD_DIRT = 0x2f,
  CMD_E    = 0x45,
  CMD_e    = 0x65,
  CMD_IPA  = 0x90,
  CMD_IPH  = 0x91,
  CMD_FSE  = 0x93,
  CMD_IDDE = 0x94,

  CMD_EMPTY   = 0xfffe,
  CMD_UNKNOWN = 0xffff,
} vdip_command_t;

typedef enum
{
  ERR_BAD_COMMAND,
  ERR_COMMAND_FAILED,
  ERR_DISK_FULL,
  ERR_INVALID,
  ERR_READ_ONLY,
  ERR_FILE_OPEN,
  ERR_DIR_NOT_EMPTY,
  ERR_FILENAME_INVALID,
  ERR_NO_UPGRADE,
  ERR_NO_DISK,
} vdip_error_t;

class VDIP_CMD {
private:
  VDIP *_vdip;
  string _response;
  StringList *_args;
  bool _check_disk;

protected:
  virtual void execute(void) = 0;

  static vdip_command_t map_extended_command(string cmd);

public:
  VDIP_CMD(VDIP *vdip, bool check_disk = false);
  virtual ~VDIP_CMD(void);

  virtual VDIP *get_vdip(void);

  virtual void add_prompt(void);
  virtual void add_error(vdip_error_t error);
  virtual void add_char(int c);
  virtual void add_hex(int digits, dword_t val);
  virtual void add_word(word_t val);
  virtual void add_dword(dword_t val);
  virtual void add_string(const char *text);

  virtual bool has_args(void) const;
  virtual int get_arg_count(void) const;
  virtual string get_arg(unsigned int arg) const;
  virtual dword_t get_dword_arg(unsigned int arg) const;
  virtual void set_args(StringList *args);
  virtual string get_response(string input);

  virtual void exec(void);
  virtual void handle_input(byte_t input);

  static VDIP_CMD * create_command(VDIP *vdip, string cmd);
  static VDIP_CMD * create_command(VDIP *vdip, vdip_command_t code);
};

class VDIP : public Callback, public PIOCallbackInterface
{
private:
  PIO *_pio;
  VDIP_CMD *_cmd;
  bool _input;
  dword_t _wrf_len;
  bool _reset;
  int _output;
  int _input_data;
  string _input_buffer;
  string _output_buffer;
  byte_t _pio_ext;
  bool _short_command_set;
  bool _binary_mode;
  FILE *_file;
  string _root;
  StringList *_cwd;
  CMD *_attach_cmd;

private:
  void set_pio_ext_b(byte_t val);

  VDIP_CMD * decode_command(string buf);

public:
  VDIP(void);
  virtual ~VDIP(void);

  virtual string get_firmware_version(void) const;

  virtual bool is_binary_mode(void) const;
  virtual void set_binary_mode(bool val);

  virtual bool is_short_command_set(void) const;
  virtual void set_short_command_set(bool val);

  virtual FILE * get_file(void) const;
  virtual void set_file(FILE *file);

  virtual string get_root(void) const;
  virtual void set_root(string root);

  virtual bool is_root(void);
  virtual bool has_disk(void);

  virtual string get_cwd(void) const;
  virtual string get_path(string dir) const;
  virtual void chdir_up(void);
  virtual void chdir_root(void);
  virtual void chdir(string dir);

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

  /*
   *  PIOCallbackInterface
   */
  virtual int callback_A_in(void);
  virtual int callback_B_in(void);
  virtual void callback_A_out(byte_t val);
  virtual void callback_B_out(byte_t val);
};

#endif /* __kc_vdip_h */
