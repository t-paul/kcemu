/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: kc.cc,v 1.25 2002/01/06 12:53:40 torsten_paul Exp $
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

#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>

#include "kc/config.h"
#include "kc/system.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#undef __CALL_MTRACE
#ifdef HAVE_MCHECK_H
#ifdef DBG_LEVEL
#if DBG_LEVEL > 1
#define __CALL_MTRACE 1
#include <mcheck.h>
#warning including mcheck.h
#endif
#endif
#endif

#include "kc/kc.h"
#include "kc/rc.h"
#include "kc/ctc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/wav.h"
#include "kc/tape.h"
#include "kc/disk.h"
#include "kc/timer.h"
#include "kc/module.h"
#include "kc/mod_list.h"
#include "kc/sh_mem.h"
#include "kc/disk_io.h"
#include "kc/fdc.h"
#include "kc/sound.h"
#include "kc/ctc_fdc.h"
#include "kc/z80_fdc.h"

#include "ui/log.h"
#include "cmd/cmd.h"
#include "fileio/load.h"
#include "libdisk/libdisk.h"

#include "kc/pio1.h"
#include "kc/ctc1.h"
#include "kc/ports1.h"
#include "kc/memory1.h"
#include "kc/keyb1.h"
#include "ui/gtk/ui_gtk1.h"

#include "kc/pio2.h"
#include "kc/memory2.h"

#include "kc/pio3.h"
#include "kc/ctc3.h"
#include "kc/ports3.h"
#include "kc/memory3.h"
#include "kc/keyb3.h"

#include "kc/pio4.h"
#include "kc/ports4.h"
#include "kc/memory4.h"

#ifdef HAVE_UI_GTK
#include "ui/gtk/ui_gtk3.h"
#include "ui/gtk/ui_gtk4.h"
#endif /* HAVE_UI_GTK */

#ifdef HAVE_UI_DUMMY
#include "ui/dummy/dummy3.h"
#include "ui/dummy/dummy4.h"
#endif /* HAVE_UI_DUMMY */

#ifdef HAVE_UI_DOS
#include "ui/dos/dos3.h"
#include "ui/dos/dos4.h"
#endif /* HAVE_UI_DOS */

Z80        *z80;
UI         *ui;
Ports      *ports;
Ports      *ports_fdc;
Module     *module;
Memory     *memory;
CTC        *ctc;
PIO        *pio;
PIO        *pio2;
Tape       *tape;
Disk       *disk;
WavPlayer  *wav;
Timer      *timer;
Keyboard   *keyboard;
ModuleList *module_list;
CMD        *cmd;
#if HAVE_LIBSDL
Sound      *sound;
#endif /* HAVE_LIBSDL */

Z80_FDC         *fdc_z80;
FloppyIO        *fdc_io;
FloppySharedMem *fdc_shmem;
Ports           *fdc_ports;
FDC             *fdc_fdc;
CTC             *fdc_ctc;


char      *kcemu_datadir;
char      *kcemu_localedir;
static kc_type_t  kcemu_kc_type;

void
usage(char *argv0)
{
  cout << endl 
       << "    ***  KC 85/4 Emulator  ***" << endl
       << "      (c) 1997  Torsten Paul  " << endl
       << endl
       << "usage: " << argv0 << " [-12347hdl]" << endl
       << endl
       << "  -1:             run in Z9001 / KC 85/1 mode" << endl
       << "  -2:             run in KC 85/2 mode" << endl
       << "  -3:             run in KC 85/3 mode" << endl
       << "  -4:             run in KC 85/4 mode (default)" << endl
       << "  -7:             run in KC 87 mode (= KC 85/1 with color expansion)" << endl
       << "  -h --help:      display help" << endl
       << "  -d --datadir:   set data directory (for ROM images)" << endl
       << "  -l --localedir: set locale directory" << endl
       << endl;
  exit(0);
}

kc_type_t
get_kc_type(void)
{
  return kcemu_kc_type;
}

int
main(int argc, char **argv)
{
  int c;
  int type;
  char *ptr;
  bool do_quit;
  const char *tmp;
  int option_index;
#ifdef HAVE_GETOPT_LONG
  static struct option long_options[] =
  {
    { "help", 0, 0, 'h' },
    { "datadir", 1, 0, 'd' },
    { "localedir", 1, 0, 'l' },
    { 0, 0, 0, 0 }
  };
#endif /* HAVE_GETOPT_LONG */
  PortGroup *portg;
  PortInterface *porti;
  Keyboard1 *k1;
  LOG *log;

#ifdef __CALL_MTRACE
  /*
   *  memory debugging with glibc goes like this:
   *
   *  1) run the program with MALLOC_CHECK_=1 ./kcemu to enable
   *     consistency checks (MALLOC_CHECK_=2 aborts immedately
   *     when encountering problems)
   *
   *  2) call mtrace() at the very beginning of the program and
   *     run it with MALLOC_TRACE=/tmp/mem.out ./kcemu to
   *     generate a memory trace that can be analyzed with
   *     mtrace ./kcemu /tmp/mem.out
   *
   *  The following mtrace is enababled if ./configure finds
   *  the mcheck.h header file, the mtrace() function and
   *  the debuglevel is set to a value greater than 1.
   */
  mtrace();
#endif /* __CALL_MTRACE */

  ptr = getenv("KCEMU_DATADIR");
  kcemu_datadir = (ptr) ? strdup(ptr) : strdup(DATADIR);
  ptr = getenv("KCEMU_LOCALEDIR");
  kcemu_localedir = (ptr) ? strdup(ptr) : strdup(LOCALEDIR);

#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif /* HAVE_SETLOCALE */
#ifdef ENABLE_NLS
  bindtextdomain(PACKAGE, kcemu_localedir);
  textdomain(PACKAGE);
#endif /* ENABLE_NLS */

  type = 0;
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      c = getopt_long(argc, argv, "12347d:l:",
                      long_options, &option_index);
#else
#ifdef HAVE_GETOPT
      c = getopt(argc, argv, "12347d:l:");
#else
#warning neither HAVE_GETOPT_LONG nor HAVE_GETOPT defined
#warning commandline parsing disabled!
      c = -1;
#endif /* HAVE_GETOPT */
#endif /* #ifdef HAVE_GETOPT_LONG */

      if (c == -1) break;

      switch (c)
        {
	case '1':
	  type = 1;
	  break;
        case '2':
          type = 2;
          break;
        case '3':
          type = 3;
          break;
        case '4':
          type = 4;
          break;
        case '7':
          type = 7;
          break;
        case 'd':
          free(kcemu_datadir);
          kcemu_datadir = strdup(optarg);
          break;
        case 'l':
          free(kcemu_localedir);
          kcemu_localedir = strdup(optarg);
          break;
        case ':':
        case '?':
        case 'h':
        default:
          usage(argv[0]);
          break;
        }
    }

#ifdef MEMORY_SLOW_ACCESS
  cout << "DEBUG: MEMORY_SLOW_ACCESS" << endl;
#endif /* MEMORY_SLOW_ACCESS */
#ifdef PROFILE_WINDOW
  cout << "DEBUG: PROFILE_WINDOW" << endl;
#endif /* PROFILE_WINDOW */

  fileio_init();
  libdisk_init();

  RC::init();
  if (type == 0)
    type = RC::instance()->get_int("Default KC Model", 4);

  switch (type)
    {
    case 1:  kcemu_kc_type = KC_TYPE_85_1; break;
    case 2:  kcemu_kc_type = KC_TYPE_85_2; break;
    case 3:  kcemu_kc_type = KC_TYPE_85_3; break;
    case 7:  kcemu_kc_type = KC_TYPE_87;   break;
    default: kcemu_kc_type = KC_TYPE_85_4; break;
    }

  do
    {
      cmd   = new CMD("*");
      z80   = new Z80;
      ports = new Ports;
      portg = ports->register_ports("-", 0, 0x100, new NullPort, 256);

      switch (kcemu_kc_type)
	{
	case KC_TYPE_87:
	case KC_TYPE_85_1:
	  memory   = new Memory1;
	  ui       = new UI_Gtk1;
	  pio      = new PIO1_1;
	  ctc      = new CTC1;
	  pio2     = new PIO1_2;
	  tape     = new Tape(500, 1000, 2000, 0);
	  disk     = NULL;
	  k1       = new Keyboard1;
	  
	  //tape->setAutoplay(false);
	  pio2->register_callback_A_in(k1);
	  pio2->register_callback_B_in(k1);
	  keyboard = k1;
	  break;
	case KC_TYPE_85_2:
	  memory   = new Memory2;
#ifdef HAVE_UI_GTK
	  ui       = new UI_Gtk3;
#endif /* HAVE_UI_GTK */
#ifdef HAVE_UI_DUMMY
	  ui       = new UI_Dummy3;
#endif /* HAVE_UI_DUMMY */
#ifdef HAVE_UI_DOS
	  ui       = new UI_Dos3;
#endif /* HAVE_UI_DOS */
	  porti    = new Ports3;
	  
	  pio      = new PIO2;
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  keyboard = new Keyboard3;
	  break;
	case KC_TYPE_85_3:
	  memory   = new Memory3;
#ifdef HAVE_UI_GTK
	  ui       = new UI_Gtk3;
#endif /* HAVE_UI_GTK */
#ifdef HAVE_UI_DUMMY
	  ui       = new UI_Dummy3;
#endif /* HAVE_UI_DUMMY */
#ifdef HAVE_UI_DOS
	  ui       = new UI_Dos3;
#endif /* HAVE_UI_DOS */
	  porti    = new Ports3;
	  
	  pio      = new PIO3;
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  keyboard = new Keyboard3;
	  break;
	case KC_TYPE_85_4:
	  memory   = new Memory4;
#ifdef HAVE_UI_GTK
	  ui       = new UI_Gtk4;
#endif /* HAVE_UI_GTK */
#ifdef HAVE_UI_DUMMY
	  ui       = new UI_Dummy4;
#endif /* HAVE_UI_DUMMY */
#ifdef HAVE_UI_DOS
	  ui       = new UI_Dos4;
#endif /* HAVE_UI_DOS */
	  porti    = new Ports4;
	  pio      = new PIO4;
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  keyboard = new Keyboard3;
	  break;
	}

#if HAVE_LIBSDL
      sound       = new Sound;
      if (RC::instance()->get_int("Enable Sound"))
	sound->init();
#endif /* HAVE_LIBSDL */
      wav         = new WavPlayer;
      timer       = new Timer;
      module      = new Module;
      module_list = new ModuleList;
      
      switch (kcemu_kc_type)
	{
	case KC_TYPE_87:
	case KC_TYPE_85_1:
	  portg = ports->register_ports("CTC",  0x80, 4, ctc,  10);
	  portg = ports->register_ports("PIO1", 0x88, 4, pio,  10);
	  portg = ports->register_ports("PIO2", 0x90, 4, pio2, 10);
	  break;
	case KC_TYPE_85_2:
	case KC_TYPE_85_3:
	  portg = ports->register_ports("Module", 0x80, 1, module, 10);
	  portg = ports->register_ports("PIO",    0x88, 4, pio,    10);
	  portg = ports->register_ports("CTC",    0x8c, 4, ctc,    10);
	  break;
	case KC_TYPE_85_4:
	  portg = ports->register_ports("Module", 0x80, 1, module, 10);
	  portg = ports->register_ports("Port84", 0x84, 1, porti,  10);
	  portg = ports->register_ports("Port86", 0x86, 1, porti,  10);
	  portg = ports->register_ports("PIO",    0x88, 4, pio,    10);
	  portg = ports->register_ports("CTC",    0x8c, 4, ctc,    10);
	  ctc->next(pio);
	  ctc->iei(1);
	  break;
	}

      if (RC::instance()->get_int("Floppy Disk Basis"))
        {
	  fdc_z80 = new Z80_FDC();
          fdc_ports = new Ports();
          fdc_io = new FloppyIO();
          fdc_shmem = new FloppySharedMem();
          fdc_shmem->set_memory(&fdc_mem[0xfc00]);
          fdc_fdc = new FDC();
	  fdc_ctc = new CTC_FDC();

          fdc_ports->register_ports("-", 0, 0x100, new NullPort(), 256);
          fdc_ports->register_ports("FDC", 0xf0, 12, fdc_fdc, 10);
	  fdc_ports->register_ports("CTC", 0xfc, 4, fdc_ctc, 10);

          portg = ports->register_ports("FloppyIO", 0xf4, 1, fdc_io, 10);
          portg = ports->register_ports("FloppySHMEM", 0xf0, 4, fdc_shmem, 10);

	  disk = new Disk();
        }
      
      log = new LOG();
      ui->init(&argc, &argv);
      module_list->init();

      if ((kcemu_kc_type == KC_TYPE_85_1) || (KC_TYPE_87))
	tape->power(true);

      //ctc->next(pio);
      //ctc->iei(1);
      
      tmp = RC::instance()->get_string("Tape File", "files.kct");
      if (tmp)
	{
	  ptr = (char *)malloc(strlen(kcemu_datadir) + strlen(tmp) + 2);
	  if (tmp[0] == '/')
	    strcpy(ptr, tmp);
	  else
	    {
	      strcpy(ptr, kcemu_datadir);
	      strcat(ptr, "/");
	      strcat(ptr, tmp);
	    }
	  tape->attach(ptr);
	  free(ptr);
	}

      do_quit = z80->run();
      
      // delete porti;
      
      delete module_list;
      delete module;
      delete keyboard;
      delete timer;
      delete tape;
      delete disk;
      delete pio;
      delete ctc;
      delete ports;
      delete ui;
      delete memory;
      delete z80;
    }
  while (0); // (!do_quit);
      
  free(kcemu_datadir);
  free(kcemu_localedir);
  
  return 0;
}
