/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: kc.h,v 1.10 2001/04/14 15:14:16 tp Exp $
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

#include "kc/config.h"
#include "kc/system.h"

class Z80;
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
     
extern FloppyIO        *fdc_io;
extern FloppySharedMem *fdc_shmem;
extern Ports           *fdc_ports;
extern FDC             *fdc_fdc;
extern Sound           *sound;
extern byte_t           fdc_mem[];

extern char *kcemu_datadir;
extern char *kcemu_localedir;

#endif /* __kc_kc_h */
