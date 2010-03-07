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

#ifndef __kc_tape_h
#define __kc_tape_h

#include <fstream>

#include "kc/system.h"

#include "kc/cb.h"

#include "cmd/cmd.h"

#include "libtape/kct.h"

#include "fileio/load.h"

typedef enum
{
  TAPE_OK,
  TAPE_OK_READONLY,
  TAPE_NOENT,
  TAPE_ERROR,
} tape_error_t;

/*
 *
 *  system clock 1.75 Mhz (KC 85/3, KC 85/4)
 *
 *  const int BIT_0 =  364;  - 2400 Hz
 *  const int BIT_1 =  729;  - 1200 Hz
 *  const int BIT_S = 1458;  -  600 Hz
 *
 *  system clock 2.5 Mhz (Z9001, KC 85/1, KC87)
 *
 *  const int BIT_0 =  520;  - 2400 Hz
 *  const int BIT_1 = 1041;  - 1200 Hz
 *  const int BIT_S = 2083;  -  600 Hz
 */

class TapeCallback {
 public:
  TapeCallback(void) {}
  virtual ~TapeCallback(void) {}

  virtual void tape_callback(byte_t val) = 0;
};

class Tape : public Callback
{
public:
  enum {
    BLOCK_SIZE = 131,
  };
  
private:
    int BIT_0;
    int BIT_1;
    int BIT_S;
    int _start_block;

    bool _power;
    bool _play;
    bool _record;
    
    int _flip_flop;
    int _sync;
    int _sync_count;
    int _bits;
    int _state;
    int _block;
    int _byte_counter;
    int _init;

    byte_t _byte;
    byte_t _crc;
    byte_t _crc_calculated;
    byte_t _buf[129];
    byte_t _last_val;
    byte_t _last_block;

    long _file_size;
    long _bytes_read;
    kct_file_type_t _file_type;
    
    std::ostream *_os;
    std::istream *_is;

    KCTFile _kct_file;

    TapeCallback *_tape_cb;

    CMD *_cmd_tape_load;
    CMD *_cmd_tape_play;
    CMD *_cmd_tape_attach;
    CMD *_cmd_tape_export;
    CMD *_cmd_tape_add_file;

 protected:
    void update_tape_list(void);
    bool check_addr(byte_t *data, long size);
    bool check_com(byte_t *data, long size);
    const char *get_filename(byte_t *data);
    
 public:
    Tape(int bit_0, int bit_1, int bit_s, int start_block);
    virtual ~Tape(void);

    void power(bool val);
    void record(void);
    void play(const char *name, int delay);
    void stop(void);
    void seek(int percent);
    void do_play(int edge);
    void do_play_bic(int edge);
    void do_play_z1013(int edge);
    void do_play_basicode(int edge);
    void do_stop(void);
    void tape_signal(void);
    void tape_signal_bic(long diff);
    void tape_signal_z1013(long diff);
    long get_delay(int seconds);

    virtual void set_tape_callback(TapeCallback *tape_cb);

    virtual void callback(void *data);

    virtual tape_error_t attach(const char *filename, bool create = false);
    virtual tape_error_t detach(void);
    virtual tape_error_t add(const char *name);
    virtual tape_error_t add_raw(const char *filename,
                                 const char *tape_filename,
                                 const char *kc_filename,
                                 unsigned short load,
                                 unsigned short start,
                                 bool autostart);
    virtual tape_error_t add_file(const char *name,
				  fileio_prop_t *prop,
				  kct_file_type_t type,
				  kct_machine_type_t machine);
    virtual tape_error_t rename(const char *from, const char *to);
    virtual tape_error_t remove(const char *name);
    virtual tape_error_t export_tap(const char *name,
				    const char *filename);
    virtual tape_error_t export_wav(const char *name,
				    const char *filename);
    virtual std::istream * read(const char *name,
			   kct_file_props_t *props = NULL);
};

#endif /* __kc_tape_h */
