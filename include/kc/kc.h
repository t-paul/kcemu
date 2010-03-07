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

#ifndef __kc_kc_h
#define __kc_kc_h

#include "kc/system.h"

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
class VDIP;
class KCNET;
class Poly880;
class DisplayC80;
class Plotter;

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
extern DisplayC80      *display_c80;
extern Plotter         *plotter;

extern Z80_FDC         *fdc_z80;     
extern FloppyIO        *fdc_io;
extern FloppySharedMem *fdc_shmem;
extern Ports           *fdc_ports;
extern FDC             *fdc_fdc;
extern Sound           *sound;
extern VDIP            *vdip;
extern KCNET           *kcnet;
extern byte_t           fdc_mem[];

extern int   kcemu_ui_scale;
extern int   kcemu_ui_debug;
extern int   kcemu_ui_fullscreen;
extern int   kcemu_ui_display_effect;
extern char *kcemu_modules;
extern char *kcemu_homedir;
extern char *kcemu_datadir;
extern char *kcemu_localedir;
extern char *kcemu_debug_output;
extern char *kcemu_autostart_file;
extern char *kcemu_autostart_addr;
extern char *kcemu_vdip_root;

const char * kc_get_license(void);
const char * kc_get_warranty(void);
const char * kc_get_license_trailer(void);

#endif /* __kc_kc_h */
