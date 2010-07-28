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

#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include "kc/kc.h"
#include "kc/system.h"

#include "kc/roms.h"

using namespace std;

const RomRegistry * RomRegistry::_instance = new RomRegistry();

const char *
RomRegistryEntry::get_id() const
{
  return _id;
}

const char *
RomRegistryEntry::get_name() const
{
  return _name;
}

const char *
RomRegistryEntry::get_filename() const
{
  return _filename;
}

const char *
RomRegistryEntry::get_directory() const
{
  return _directory;
}

RomRegistry::RomRegistry(void)
{
  add_rom(_("System-ROM 1"),
          "5c33139db9f59e50da5c76729752f8e653ae34ae",
          "k1505_00.rom", "roms/a5105",
          0x8000);
  add_rom(_("System-ROM 2"),
          "7e5b587c59676e8549561117ea0b70234f439a94",
          "k1505_80.rom", "roms/a5105",
          0x2000);
  add_rom(_("DSE-ROM 1"),
          "9a7bbe6f0d180dd513c7854f441cee986c8d9765",
          "k5651_40.rom", "roms/a5105",
          0x2000);
  add_rom(_("DSE-ROM 2"),
          "7c16226be6c4c71013e8008fba9d2e9c5640b6a7",
          "k5651_60.rom", "roms/a5105",
          0x2000);
  add_rom(_("System-ROM"),
          "14f72cb73a4068b7a5d763cc0e254639c251ce2e",
          "monitor.rom", "roms/c80",
          0x400);
  add_rom(_("KC-BASIC"),
          "c2e3af55c79e049e811607364f88c703b0285e2e",
          "basic_c0.853", "roms/kc85",
          0x2000);
  add_rom(_("KC-BASIC (EDAS/TEMO/FORTH)"),
          "be2c68a5b461014c57e33a127c3ffb32b0ff2346",
          "basic_c0.855", "roms/kc85",
          0x8000);
  add_rom(_("KC-BASIC (EDAS/TEMO/USB)"),
          "dcd3b44a4ef116c5981fb932dff1aa5f57735f42",
          "basicusb.855", "roms/kc85",
          0x8000);
  add_rom(_("CAOS 3.3"),
          "20447d27c9aa41a1c7a3d6ad0699edb06a207aa6",
          "caos33.853", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 3.4"),
          "ee9f8e7427b9225ae2cecbcfb625d629ab6a601d",
          "caos34.853", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 4.1 (C)"),
          "acd998e3d9e8f592cd884aafc8ac4d291e40e097",
          "caos41c.854", "roms/kc85",
          0x1000);
  add_rom(_("CAOS 4.1 (E)"),
          "e19819fb477dcb742a13729a9bf5943d63abe863",
          "caos41e.854", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 4.3 (C)"),
          "5342be5104206d15e7471b094c7749a8a3d708ad",
          "caos43c.855", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 4.3 (E)"),
          "521ac2fbded4148220f8af2d5a5ab99634364079",
          "caos43e.855", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 4.2 (C)"),
          "774fc2496a59b77c7c392eb5aa46420e7722797e",
          "caos__c0.854", "roms/kc85",
          0x1000);
  add_rom(_("CAOS 4.4 (C)"),
          "2cf8023ee71ca50b92f9f151b7519f59727d1c79",
          "caos__c0.855", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 2.2"),
          "568dd59bfad4c604ba36bc05b094fc598a642f85",
          "caos__e0.852", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 3.1"),
          "efd002fc9146116936e6e6be0366d2afca33c1ab",
          "caos__e0.853", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 4.2 (E)"),
          "4300f7ff813c1fb2d5c928dbbf1c9e1fe52a9577",
          "caos__e0.854", "roms/kc85",
          0x2000);
  add_rom(_("CAOS 4.4 (E)"),
          "53ba4394d96e287ff8af01322af1e9879d4e77c4",
          "caos__e0.855", "roms/kc85",
          0x2000);
  add_rom(_("D004 Version 2.0 (02.01.1989)"),
          "66f476de78fb474d9ac61c6eaffce3354fd66776",
          "floppy20.rom", "roms/kc85",
          0x2000);
  add_rom(_("D004 Version 3.0 (07.12.1997)"),
          "5582b2541a34a90c7a9516a6a222d4961fc54fcf",
          "floppy30.rom", "roms/kc85",
          0x2000);
  add_rom(_("D004 Version 3.1 (22.01.2003)"),
          "38b3164dce23573375fc0237f348d9a699fc6f9f",
          "floppy31.rom", "roms/kc85",
          0x2000);
  add_rom(_("D004 Version 3.2 (28.10.2008)"),
          "8232adb5e5f0b25b52f9873cff14831da3a0398a",
          "floppy32.rom", "roms/kc85",
          0x2000);
  add_rom(_("HC-900 CAOS"),
          "242a777788c774c5f764313361b1e0a65139ab32",
          "hc900.852", "roms/kc85",
          0x2000);
  add_rom(_(""),
          "68d8c0d19e9dab213b0d27506728a8badebd734b",
          "m006.rom", "roms/kc85",
          0x4000);
  add_rom(_(""),
          "b1fe2880dab227a277243161377a4e41f1d3f67e",
          "m012.rom", "roms/kc85",
          0x2000);
  add_rom(_(""),
          "6e95a5a16e555c1e768b27dfa5216a2b648c2297",
          "m026.rom", "roms/kc85",
          0x2000);
  add_rom(_(""),
          "04fe21279ebcf07e2dae4b9a462a43023b117e6a",
          "m027.rom", "roms/kc85",
          0x2000);
  add_rom(_(""),
          "95dec095fb23fcd19ab61d37a643b179b9e513a1",
          "m033.rom", "roms/kc85",
          0x4000);
  add_rom(_(""),
          "81788f10f934ad9726cdbe3f200e3157a628935c",
          "m052.rom", "roms/kc85",
          0x8000);
  add_rom(_(""),
          "b43de5e468d20bc14025cf38efc2a4232e580c0f",
          "m900.rom", "roms/kc85",
          0x2000);
  add_rom(_(""),
          "c252b109cbdc104362989ff1a325e0d017e5751f",
          "m901.rom", "roms/kc85",
          0x2000);
  add_rom(_("OS PI/88 (yellow/blue)"),
          "b8373a44e4553197e3dd23008168d5214b878837",
          "pi88_ge.853", "roms/kc85",
          0x2000);
  add_rom(_("OS PI/88 (black/white)"),
          "9b5c068f10ff34bc3253f5b51abad51c8da9dd5d",
          "pi88_sw.853", "roms/kc85",
          0x2000);
  add_rom(_("OS PI/88 (white/blue)"),
          "b8b6f606b76bce9fb7fcd61a14120e5e026b6b6e",
          "pi88_ws.853", "roms/kc85",
          0x2000);
  add_rom(_(""),
          "fd64fb7b0c7d6e3cfa87ba0087d11ad9aad17feb",
          "vdip12.rom", "roms/kc85",
          0x800);
  add_rom(_(""),
          "9ca2f420a15e9f18682f65c8f5a08eb13afe1409",
          "vdip21.rom", "roms/kc85",
          0x1280);
  add_rom(_("Assembler"),
          "a578d2cf0ea6eb35dcd13e4107e15187de906097",
          "ass.kmc", "roms/kramermc",
          0x1c00);
  add_rom(_("BASIC"),
          "61d055495ffcc4a281ef0abc3e299ea95f42544b",
          "basic.kmc", "roms/kramermc",
          0x3000);
  add_rom(_("Charset-ROM"),
          "71bbad90dd427d0132c871a4d3848ab3d4d84b8a",
          "chargen.kmc", "roms/kramermc",
          0x800);
  add_rom(_("Debugger"),
          "42e5ced4f965124ae50ec7ac9861d6b668cfab99",
          "debugger.kmc", "roms/kramermc",
          0x400);
  add_rom(_("Editor"),
          "505615a218865aa8becde13848a23e1241a14b96",
          "editor.kmc", "roms/kramermc",
          0x400);
  add_rom(_("IO-Monitor"),
          "197d4ede31ee8768dd4a17854ee21c468e98b3d6",
          "io-mon.kmc", "roms/kramermc",
          0x400);
  add_rom(_("Reassembler"),
          "3319a96aad710441af30dace906b9725e07ca92c",
          "reass.kmc", "roms/kramermc",
          0x400);
  add_rom(_("System-ROM 1"),
          "044440b13e62addbc3f6a77369cfd16f99b39752",
          "lc80__00.rom", "roms/lc80",
          0x400);
  add_rom(_("System-ROM 2"),
          "3a6cbd0c57c38eadb7055dca4b396c348567d1d5",
          "lc80__08.rom", "roms/lc80",
          0x400);
  add_rom(_("System-ROM"),
          "6fff953f0f1eee829fd774366313ab7e8053468c",
          "lc80__2k.rom", "roms/lc80",
          0x800);
  add_rom(_("System-ROM 1"),
          "f2be3f2a9d3780d59657e49b3abeffb0fc13db89",
          "lc80e_00.rom", "roms/lc80",
          0x1000);
  add_rom(_("System-ROM 2"),
          "0ea019b0944736ae5b842bf9aa3537300f259b98",
          "lc80e_10.rom", "roms/lc80",
          0x1000);
  add_rom(_("System-ROM 3"),
          "2f7b3fd046c965185606253f6cd9372da289ca6f",
          "lc80e_c0.rom", "roms/lc80",
          0x1000);
//  add_rom(_("Charset-ROM"),
//          "71bbad90dd427d0132c871a4d3848ab3d4d84b8a",
//          "chargen.pcm", "roms/muglerpc",
//          0x800);
  add_rom(_("System-ROM"),
          "ed45100afde4fa807557bb2c98bedb8f83d7e7e0",
          "muglerpc.pcm", "roms/muglerpc",
          0x2000);
  add_rom(_("System-ROM 1"),
          "323f9d3165ed8e95fd530967d8d2f44928015bbd",
          "poly880a.rom", "roms/poly880",
          0x400);
  add_rom(_("System-ROM 2"),
          "274f1dbe58560b1e0a135be6efb59cffdfd78dde",
          "poly880b.rom", "roms/poly880",
          0x400);
  add_rom(_("SC1-ROM"),
          "01568911446eda9f05ec136df53da147b7c6f2bf",
          "sc1.rom", "roms/poly880",
          0x1000);
  add_rom(_("System-ROM"),
          "3472e5a9357eaba3ed6de65dee2b1c6b29349dd2",
          "monitor.rom", "roms/vcs80",
          0x200);
  add_rom(_("System-ROM V2.0"),
          "c3a45ea5cc4da2b7c270068ba1e2d75916960709",
          "z1013_20.rom", "roms/z1013",
          0x800);
  add_rom(_("System-ROM A2"),
          "97e158f589198cb96aae1567ee0aa6e47824027e",
          "z1013_a2.rom", "roms/z1013",
          0x800);
  add_rom(_("Bootlader BL4"),
          "eb973eb1bb736ab93e0538a1008a681624c90564",
          "z1013_bl.rom", "roms/z1013",
          0x980);
  add_rom(_("Brosig-ROM K7659/2.028"),
          "0bcd20338cf0706b384f40901b7f8498c6f6c320",
          "z1013_rb.rom", "roms/z1013",
          0x1000);
  add_rom(_("Super Ur-Lader 5.0"),
          "26d3c8352e57bccb9324ff5e89c0b557d78a9aad",
          "z1013_ul.rom", "roms/z1013",
          0x800);
  add_rom(_("Z1013 Charset-ROM"),
          "c34da6acfb278dfaebd0efd2882a116f18780a50",
          "z1013_zg.rom", "roms/z1013",
          0x1000);
  add_rom(_(""),
          "5a904c47a56dfb3ea2d9e10d46e86693c86b4f3f",
          "192k__d2.851", "roms/z9001",
          0x10000);
  add_rom(_(""),
          "b0a16005d8c15db25d17d7f04ee7b367bb152cc6",
          "192k__d3.851", "roms/z9001",
          0x8000);
  add_rom(_(""),
          "dd2b2e5332bee18c9866599aadae6a107a879735",
          "192k__d5.851", "roms/z9001",
          0x2000);
  add_rom(_(""),
          "dfd9b2633b370dd1c443626794ce4a8cb8843a5a",
          "basic_c0.851", "roms/z9001",
          0x2800);
  add_rom(_("KC-BASIC"),
          "ea85b53e21429c4cb85cdb81b92f278a8f4eb574",
          "basic_c0.87a", "roms/z9001",
          0x2800);
  add_rom(_(""),
          "8ffecc64ba35c953c93738f8568c83dc6af1ae72",
          "basic_c0.87b", "roms/z9001",
          0x2800);
  add_rom(_(""),
          "af290ec9fbd9c29240caea19730e2c7c433077fa",
          "bitex_c0.851", "roms/z9001",
          0x1800);
  add_rom(_("Z9001 Charset-ROM"),
          "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2",
          "chargen.851", "roms/z9001",
          0x800);
  add_rom(_(""),
          "5dbb0a195080da2dd7bfa9176155c44caf448180",
          "cpmz9_c0.851", "roms/z9001",
          0x800);
  add_rom(_(""),
          "c27fbfcf39cdf683cbb232b25ec6e99a90d6d7a8",
          "edas__c0.851", "roms/z9001",
          0x2800);
  add_rom(_(""),
          "ba5b0e2da22a876fef0e540bec55c68c29f261a0",
          "idas__c0.851", "roms/z9001",
          0x2800);
  add_rom(_(""),
          "9a30422f9f01856573a6364adfffabd6b3e303a0",
          "kc_caos.rom", "roms/z9001",
          0x1000);
  add_rom(_(""),
          "27e4566f000ad5678f6e2bc7055dfb39484f8833",
          "kcpascal.851", "roms/z9001",
          0x8000);
  add_rom(_(""),
          "05fe45dc62e9eced3a5f334417677acedfe028aa",
          "mega_rom.851", "roms/z9001",
          0x280000);
  add_rom(_("System-ROM"),
          "553609631f5eaa7d6758a73f56c613e280a5b310",
          "os____f0.851", "roms/z9001",
          0x1000);
  add_rom(_("System-ROM"),
          "b1df6b499517c8366a0795030ee800e8a258e938",
          "os____f0.87b", "roms/z9001",
          0x1000);
  add_rom(_("ROM-Bank System without menu"),
          "c2893ce5bb23b280ba4e982e860586d21de2469b",
          "os_rb20.rom", "roms/z9001",
          0x1000);
  add_rom(_("ROM-Bank System with menu"),
          "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb",
          "os_rb21.rom", "roms/z9001",
          0x1000);
  add_rom(_(""),
          "5736f3b3f705ef4d099fd06db1120d81f0cab933",
          "r80___c0.851", "roms/z9001",
          0x1c00);
  add_rom(_(""),
          "7e3a6a659edacd58d748cae30a662a50f6262073",
          "rom_bank.851", "roms/z9001",
          0x20000);
  add_rom(_("CGA Charset-ROM (extended)"),
          "4d36fefd335903680c45a5e3f38b969d2e9bb621",
          "zg_cgai.rom", "roms/z9001",
          0x800);
  add_rom(_("CGA Charset-ROM"),
          "f57a78a928fe1151b2fedb7f1a93a141195422ff",
          "zg_cga.rom", "roms/z9001",
          0x800);
  add_rom(_("Z9001 Charset-ROM (german)"),
          "912bb7d1f8b4582894125e82da080bd9c3b88f34",
          "zg_de.rom", "roms/z9001",
          0x800);
  add_rom(_(""),
          "1fd35f33661eb278e727a15cb9480a6e196b65b6",
          "zm30__c0.851", "roms/z9001",
          0xd00);
  add_rom(_(""),
          "45257fa753fef64419d755f917ca29b696872dfa",
          "zsid__c0.851", "roms/z9001",
          0x2800);
}

RomRegistry::~RomRegistry(void)
{
}

void
RomRegistry::add_rom(const char *name, const char *id, const char *filename, const char *directory, int size)
{
  if (_entries.find(id) != _entries.end())
    {
      printf("already has key '%s'\n", id);
      return;
    }
  _entries[id] = new RomRegistryEntry(id, name, filename, directory, size);
}

const RomRegistryEntry *
RomRegistry::get_rom(const char *id) const
{
  rom_entry_map_t::const_iterator it = _entries.find(id);
  return it == _entries.end() ? NULL : (*it).second;
}

bool
RomRegistry::check_roms(void) const
{
  for (rom_entry_map_t::const_iterator it = _entries.begin();it != _entries.end();it++)
    {
      string datadir(kcemu_datadir);
      string romdir = datadir + "/" + (*it).second->get_directory() + "/";
      string rompath = romdir + (*it).second->get_filename();
      const char *romfile = rompath.c_str();

      cout << (*it).second->get_id() << " - " << (*it).second->get_name() << " - " << (*it).second->get_filename() << " -> " << romfile;
      if (access(romfile, R_OK) == 0)
        cout << " OK" << endl;
      else
        cout << " *** ERROR ***" << endl;
    }
  return true;
}

const RomRegistry *
RomRegistry::instance()
{
  return _instance;
}
