/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: kc.h,v 1.15 2002/06/09 14:24:32 torsten_paul Exp $
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

#ifndef __kc_kc_h
#define __kc_kc_h

#include "kc/system.h"

typedef enum
{
  KC_TYPE_NONE       = 0,
  KC_TYPE_85_1 	     = (1 << 0),
  KC_TYPE_85_2 	     = (1 << 1),
  KC_TYPE_85_3 	     = (1 << 2),
  KC_TYPE_85_4 	     = (1 << 3),
  KC_TYPE_87         = (1 << 4),
  KC_TYPE_LC80 	     = (1 << 5),
  KC_TYPE_85_1_CLASS = KC_TYPE_85_1 | KC_TYPE_87,
  KC_TYPE_85_2_CLASS = KC_TYPE_85_2 | KC_TYPE_85_3 | KC_TYPE_85_4,
  KC_TYPE_ALL        = KC_TYPE_85_1_CLASS | KC_TYPE_85_2_CLASS | KC_TYPE_LC80,
} kc_type_t;

class Z80;
class Z80_FDC;
class UI;
class Ports;
class Module;
class Memory;
class CTC;
class PIO;
class Tape;
class Disk;
class Timer;
class Keyboard;
class ModuleList;
class CMD;
class FloppySharedMem;
class FloppyIO;
class FDC;
class Sound;

extern Z80             *z80;
extern UI              *ui;
extern Ports           *ports;
extern Module          *module;
extern Memory          *memory;
extern CTC 	       *ctc;
extern PIO             *pio;
extern PIO             *pio2;
extern Tape            *tape;
extern Disk            *disk;
extern Timer           *timer;
extern Keyboard        *keyboard;
extern ModuleList      *module_list;
extern CMD             *cmd;

extern Z80_FDC         *fdc_z80;     
extern FloppyIO        *fdc_io;
extern FloppySharedMem *fdc_shmem;
extern Ports           *fdc_ports;
extern FDC             *fdc_fdc;
extern Sound           *sound;
extern byte_t           fdc_mem[];

extern int   kcemu_ui_scale;
extern char *kcemu_datadir;
extern char *kcemu_localedir;

kc_type_t get_kc_type(void);

#endif /* __kc_kc_h */
