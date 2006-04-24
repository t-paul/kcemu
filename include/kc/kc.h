/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2004 Torsten Paul
 *
 *  $Id: kc.h,v 1.16 2002/10/31 01:46:33 torsten_paul Exp $
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
  KC_TYPE_85_1 	     = (1 <<  0),
  KC_TYPE_85_2 	     = (1 <<  1),
  KC_TYPE_85_3 	     = (1 <<  2),
  KC_TYPE_85_4 	     = (1 <<  3),
  KC_TYPE_85_5 	     = (1 <<  4),
  KC_TYPE_87         = (1 <<  5),
  KC_TYPE_LC80 	     = (1 <<  6),
  KC_TYPE_Z1013      = (1 <<  7),
  KC_TYPE_A5105      = (1 <<  8),
  KC_TYPE_POLY880    = (1 <<  9),
  KC_TYPE_KRAMERMC   = (1 << 10),
  KC_TYPE_MUGLERPC   = (1 << 11),
  KC_TYPE_85_1_CLASS = KC_TYPE_85_1 | KC_TYPE_87,
  KC_TYPE_85_2_CLASS = KC_TYPE_85_2 | KC_TYPE_85_3 | KC_TYPE_85_4 | KC_TYPE_85_5,
  KC_TYPE_ALL        = (KC_TYPE_85_1_CLASS |
			KC_TYPE_85_2_CLASS |
			KC_TYPE_LC80 |
			KC_TYPE_Z1013 |
			KC_TYPE_A5105 |
			KC_TYPE_POLY880 |
			KC_TYPE_KRAMERMC |
			KC_TYPE_MUGLERPC)
} kc_type_t;

typedef enum
{
  KC_VARIANT_NONE    	 = 0,
  KC_VARIANT_85_1_10 	 = 0,
  KC_VARIANT_85_1_11 	 = 1,
  KC_VARIANT_87_10   	 = 0,
  KC_VARIANT_87_11   	 = 1,
  KC_VARIANT_87_20   	 = 2,
  KC_VARIANT_87_21   	 = 3,
  KC_VARIANT_87_30   	 = 4,
  KC_VARIANT_87_31   	 = 5,
  KC_VARIANT_Z1013_01    = 0,
  KC_VARIANT_Z1013_12    = 1,
  KC_VARIANT_Z1013_16    = 3,
  KC_VARIANT_Z1013_64    = 4,
  KC_VARIANT_Z1013_A2    = 5,
  KC_VARIANT_Z1013_RB    = 6,
  KC_VARIANT_Z1013_SURL  = 7,
  KC_VARIANT_Z1013_BL4   = 8,
  KC_VARIANT_A5105_K1505 = 0,
  KC_VARIANT_A5105_A5105 = 1
} kc_variant_t;

typedef struct
{
  char         *name;
  int           type;
  kc_type_t     kc_type;
  kc_variant_t  kc_variant;
  char         *desc;
} kc_variant_names_t;

class Z80;
class Z80_FDC;
class DaisyChain;
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
class PortInterface;
class GDC;
class VIS;
class Poly880;

extern Z80             *z80;
extern DaisyChain      *daisy;
extern UI              *ui;
extern Ports           *ports;
extern PortInterface   *porti;
extern Module          *module;
extern Memory          *memory;
extern CTC 	       *ctc;
extern CTC 	       *ctc2;
extern PIO             *pio;
extern PIO             *pio2;
extern Tape            *tape;
extern Disk            *disk;
extern Timer           *timer;
extern Keyboard        *keyboard;
extern ModuleList      *module_list;
extern CMD             *cmd;
extern GDC             *gdc;
extern VIS             *vis;
extern Poly880         *poly880;

extern Z80_FDC         *fdc_z80;     
extern FloppyIO        *fdc_io;
extern FloppySharedMem *fdc_shmem;
extern Ports           *fdc_ports;
extern FDC             *fdc_fdc;
extern Sound           *sound;
extern byte_t           fdc_mem[];

extern int   kcemu_ui_scale;
extern int   kcemu_ui_debug;
extern int   kcemu_ui_fullscreen;
extern int   kcemu_ui_display_effect;
extern char *kcemu_modules;
extern char *kcemu_homedir;
extern char *kcemu_datadir;
extern char *kcemu_localedir;
extern char *kcemu_configfile;
extern char *kcemu_autostart_file;
extern char *kcemu_autostart_addr;

kc_type_t get_kc_type(void);
kc_variant_t get_kc_variant(void);
const char * get_kc_variant_name(void);

const char * kc_get_license(void);
const char * kc_get_warranty(void);
const char * kc_get_license_trailer(void);

#endif /* __kc_kc_h */
