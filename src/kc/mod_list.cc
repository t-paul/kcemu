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

/*
 *  Quelle: http://www.kc85emu.de/scans/fa0192/Bild5.jpg
 *  Original: Zeitschrift Funkamateur
 * 
 *  ---: D001 Grundgerät KC85/2
 *  ---: D001 Grundgerät KC85/3
 *  ---: D001 Grundgerät KC85/4
 *  ---: D002 Busdriver
 *  ---: D004 Floppy Disk Drive
 *  a7h: D004 Floppy Disk Basis (ROM)
 *  efh: M001 Digital IN/OUT
 *  eeh: M003 V24 (2 Kanäle)
 *  ---: M005 User (Leermodul)
 *  ---: M006 BASIC (für KC 85/2)
 *  ---: M007 Adapter (Herausführung der Anschlüsse an die Frontseite)
 *  ---: M008 Joystick
 *  e7h: M010 ADU1
 *  f6h: M011 64 KByte RAM
 *  fbh: M012 Software: Texor
 *  ---: M021 Joy + Centronics
 *  f4h: M022 Expander RAM (16 KByte)
 *  f7h: M025 User PROM (8 KByte)
 *  fbh: M026 Software: Forth
 *  fbh: M027 Software: Development
 *  e3h: M029 DAU1
 *  ---: M030 EPROMmer
 *  79h: M032 256 KByte RAM
 *  ---: M033 Software: TYPESTAR
 *  7bh: M035 1 MByte RAM
 *  7bh: M035 4 MByte RAM (intern 4 x 1MByte)
 *  ---: M036 128 KByte RAM
 *  ---: M040 USER PROM ? KByte
 *  ---: M053 RS 232
 *  ---: M125 USER ROM ? KByte
 */

/*
 *  Modulübersicht für KC85
 *  
 *  von Mario Leubner
 *  
 *  (KC-News 2/95 - http://www.iee.et.tu-dresden.de/~kc-club/)
 *  
 *  Modul  Kenn.  Steuerb.  Bezeichnung
 *  
 *  D001     -       -      Grundgerät KC85/2 (PIO 88-8B, CTC 8C-8F)
 *  D001     -       -      Grundgerät KC85/3 (PIO 88-8B, CTC 8C-8F)
 *  D001     -       -      Grundgerät KC85/4 (OUT 84-87, PIO 88-8B, CTC 8C-8F)
 *  D002     -       -      Busdriver für 4 Module (OUT 80 zur Modulsteuerung)
 *  D003*    -       -      Programmer (ähnl. D002, ROM-Module brennen?)
 *  D004    A7    xxAxxKxM  Floppy Disk Basis (Koppel-RAM F0-F3, OUT F4-F7)
 *  D004     -       -      Floppy Disk Drive (Laufwerk K5601, 5 1/4")
 *  D005                    Komfort-Tastatur für KC85/4 (mit EMR UB8830)
 *  
 *  M000*   01              Spezial f. KC85/3: autom. Start in Schacht 8 ab 4000H
 *  M001    EF    xxxxxxxM  Digital IN/OUT (CTC 00-03, PIO 04-07)
 *  M002*   DA    xxxxxxxM  PIO 3 (PIO 10-13, PIO 14-17)
 *  M003    EE    xxxxxxxM  V.24 (2 Kanäle, SIO 08-0B, CTC 0C-0F)
 *  M005     -       -      USER-Leermodul (freie Ports: C0-CF, Kennbytes C0-D7)
 *  M006    FC    AAxxxxxM  BASIC und CAOS 3.1 für KC85/2 (1 Block zu 16K ROM)
 *  M007     -       -      Adapter (Busverlängerung für Modulschacht)
 *  M008     -       -      Joystick (PIO 90-93)
 *  M009*   ED    xxxxxxxM  TLCM (Spracheingabe- und Datenkompressionsmodul)
 *  M010    E7    xxxxxxxM  ADU1: 4 Analogeingänge mit Multiplexer (PIO 40-43)
 *  M011    F6    AAxxxxWM  64K RAM (1 Block zu 64K, rotierbar)
 *  M012    FB    AAAxxxxM  Software: TEXOR + V.24-Treiber (1 Block ROM 8K)
 *  M021*    -       -      Joystick + Centronics (PIO 90-93)
 *  M022    F4    AAxxxxWM  Expander-RAM 16K (1 Block zu 16K)
 *  M024*   F5    AAxxxxWM  32K RAM (1 Block zu 32K)
 *  M025    F7    AAAxxxxM  USER PROM 8K (1 Block zu 8K, 4 Sockel für 2716)
 *  M026    FB    AAAxxxxM  Software: FORTH (1 Block ROM 8K)
 *  M027    FB    AAAxxxxM  Software: DEVELOPMENT+V.24-Treiber (1 Block ROM 8K)
 *  M029    E3    xxxxxxxM  DAU1: 2 Analogausgänge + 1 Relais (OUT 44..47)
 *  M030    D9              EPROMER ?
 *  M030    DB    AAAxxxxM  EPROMER für 2-64K (8K EPROM, PIO D0-D3, PIO D4-D7)
 *  M032    79    AxSSSSWM  256K segmented RAM (16 Blöcke je 16K, 4000 od. 8000)
 *  M033    01    AA0SxxxM  Software: TYPESTAR+RAMDOS (2 Blöcke ROM zu je 8K)
 *  M034    7A    ASSSSSWM  512K segmented RAM (32 Blöcke je 16K, 4000 od. 8000)
 *  M035    7B    SSSSSSWM  1M segmented RAM (64 Blöcke je 16K, Adr. 8000)
 *  M036    78    AxxSSSWM  128K segmented RAM (8 Blöcke je 16K, 4000 od. 8000)
 *  M040    F8    AAxxxxxM  USER PROM 16K (1 Block zu 16KByte)
 *  M045    70    AASSxxxM  32K segmented ROM (4 Blöcke je 8K)
 *  M046    71    AASSxSxM  64K segmented ROM (8 Blöcke je 8K)
 *  M047    72    AASSSSxM  128K segmented ROM (16 Blöcke je 8K)
 *  M048*   73    AASSSSxM  256K segmented ROM (16 Blöcke je 16K)
 *  M053    EE    xxxxxxxM  RS232: wie M003, jedoch 2. Kanal mit TTL-Pegel
 *  M120*   F0    AAAxxxWM  8K CMOS-RAM (1 Block zu 8K)
 *  M122*   F1    AAxxxxWM  16K CMOS-RAM (1 Block zu 16K)
 *  M124*   F2    AAxxxxWM  32K CMOS-RAM (1 Block zu 32K)
 *  
 *  Im Steuerbyte bedeuten:
 *  
 *      A (Basis-)Adresse
 *      K Kopplung ein/aus
 *      S Segmentnummer
 *      W Schreibfreigabe
 *      M Modulschaltzustand
 *      x nicht benutzt
 *  
 *  Verwendete Kennbytes
 *  
 *      01 Autostart-ROM
 *      7x segmented Memory
 *      Dx..Ex I/O-Module
 *      Fx Memory
 *  
 *  Mit '*' gekennzeichnete Module kamen offenbar nicht in den Handel! 
 */

#include <string>

#include "kc/system.h"
#include "kc/prefs/prefs.h"
#include "kc/prefs/strlist.h"

#include "kc/kc.h"
#include "kc/mod_dio.h"
#include "kc/mod_ram.h"
#include "kc/mod_ram1.h"
#include "kc/mod_r16.h"
#include "kc/mod_r64.h"
#include "kc/mod_cpm.h"
#include "kc/mod_ram8.h"
#include "kc/mod_64k.h"
#include "kc/mod_128k.h"
#include "kc/mod_256k.h"
#include "kc/mod_512k.h"
#include "kc/mod_1m.h"
#include "kc/mod_4m.h"
#include "kc/mod_rom.h"
#include "kc/mod_urom.h"
#include "kc/mod_auto.h"
#include "kc/mod_rom1.h"
#include "kc/mod_romb.h"
#include "kc/mod_raf.h"
#include "kc/mod_boot.h"
#include "kc/mod_192k.h"
#include "kc/mod_320k.h"
#include "kc/mod_fdc.h"
#include "kc/mod_gdc.h"
#include "kc/mod_rtc.h"
#include "kc/mod_ramf.h"
#include "kc/mod_disk.h"
#include "kc/mod_list.h"
#include "kc/mod_4131.h"
#include "kc/mod_m052.h"
#include "kc/mod_vdip.h"

#ifdef HAVE_V24
#include "kc/mod_v24.h"
#endif /* HAVE_V24 */

#ifdef HAVE_JOYSTICK
#include "kc/mod_js.h"
#endif

#include "fileio/load.h"

#include "ui/ui.h"
#include "ui/error.h"

#include "libdbg/dbg.h"

using namespace std;

ModuleList::ModuleList(void)
{
  ModuleInterface *m;

  string datadir(kcemu_datadir);
  string kc85_romdir = datadir + "/roms/kc85";
  string z9001_romdir = datadir + "/roms/z9001";

  kc_type_t kc_type = Preferences::instance()->get_kc_type();
  kc_variant_t kc_variant = Preferences::instance()->get_kc_variant();
  
  /*
   *  placeholder for a not present module ;-)
   */
  m = 0;
  _mod_list.push_back(new ModuleListEntry(_("<no module>"), m, KC_TYPE_ALL));

  /*
   *  RAM modules at 2400h-27ffh / 1k
   *                 2800h-2fffh / 2k
   *                 3000h-3fffh / 4k
   *
   *  (lc80)
   */
  m = new ModuleRAM8("RAM2400", 0x2400, 0x0400);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (1k/2400h)"), m, KC_TYPE_LC80));
  m = new ModuleRAM8("RAM2800", 0x2800, 0x0800);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (2k/2800h)"), m, KC_TYPE_LC80));
  m = new ModuleRAM8("RAM3000", 0x3000, 0x1000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (4k/3000h)"), m, KC_TYPE_LC80));

  /*
   *  RAM modules at 0800h-0fffh / 2k
   *                 1000h-1fffh / 4k
   *                 2000h-3fffh / 8k
   *
   *  (vcs80)
   */
  m = new ModuleRAM8("RAM0800", 0x0800, 0x0800);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (2k/0800h)"), m, KC_TYPE_VCS80));
  m = new ModuleRAM8("RAM1000", 0x1000, 0x1000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (4k/1000h)"), m, KC_TYPE_VCS80));
  m = new ModuleRAM8("RAM2000", 0x2000, 0x2000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (8k/2000h)"), m, KC_TYPE_VCS80));

  /*
   *  RAM modules at 4000h-7fffh / 16k
   *                 8000h-ffffh / 32k
   *
   *  (lc80 + vcs80)
   */
  m = new ModuleRAM8("RAM4000", 0x4000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/4000h)"), m, (kc_type_t)(KC_TYPE_LC80 | KC_TYPE_VCS80)));
  m = new ModuleRAM8("RAM8000", 0x8000, 0x8000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (32k/8000h)"), m, (kc_type_t)(KC_TYPE_LC80 | KC_TYPE_VCS80)));

  /*
   *  256k RAM floppy (Z1013)
   */
  m = new ModuleRAMFloppy("RAMFLOPPY");
  _mod_list.push_back(new ModuleListEntry(_("256k RAM Floppy"), m, KC_TYPE_Z1013));

  /*
   *  FDC (schneider) (Z1013)
   */
  m = new ModuleFDC("FDC_SCHNEIDER", ModuleFDC::FDC_INTERFACE_SCHNEIDER);
  _mod_list.push_back(new ModuleListEntry(_("FDC (Schneider)"), m, KC_TYPE_Z1013));

  /*
   *  FDC (kramer) (Z1013)
   */
  m = new ModuleFDC("FDC_KRAMER", ModuleFDC::FDC_INTERFACE_KRAMER);
  _mod_list.push_back(new ModuleListEntry(_("FDC (Kramer)"), m, KC_TYPE_Z1013));

  /*
   *  GDC (Z1013)
   */
  m = new ModuleGDC("GDC");
  _mod_list.push_back(new ModuleListEntry(_("GDC 82720"), m, KC_TYPE_Z1013));

  /*
   *  RTC (port 20h-2fh) (Z1013)
   */
  m = new ModuleRTC("RTC20", 0x20);
  _mod_list.push_back(new ModuleListEntry(_("RTC (port 20h-2fh)"), m, KC_TYPE_Z1013));

  /*
   *  RTC (port 70h-7fh) (Z1013)
   */
  m = new ModuleRTC("RTC70", 0x70);
  _mod_list.push_back(new ModuleListEntry(_("RTC (port 70h-7fh)"), m, KC_TYPE_Z1013));

  /*
   *  basic (kc85/1)
   */
  string z9001_basic_rom = z9001_romdir + "/basic_c0.851";
  m = new ModuleROM1(z9001_basic_rom.c_str(), "BASIC", 0xc000, 0x2800);
  _mod_list.push_back(new ModuleListEntry(_("Basic Module (c000h-e7ffh)"), m, KC_TYPE_85_1));

  /*
   *  plotter module (kc85/1, kc87.10, kc87.11)
   *
   *  The ROM of this module is identical with the internal ROM BASIC of the KC87.21
   *  that already includes the extensions to interface to an external plotter unit
   *  like the XY4131. To actually use the plotter an additional driver is needed
   *  though.
   */
  string z9001_plotter_rom = z9001_romdir + "/basic_c0.87b";
  m = new ModuleROM1(z9001_plotter_rom.c_str(), "PLOTTER", 0xc000, 0x2800);
  _mod_list.push_back(new ModuleListEntry(_("Plotter Module (c000h-e7ffh)"), m, KC_TYPE_85_1));

  /*
   *  RAM modules 16k at 4000h and 8000h (kc85/1)
   */
  m = new ModuleRAM1("RAM4", 0x4000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/4000h)"), m, KC_TYPE_85_1_CLASS));
  m = new ModuleRAM1("RAM8", 0x8000, 0x4000);
  _mod_list.push_back(new ModuleListEntry(_("RAM Module (16k/8000h)"), m, KC_TYPE_85_1_CLASS));

  bool romdi_handling = (Preferences::instance()->get_kc_type() == KC_TYPE_87);

  /*
   *  ZM30 (kc85/1)
   */
  string z9001_zm30_rom = z9001_romdir + "/zm30__c0.851";
  m = new ModuleROM1(z9001_zm30_rom.c_str(), "ZM30", 0xc000, 0x0d00, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("ZM30 (c000h-ccffh)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  edas (kc85/1)
   */
  string z9001_edas_rom = z9001_romdir + "/edas__c0.851";
  m = new ModuleROM1(z9001_edas_rom.c_str(), "EDAS", 0xc000, 0x2800, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("EDAS (c000h-e7ffh)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  idas (kc85/1)
   */
  string z9001_idas_rom = z9001_romdir + "/idas__c0.851";
  m = new ModuleROM1(z9001_idas_rom.c_str(), "IDAS", 0xc000, 0x2800, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("IDAS / ZM (c000h-e7ffh)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  bitex (kc85/1)
   */
  string z9001_bitex_rom = z9001_romdir + "/bitex_c0.851";
  m = new ModuleROM1(z9001_bitex_rom.c_str(), "BITEX", 0xc000, 0x1800, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("BITEX (c000h-d7ffh)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  zsid (kc85/1)
   */
  /*
  string z9001_zsid_rom = z9001_romdir + "/zsid__c0.851";
  m = new ModuleROM1(z9001_zsid_rom.c_str(), "ZSID", 0xc000, 0x2800, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("ZSID (c000h-e7ffh)"), m, KC_TYPE_85_1_CLASS));
  */

  /*
   *  r80 (kc85/1)
   */
  /*
  string z9001_r80_rom = z9001_romdir + "/r80___c0.851";
  m = new ModuleROM1(z9001_r80_rom.c_str(), "R80", 0xc000, 0x1c00, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("R80 (c000h-dbffh)"), m, KC_TYPE_85_1_CLASS));
  */

  /*
   *  kc-caos (kc85/1)
   */
  string z9001_kc_caos_rom = z9001_romdir + "/kc_caos.rom";
  m = new ModuleROM1(z9001_kc_caos_rom.c_str(), "KC-CAOS", 0x8000, 0x1000);
  _mod_list.push_back(new ModuleListEntry(_("KC-CAOS 3.1 (8000h-8fffh)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  192 KByte RAM/EPROM module (kc85/1)
   */
  string z9001_192k_d2_rom = z9001_romdir + "/192k__d2.851";
  string z9001_192k_d3_rom = z9001_romdir + "/192k__d3.851";
  string z9001_192k_d5_rom = z9001_romdir + "/192k__d5.851";

  m = new Module192k(z9001_192k_d2_rom.c_str(),
		     z9001_192k_d3_rom.c_str(),
		     z9001_192k_d5_rom.c_str(),
		     "192k");
  _mod_list.push_back(new ModuleListEntry(_("192 KByte RAM/EPROM"), m, KC_TYPE_85_1_CLASS));

  /*
   *  KC-Pascal module (kc85/1)
   */
  string z9001_kcpascal_rom = z9001_romdir + "/kcpascal.851";
  m = new ModuleROM1(z9001_kcpascal_rom.c_str(), "KC-Pascal", 0x4000, 0x8000);
  _mod_list.push_back(new ModuleListEntry(_("KC-Pascal Module"), m, KC_TYPE_85_1_CLASS));

  /*
   *  2,5Mb ROM Modul (kc85/1)
   */
  string z9001_mega_rom = z9001_romdir + "/mega_rom.851";
  m = new Module320k(z9001_mega_rom.c_str(), "Mega-ROM");
  _mod_list.push_back(new ModuleListEntry(_("Mega-ROM Module"), m, KC_TYPE_85_1_CLASS));

  /*
   *  128 KByte ROM bank module (kc85/1)
   */
  string z9001_rombank_rom = z9001_romdir + "/rom_bank.851";
  m = new ModuleROMBank(z9001_rombank_rom.c_str(), "ROMBANK");
  _mod_list.push_back(new ModuleListEntry(_("128k ROM Bank"), m, KC_TYPE_85_1_CLASS));

  /*
   *  IRM Expansion for color display (kc85/1)
   *
   *  FIXME: check whether to enable the color expansion by default shouldn't
   *  FIXME: go here!
   */
  m = new ModuleRAM1("IRMX", 0xe800, 0x0800);
  _color_expansion = new ModuleListEntry(_("IRM Color Expansion"), m, KC_TYPE_NONE);
  _mod_list.push_back(_color_expansion);
  _init_color_expansion = 0;
  
  if (kc_type == KC_TYPE_85_1)
    if (kc_variant == KC_VARIANT_85_1_11)
      _init_color_expansion = _color_expansion;
  if (kc_type == KC_TYPE_87)
    if ((kc_variant != KC_VARIANT_87_10) && (kc_variant != KC_VARIANT_87_20))
      _init_color_expansion = _color_expansion;

  /*
   *  Plotter-Anschluss
   */
  m = new ModuleXY4131("XY4131");
  _mod_list.push_back(new ModuleListEntry(_("Plotter XY4131"), m, KC_TYPE_85_1_CLASS));

  /*
   *  CPM-Z9 boot rom module (kc85/1)
   */
  string z9001_z9_boot_rom = z9001_romdir + "/cpmz9_c0.851";
  m = new ModuleBOOT(z9001_z9_boot_rom.c_str(), "CPM-Z9-BOOT", 0xc000, 0x0800, romdi_handling);
  _mod_list.push_back(new ModuleListEntry(_("CPM-Z9 BOOT (c000h-c7ffh)"), m, KC_TYPE_85_1_CLASS));

  /*
   *  CPM-Z9 floppy module (kc85/1)
   */
  m = new ModuleCPMZ9("CPM-Z9-FLOPPY");
  _mod_list.push_back(new ModuleListEntry(_("CPM-Z9 Floppy"), m, KC_TYPE_85_1_CLASS));

  /*
   *  CPM-Z9 64k ram module
   */
  m = new ModuleRAM64("CPM-Z9-RAM64");
  _mod_list.push_back(new ModuleListEntry(_("CPM-Z9 64k RAM"), m, KC_TYPE_85_1_CLASS));

  /*
   *  RAF (ram floppy module for KC85/1 and A5105 at port 20h/24h)
   */
  m = new ModuleRAF("RAF-4MB-20h", 0x20, 0x400000);
  _mod_list.push_back(new ModuleListEntry(_("RAF 4MB (port 20h)"), m, (kc_type_t)(KC_TYPE_85_1_CLASS | KC_TYPE_A5105)));

  m = new ModuleRAF("RAF-4MB-24h", 0x24, 0x400000);
  _mod_list.push_back(new ModuleListEntry(_("RAF 4MB (port 24h)"), m, (kc_type_t)(KC_TYPE_85_1_CLASS | KC_TYPE_A5105)));

  /*
   *  Digital I/O (kc85/2-4)
   */
  m = new ModuleDIO("M001", 0xef);
  _mod_list.push_back(new ModuleListEntry(_("M001: Digital In / Out"), m, KC_TYPE_85_2_CLASS));

  /*
   *  V24 module
   */
#ifdef HAVE_V24
    m = new ModuleV24("M003", 0xee);
    _mod_list.push_back(new ModuleListEntry(_("M003: V24"), m, KC_TYPE_85_2_CLASS));
#endif /* HAVE_V24 */

  /*
   *  basic (kc85/2) (this is actually 16k and includes a new system rom!)
   */
  string kc85_m006_rom = kc85_romdir + "/m006.rom";
  m = new ModuleROM(kc85_m006_rom.c_str(), "M006", 0x4000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M006: Basic"), m, (kc_type_t)(KC_TYPE_85_2 | KC_TYPE_85_3)));

  /*
   *  Joystick module (kc85/2-4)
   */
#ifdef HAVE_JOYSTICK
  m = new ModuleJoystick("M008", 0xff);
  _mod_list.push_back(new ModuleListEntry(_("M008: Joystick"), m, KC_TYPE_85_2_CLASS));
#endif /* HAVE_JOYSTICK */

  /*
   *  RAM module 64k (kc85/2-4)
   */
  m = new Module64k("M011", 0xf6);
  _mod_list.push_back(new ModuleListEntry(_("M011: 64k RAM"), m, KC_TYPE_85_2_CLASS));
  
  /*
   *  texor
   */
  string kc85_m012_rom = kc85_romdir + "/m012.rom";
  m = new ModuleROM(kc85_m012_rom.c_str(), "M012", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M012: Texor"), m, KC_TYPE_85_2_CLASS));

  /*
   *  RAM module 16k (kc85/2-4)
   */
  m = new ModuleRAM("M022", 0xf4);
  _mod_list.push_back(new ModuleListEntry(_("M022: Expander RAM (16k)"), m, KC_TYPE_85_2_CLASS));

  /*
   *  forth
   */
  string kc85_m026_rom = kc85_romdir + "/m026.rom";
  m = new ModuleROM(kc85_m026_rom.c_str(), "M026", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M026: Forth"), m, KC_TYPE_85_2_CLASS));

  /*
   *  development
   */
  string kc85_m027_rom = kc85_romdir + "/m027.rom";
  m = new ModuleROM(kc85_m027_rom.c_str(), "M027", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M027: Development"), m, KC_TYPE_85_2_CLASS));

  /*
   *  RAM module 256k (kc85/2-4)
   */
  m = new Module256k("M032", 0x79);
  _mod_list.push_back(new ModuleListEntry(_("M032: 256k Segmented RAM"), m, KC_TYPE_85_2_CLASS));

  /*
   *  typestar + ramdos
   */
  string kc85_m033_rom = kc85_romdir + "/m033.rom";
  m = new ModuleSegmentedROM16k(kc85_m033_rom.c_str(), "M033", 0x01);
  _mod_list.push_back(new ModuleListEntry(_("M033: TypeStar + RAMDOS"), m, KC_TYPE_85_2_CLASS));

  /*
   *  RAM module 512k (kc85/2-4)
   */
  m = new Module512k("M034", 0x7a);
  _mod_list.push_back(new ModuleListEntry(_("M034: 512k Segmented RAM"), m, KC_TYPE_85_2_CLASS));

  /*
   *  RAM module 1M (kc85/2-4)
   */
  m = new Module1M("M035", 0x7b);
  _mod_list.push_back(new ModuleListEntry(_("M035: 1M Segmented RAM"), m, KC_TYPE_85_2_CLASS));

  /*
   *  RAM module 4M (kc85/2-4)
   */
  m = new Module4M("M035x4", 0x7b);
  _mod_list.push_back(new ModuleListEntry(_("M035x4: 4M RAM"), m, KC_TYPE_85_2_CLASS));

  /*
   *  RAM module 128k (kc85/2-4)
   */
  m = new Module128k("M036", 0x78);
  _mod_list.push_back(new ModuleListEntry(_("M036: 128k Segmented RAM"), m, KC_TYPE_85_2_CLASS));

  /*
   *  M052 network + usb (kc85/2-4)
   */
  string kc85_m052_rom = kc85_romdir + "/m052.rom";
  m = new ModuleNetworkUSB(kc85_m052_rom.c_str(), "M052", 0xfd);
  _mod_list.push_back(new ModuleListEntry(_("M052: Network + USB"), m, KC_TYPE_85_2_CLASS));

  /*
   *  wordpro ROM version for kc85/3
   */
  string kc85_m900_rom = kc85_romdir + "/m900.rom";
  m = new ModuleROM(kc85_m900_rom.c_str(), "M900", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M900: WordPro '86 (KC85/3)"), m, KC_TYPE_85_3));

  /*
   *  wordpro ROM version for kc85/4
   */
  string kc85_m901_rom = kc85_romdir + "/m901.rom";
  m = new ModuleROM(kc85_m901_rom.c_str(), "M901", 0x2000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("M901: WordPro '86 (KC85/4)"), m, KC_TYPE_85_4));

  /*
   *  vinculum usb + driver rom
   */
  m = new ModuleVDIP("VDIP", 0xef);
  _mod_list.push_back(new ModuleListEntry(_("VDIP: Vinculum USB"), m, KC_TYPE_85_2_CLASS));

  string kc85_vdip1_rom = kc85_romdir + "/vdip12.rom";
  m = new ModuleROM(kc85_vdip1_rom.c_str(), "VDIP1", 0x0800, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("VDIP1: Vinculum USB ROM V1.2"), m, KC_TYPE_85_2_CLASS));

  string kc85_vdip2_rom = kc85_romdir + "/vdip21.rom";
  m = new ModuleROM(kc85_vdip2_rom.c_str(), "VDIP2", 0x1000, 0xfb);
  _mod_list.push_back(new ModuleListEntry(_("VDIP2: Vinculum USB ROM V2.1"), m, KC_TYPE_85_2_CLASS));

  int d004_enabled = Preferences::instance()->get_int_value("d004", 0);

  _init_floppy_basis_f8 = 0;
  _init_floppy_basis_fc = 0;
  if ((kc_type & KC_TYPE_85_2_CLASS) && d004_enabled)
    {
     if (Preferences::instance()->get_int_value("d004_f8", 0))
       {
          /*
           *  Floppy Disk Basis Extension ROM at slot F8, rom is defined by profile setting
           */
          m = new ModuleDisk(SystemROM::ROM_KEY_D004_F8, "Floppy Disk Basis F8", 0xa7, 0xf8);
          ModuleListEntry *mod_floppy_f8 = new ModuleListEntry(_("Floppy Disk Basis F8"), m, KC_TYPE_NONE);
          _mod_list.push_back(mod_floppy_f8);
          _init_floppy_basis_f8 = mod_floppy_f8;
       }

     /*
       *  Floppy Disk Basis ROM at slot FC, rom is defined by profile setting
       */
      m = new ModuleDisk(SystemROM::ROM_KEY_D004_FC, "Floppy Disk Basis FC", 0xa7, 0xfc);
      ModuleListEntry *mod_floppy_fc = new ModuleListEntry(_("Floppy Disk Basis FC"), m, KC_TYPE_NONE);
      _mod_list.push_back(mod_floppy_fc);
      _init_floppy_basis_fc = mod_floppy_fc;
    }

  _nr_of_bd = Preferences::instance()->get_int_value("busdrivers", 0);
  if (_nr_of_bd < 0)
    _nr_of_bd = 0;
  if (_nr_of_bd > MAX_BD)
    _nr_of_bd = MAX_BD;
  if (kc_type & KC_TYPE_85_1_CLASS)
    _nr_of_bd = 0;
  if (kc_type & KC_TYPE_LC80)
    _nr_of_bd = 1;

  int cnt;
  switch (kc_type)
    {
    case KC_TYPE_85_1:
    case KC_TYPE_87:
    case KC_TYPE_A5105:
      cnt = 4;
      break;
    case KC_TYPE_LC80:
      cnt = 6;
      break;
    case KC_TYPE_Z1013:
      cnt = 8;
      break;
    case KC_TYPE_85_2:
    case KC_TYPE_85_3:
    case KC_TYPE_85_4:
    case KC_TYPE_85_5:
      cnt = 4 * _nr_of_bd + 2;
      break;
    default:
      cnt = 0;
      break;
    }

  add_custom_modules();

  init_modules(cnt);
}

void
ModuleList::add_custom_modules(void)
{
#if 0
  for (int a = 0;a < 10;a++)
    {
      const char *name = RC::instance()->get_string_i(a, "Custom ROM Module");
      const char *text = RC::instance()->get_string_i(a, "Custom ROM Module (Name)");
      int id = RC::instance()->get_int_i(a, "Custom ROM Module (Id)");
      const char *file = RC::instance()->get_string_i(a, "Custom ROM Module (File)");
      int size = RC::instance()->get_int_i(a, "Custom ROM Module (Size)");

      if ((name == NULL) || (file == NULL))
	continue;

      if (id == 0)
	id = 0xfb;

      size &= 0xfc00;

      if (size == 0)
	size = 0x2000;

      if (size > 0x4000)
	size = 0x4000;

      if (text == NULL)
	text = name;

      ModuleInterface *m = new ModuleUserROM(file, name, size, id);
      ModuleListEntry *entry = new ModuleListEntry(text, m, KC_TYPE_85_2_CLASS);
      _mod_list.push_back(entry);
    }
#endif
}

void
ModuleList::init_modules(int max_modules)
{
  for (int a = 0;a < 4 * MAX_BD + 2;a++)
    _init_mod[a] = NULL;

  int mode = 1;
  const char *ptr = kcemu_modules;
  if (ptr)
    {
      if (*ptr == '+')
        ptr++;
      else
	mode = 0;
    }

  int idx = init_modules_autostart(0);
  if (mode == 1)
    idx = init_modules_configfile(idx, max_modules);

  init_modules_commandline(idx, max_modules, ptr);
}

int
ModuleList::init_modules_autostart(int idx)
{
  if (!(Preferences::instance()->get_kc_type() & KC_TYPE_85_2_CLASS))
    return idx;

  if (kcemu_autostart_file == NULL)
    return idx;

  DBG(2, form("KCemu/ModuleAutoStart/page",
	      "ModuleAutoStart: trying file '%s'\n",
	      kcemu_autostart_file));

  fileio_prop_t *ptr, *prop;
  if (fileio_load_file(kcemu_autostart_file, &prop) != 0)
    return idx;

  int size = 0x10000;
  byte_t *rom = new byte_t[size];
  memset(rom, 0xff, size);

  byte_t init[0x200];
  memset(init, 0xff, 0x200);
  string datadir = string(kcemu_datadir);
  string autostart = datadir + "/lib/z80/kc853.bin";
  FILE *f = fopen(autostart.c_str(), "rb");
  if (f != NULL)
    {
      fread(init, 1, 0x200, f);
      fclose(f);
    }

  int page = 0;
  int info_idx = 0x10;
  byte_t *rom_ptr = rom + 0x200;
  for (ptr = prop;ptr != NULL;ptr = ptr->next)
    {
      DBG(2, form("KCemu/ModuleAutoStart/page",
		  "ModuleAutoStart: load = %04x, size = %ld, start = %04x\n",
		  ptr->load_addr, ptr->size, ptr->start_addr));

      byte_t *img = new byte_t[ptr->size];
      int image_size = fileio_get_image(ptr, img);

      byte_t *img_ptr = img;
      int load_addr = ptr->load_addr;
      while (image_size > 0)
	{
	  if (page == 3)
	    break;

	  page++;

	  int len = image_size < 0x3e00 ? image_size : 0x3e00;
	  
	  memcpy(rom_ptr, img_ptr, len);

	  if (info_idx == 0x10)
	    {
	      int start_addr = ptr->start_addr;
	      if (kcemu_autostart_addr != NULL)
		{
		  start_addr = strtoul(kcemu_autostart_addr, NULL, 0) & 0xffff;
		  DBG(2, form("KCemu/ModuleAutoStart/page",
			      "ModuleAutoStart: overriding start address with %04xh\n",
			      start_addr));
		}

	      memcpy(&init[2], ptr->name, 12);
	      init[info_idx++] = start_addr & 0xff;
	      init[info_idx++] = start_addr >> 8;
	    }

	  init[info_idx++] = load_addr & 0xff;
	  init[info_idx++] = load_addr >> 8;
	  init[info_idx++] = len & 0xff;
	  init[info_idx++] = len >> 8;

	  init[info_idx++] = len > 0x1000;
	  init[info_idx++] = 0;

	  rom_ptr += 0x4000;
	  img_ptr += 0x3e00;
	  load_addr += 0x3e00;
	  image_size -= 0x3e00;
	}

      delete[] img;
    }

  for (int a = 0;a < 4;a++)
    memcpy(rom + a * 0x4000, init, 0x200);

  ModuleInterface *m = new ModuleAutoStart(&rom[0], "Autostart", 0x10000, 0x01);
  _mod_list.push_back(new ModuleListEntry(_("Autostart"), m, KC_TYPE_85_2_CLASS));

  _init_mod[idx++] = strdup("Autostart");

  fileio_free_prop(&prop);

  return idx;
}

int
ModuleList::init_modules_configfile(int idx, int max_modules)
{
  StringList list(Preferences::instance()->get_string_value("modules", ""));
  
  int a = 0;
  for (StringList::iterator it = list.begin();(it != list.end()) && (a < max_modules);it++) {
	_init_mod[idx++] = strdup((*it).c_str());
        a++;
  }

  return idx;
}

int
ModuleList::init_modules_commandline(int idx, int max_modules, const char *param)
{
  if (param == NULL)
    return idx;

  char *buffer = new char[strlen(param) + 1];
  strcpy(buffer, param);

  char *ptr = strtok(buffer, ",");
  while (ptr != 0)
    {
      _init_mod[idx++] = strdup(ptr);
      ptr = strtok(0, ",");
    }

  delete[] buffer;

  return idx;
}

ModuleList::~ModuleList(void)
{
  ModuleList::iterator it;

  for (it = module_list->begin();it != module_list->end();it++)
    {
      ModuleListEntry *entry = *it;
      delete entry->get_mod();
      delete entry;
    }

  for (int a = 0;a < 4 * MAX_BD + 2;a++)
    if (_init_mod[a])
      free(_init_mod[a]);

  /* inserted clones of modules are freed in module.cc */
}

void
ModuleList::init(void)
{
  int a;
  ModuleInterface *m;
  ModuleList::iterator it;

  for (a = 0;a < 4 * MAX_BD + 2;a++)
    {
      if (!_init_mod[a])
	continue;

      for (it = module_list->begin();it != module_list->end();it++)
        {
          m = (*it)->get_mod();
          if (!m)
	    continue;
          if (strcmp(m->get_name(), _init_mod[a]) == 0)
            insert(a, *it);
        }
    }
  if (_init_floppy_basis_f8)
    {
      /*
       *  floppy disk basis extension ROM is always in slot f8h!
       */
      insert(60, _init_floppy_basis_f8);
    }

  if (_init_floppy_basis_fc)
    {
      /*
       *  floppy disk basis ROM is always in slot fch!
       */
      insert(61, _init_floppy_basis_fc);
    }

  if (_init_color_expansion)
    {
      /*
       *  this is set if the emulator runs in KC 87 mode which means
       *  the color expansion is always present
       *  the KC 87 doesn't support the module slot notation like
       *  the KC 85/2 class we just insert it at slot fch like
       *  the floppy disk interface for KC 85/2 which is never available
       *  in KC 87 mode.
       */
      insert(61, _init_color_expansion);
    }
}

void
ModuleList::insert(int slot, ModuleListEntry *entry)
{
  ModuleInterface *m = NULL;
  
  /*
   *  the module object is deleted in module->insert()
   *  or module->remove()!
   */
  if (entry != NULL)
    m = entry->get_mod();

  if (m)
    {
      ModuleInterface *mod = m->clone();
      if (!mod)
	return;

      if (mod->is_valid())
	{
	  module->insert(slot, mod);
	}
      else
	{
	  const char *error_text = mod->get_error_text();
	  if (error_text != NULL)
	    Error::instance()->info(error_text);
	  m = NULL;
	  delete mod;
	}
    }
  else
    {
      module->remove(slot);
    }

  ui->getModuleInterface()->insert(slot, m);
  ui->getModuleInterface()->activate(slot, 0);
}

ModuleListEntry *
ModuleList::get_color_expansion()
{
  return _color_expansion;
}
