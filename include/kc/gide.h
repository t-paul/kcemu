/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
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

#ifndef __kc_gide_h
#define __kc_gide_h

#include "kc/system.h"

#include "kc/ports.h"

class GIDE : public PortInterface
{
 private:
  enum {
    GIDE_STATUS_ERROR          = (1 << 0),
    GIDE_STATUS_INDEX          = (1 << 1),
    GIDE_STATUS_CORRECTED_DATA = (1 << 2),
    GIDE_STATUS_DATA_REQUEST   = (1 << 3),
    GIDE_STATUS_SEEK_COMPLETE  = (1 << 4),
    GIDE_STATUS_WRITE_FAULT    = (1 << 5),
    GIDE_STATUS_READY          = (1 << 6),
    GIDE_STATUS_BUSY           = (1 << 7),
  };

 private:
  int _cylinder;
  int _sector;
  int _sector_count;
  int _head;
  int _drive;
  int _status;
  int _error;

 protected:
  byte_t in_data(void);
  byte_t in_error(void);
  byte_t in_sector_count();
  byte_t in_sector_number();
  byte_t in_cylinder_low();
  byte_t in_cylinder_high();
  byte_t in_drive_head();
  byte_t in_status(void);

  void out_command(byte_t val);
  void out_digital_output(byte_t val);
  void out_sector_count(byte_t val);
  void out_sector_number(byte_t val);
  void out_cylinder_low(byte_t val);
  void out_cylinder_high(byte_t val);
  void out_drive_head(byte_t val);

 public:
  GIDE(void);
  virtual ~GIDE(void);
  
  /* PortInterface */
  byte_t in(word_t addr);
  void out(word_t addr, byte_t val);
};

#endif /* __kc_gide_h */
