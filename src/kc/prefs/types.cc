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

#include <iostream>

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "kc/system.h"
#include "kc/roms.h"
#include "kc/prefs/types.h"
#include "kc/prefs/prefs.h"

using namespace std;

struct less_system_type : public binary_function<SystemType *, SystemType *, bool> {
    bool operator()(SystemType *type1, SystemType *type2) {
        return type1->get_sort() < type2->get_sort();
    }
};

struct find_by_name : public unary_function<SystemType *, void> {
    string _name;
    SystemType *_result;
    find_by_name(string name) : _name(name), _result(NULL) {}
    void operator()(SystemType *type) {
        if ((_result == NULL) && (strcmp(_name.c_str(), type->get_name()) == 0))
            _result = type;
    }
};

emulation_type_list_t EmulationType::_emulation_type_list;

EmulationType EmulationType::_emulation_type_kc85_1(  KC_TYPE_85_1,      1, "KC 85/1",              "KC 85/1",   "kc85-1",   "icon-kc87.png",     "kcemu-kc87.xpm",     "kc851.key",   "sys-z9001",    "window-module-z9001",   4, 0xf000, 0xf000);
EmulationType EmulationType::_emulation_type_kc85_2(  KC_TYPE_85_2,      2, "KC 85/2",              "KC 85/2",   "kc85-2",   "icon-kc85.png",     "kcemu-kc85.xpm",     "kc854.key",   "sys-kc85",     "window-module-kc85",  -12, 0xf000, 0xe000);
EmulationType EmulationType::_emulation_type_kc85_3(  KC_TYPE_85_3,      3, "KC 85/3",              "KC 85/3",   "kc85-3",   "icon-kc85.png",     "kcemu-kc85.xpm",     "kc854.key",   "sys-kc85",     "window-module-kc85",  -12, 0xf000, 0xe000);
EmulationType EmulationType::_emulation_type_kc85_4(  KC_TYPE_85_4,      4, "KC 85/4",              "KC 85/4",   "kc85-4",   "icon-kc85.png",     "kcemu-kc85.xpm",     "kc854.key",   "sys-kc85",     "window-module-kc85",  -12, 0xf000, 0xe000);
EmulationType EmulationType::_emulation_type_kc85_5(  KC_TYPE_85_5,      5, "KC 85/5",              "KC 85/5",   "kc85-5",   "icon-kc85.png",     "kcemu-kc85.xpm",     "kc854.key",   "sys-kc855",    "window-module-kc85",  -12, 0xf000, 0xe000);
EmulationType EmulationType::_emulation_type_kc87(    KC_TYPE_87,        7, "KC 87",                "KC 87",     "kc87",     "icon-kc87.png",     "kcemu-kc87.xpm",     "kc851.key",   "sys-z9001",    "window-module-z9001",   4, 0xf000, 0xf000);
EmulationType EmulationType::_emulation_type_a5105(   KC_TYPE_A5105,     9, "A5105",                "A5105",     "a5105",    "icon-bic.png",      "kcemu-bic.xpm",      "a5105.key",   "sys-a5105",    "",                      4, 0x0000, 0x0000);

EmulationType EmulationType::_emulation_type_z1013(   KC_TYPE_Z1013,     0, "Z1013",                "Z1013",     "z1013",    "icon-z1013.png",    "kcemu-z1013.xpm",    "",            "sys-z1013",    "window-module-z1013",   8, 0xf000, 0xf000);
EmulationType EmulationType::_emulation_type_lc80(    KC_TYPE_LC80,      8, "LC 80",                "LC 80",     "lc80",     "icon-lc80.png",     "kcemu-lc80.xpm",     "lc80.key",    "sys-lc80",     "",                      6, 0x0000, 0x0000);
EmulationType EmulationType::_emulation_type_poly880( KC_TYPE_POLY880,   6, "Polycomputer 880",     "Poly880",   "poly880",  "icon-poly880.png",  "kcemu-poly880.xpm",  "poly880.key", "sys-poly880",  "",                      0, 0x0000, 0x0000);
EmulationType EmulationType::_emulation_type_muglerpc(KC_TYPE_MUGLERPC, -1, "Mugler PC (PC/M)",     "PC/M",      "pcm",      "icon-pcm.png",      "kcemu-pcm.xpm",      "",            "sys-muglerpc", "",                      0, 0x0000, 0x0000);
EmulationType EmulationType::_emulation_type_kramermc(KC_TYPE_KRAMERMC, -1, "Kramer Microcomputer", "Kramer-MC", "kramermc", "icon-kramermc.png", "kcemu-kramermc.xpm", "",            "sys-kramermc", "",                      0, 0x0000, 0x0000);
EmulationType EmulationType::_emulation_type_vcs80(   KC_TYPE_VCS80,    -1, "VCS 80",               "VCS 80",    "vcs80",    "icon-vcs80.png",    "kcemu-vcs80.xpm",    "vcs80.key",   "sys-vcs80",    "",                      6, 0x0000, 0x0000);
EmulationType EmulationType::_emulation_type_c80(     KC_TYPE_C80,      -1, "C-80",                 "C-80",      "c80",      "icon-c80.png",      "kcemu-c80.xpm",      "",            "sys-c80",      "",                      0, 0x0000, 0x0000);

EmulationType::EmulationType(kc_type_t kc_type, int type, string name, string short_name, string config_name, string icon_name, string image_name, string keyboard_filename, string help_topic, string help_topic_module, int module_slots, word_t power_on_addr, word_t reset_addr) {
    _type = type;
    _kc_type = kc_type;
    
    _name = name;
    _short_name = short_name;
    _config_name = config_name;
    _icon_name = icon_name;
    _image_name = image_name;
    _keyboard_filename = keyboard_filename;
    _help_topic = help_topic;
    _help_topic_module = help_topic_module;
    _module_slots = module_slots;
    _power_on_addr = power_on_addr;
    _reset_addr = reset_addr;

}

EmulationType::~EmulationType(void) {
}

const char *
EmulationType::get_name(void) const {
    return _name.c_str();
}

const char *
EmulationType::get_config_name(void) const {
    return _config_name.c_str();
}

const char *
EmulationType::get_short_name(void) const {
    return _short_name.c_str();
}

const char *
EmulationType::get_icon_name(void) const {
    return _icon_name.c_str();
}

const char *
EmulationType::get_image_name(void) const {
    return _image_name.c_str();
}

const char *
EmulationType::get_keyboard_filename(void) const {
  if (_keyboard_filename.empty())
    return NULL;
  
  return _keyboard_filename.c_str();
}

const char *
EmulationType::get_help_topic(void) const {
  return _help_topic.c_str();
}

const char *
EmulationType::get_help_topic_module(void) const {
  if (_help_topic_module.empty())
    return NULL;

  return _help_topic_module.c_str();
}

const int
EmulationType::get_module_slots(void) const {
  return _module_slots;
}

const word_t
EmulationType::get_power_on_addr(void) const {
  return _power_on_addr;
}

const word_t
EmulationType::get_reset_addr(void) const {
  return _reset_addr;
}

int
EmulationType::get_type(void) const {
    return _type;
}

kc_type_t
EmulationType::get_kc_type(void) const {
    return _kc_type;
}

emulation_type_list_t &
EmulationType::get_emulation_types(void) {
    if (_emulation_type_list.size() == 0) {
        _emulation_type_list.push_back(&_emulation_type_kc85_1);
        _emulation_type_list.push_back(&_emulation_type_kc85_2);
        _emulation_type_list.push_back(&_emulation_type_kc85_3);
        _emulation_type_list.push_back(&_emulation_type_kc85_4);
        _emulation_type_list.push_back(&_emulation_type_kc85_5);
        _emulation_type_list.push_back(&_emulation_type_kc87);
        _emulation_type_list.push_back(&_emulation_type_lc80);
        _emulation_type_list.push_back(&_emulation_type_z1013);
        _emulation_type_list.push_back(&_emulation_type_a5105);
        _emulation_type_list.push_back(&_emulation_type_poly880);
        _emulation_type_list.push_back(&_emulation_type_kramermc);
        _emulation_type_list.push_back(&_emulation_type_muglerpc);
        _emulation_type_list.push_back(&_emulation_type_vcs80);
        _emulation_type_list.push_back(&_emulation_type_c80);
    }
    return _emulation_type_list;
}

ROMEntry::ROMEntry(const char *id, const char *name, const char *description, bool is_default) : _id(id), _name(name), _description(description), _is_default(is_default)
{
}

ROMEntry::~ROMEntry(void)
{
}

const string
ROMEntry::get_id(void) const
{
    return _id;
}

const string
ROMEntry::get_name(void) const
{
    return _name;
}

const string
ROMEntry::get_description(void) const
{
    return _description;
}

const bool
ROMEntry::is_default(void) const
{
    return _is_default;
}

const char * SystemROM::ROM_KEY_CAOSC       = N_("rom_caos_c");
const char * SystemROM::ROM_KEY_CAOSE       = N_("rom_caos_e");
const char * SystemROM::ROM_KEY_SYSTEM      = N_("rom_system");
const char * SystemROM::ROM_KEY_SYSTEM1     = N_("rom_system_1");
const char * SystemROM::ROM_KEY_SYSTEM2     = N_("rom_system_2");
const char * SystemROM::ROM_KEY_SYSTEM3     = N_("rom_system_3");
const char * SystemROM::ROM_KEY_SYSTEM4     = N_("rom_system_4");
const char * SystemROM::ROM_KEY_USER        = N_("rom_user");
const char * SystemROM::ROM_KEY_BASIC       = N_("rom_basic");
const char * SystemROM::ROM_KEY_DEBUGGER    = N_("rom_debugger");
const char * SystemROM::ROM_KEY_REASSEMBLER = N_("rom_reassembler");
const char * SystemROM::ROM_KEY_EDITOR      = N_("rom_editor");
const char * SystemROM::ROM_KEY_ASSEMBLER   = N_("rom_assembler");
const char * SystemROM::ROM_KEY_CHARGEN     = N_("rom_charset");
const char * SystemROM::ROM_KEY_D004_FC     = N_("rom_d004_fc");
const char * SystemROM::ROM_KEY_D004_F8     = N_("rom_d004_f8");

SystemROM::SystemROM(const char *name, bool mandatory, int size)
: _size(size),
  _name(name),
  _mandatory(mandatory)
{
}

SystemROM::~SystemROM(void)
{
}

int
SystemROM::get_size(void) const
{
    return _size;
}

bool
SystemROM::is_mandatory(void) const
{
    return _mandatory;
}

const string
SystemROM::get_name(void) const
{
    return _name;
}

const rom_entry_list_t &
SystemROM::get_roms(void) const
{
    return _roms;
}

const ROMEntry *
SystemROM::get_default_rom(void) const
{
  for (rom_entry_list_t::const_iterator it = _roms.begin(); it != _roms.end(); it++)
    if ((*it)->is_default())
      return (*it);

  return NULL;
}

void
SystemROM::add_rom(const char *id, const char *description, bool is_default)
{
  if (is_mandatory())
    {
      const RomRegistryEntry *entry = RomRegistry::instance()->get_rom(id);
      if (entry == NULL)
        {
          cerr << "can't find ROM with id '" << id << "' in registry\n";
          exit(1);
        }
      _roms.push_back(new ROMEntry(id, entry->get_filename(), description, is_default));
    }
  else
    {
      _roms.push_back(new ROMEntry("", id, description, is_default));
    }
}

SystemType::SystemType(int sort, string name, int type, EmulationType &emulation_type, kc_variant_t kc_variant, string description)
: _sort(sort),
  _type(type),
  _name(name),
  _display_name(emulation_type.get_name()),
  _kc_variant(kc_variant),
  _description(description),
  _emulation_type(emulation_type)
{
  _ui_callback_value = 50000;
  _ui_callback_retrace_value = 0;
}

SystemType::~SystemType(void) {
}

const char *
SystemType::get_display_name(void) const
{
    return _display_name.c_str();
}

SystemType &
SystemType::set_display_name(const char *display_name)
{
    _display_name = display_name;
    return *this;
}

const string
SystemType::get_rom_directory(void) const
{
    return _rom_dir;
}

SystemType &
SystemType::set_rom_directory(const char *romdir)
{
    _rom_dir = romdir;
    return *this;
}

const unsigned long long
SystemType::get_ui_callback_value(void) const
{
    return _ui_callback_value;
}

SystemType &
SystemType::set_ui_callback_value(unsigned long long value)
{
    _ui_callback_value = value;
    return *this;
}

const unsigned long long
SystemType::get_ui_callback_retrace_value(void) const
{
    return _ui_callback_retrace_value;
}

SystemType &
SystemType::set_ui_callback_retrace_value(unsigned long long value)
{
    _ui_callback_retrace_value = value;
    return *this;
}

const system_rom_list_t &
SystemType::get_rom_list(void) const
{
    return _rom_list;
}

const SystemROM *
SystemType::get_rom(const char *key) const
{
    for (system_rom_list_t::const_iterator it = _rom_list.begin();it != _rom_list.end();it++) {
        if (strcmp((*it)->get_name().c_str(), key) == 0) {
            return (*it);
        }
    }
    return NULL;
}

void
SystemType::add_rom(SystemROM *rom, int default_idx, const char *filename, const char *description, va_list ap)
{
    rom->add_rom(filename, description, 0 == default_idx);

    int idx = 0;
    while (242) {
        idx++;
        const char *arg1 = va_arg(ap, const char *);
        if (arg1 == NULL)
            break;
        
        const char *arg2 = va_arg(ap, const char *);
        if (arg2 == NULL)
            break;
        
        rom->add_rom(arg1, arg2, idx == default_idx);
    }
    
    _rom_list.push_back(rom);
}

SystemType &
SystemType::add_rom(const char *name, int size, const char *filename, const char *description, ...)
{
    va_list ap;
    va_start(ap, description);
    add_rom(new SystemROM(name, true, size), 0, filename, description, ap);
    va_end(ap);
    return *this;
}

SystemType &
SystemType::add_rom(const char *name, int size, int default_idx, const char *filename, const char *description, ...)
{
    va_list ap;
    va_start(ap, description);
    add_rom(new SystemROM(name, true, size), default_idx, filename, description, ap);
    va_end(ap);
    return *this;
}

SystemType &
SystemType::add_optional_rom(const char *name, int size, const char *filename, const char *description, ...)
{
    va_list ap;
    va_start(ap, description);
    add_rom(new SystemROM(name, false, size), 0, filename, description, ap);
    va_end(ap);
    return *this;
}

const char *
SystemType::get_name(void) const
{
    return _name.c_str();
}

int
SystemType::get_sort(void) const
{
    return _sort;
}

int
SystemType::get_type(void) const
{
    return _type;
}

bool
SystemType::is_default(int type) const
{
    return _type == type;
}

kc_type_t
SystemType::get_kc_type(void) const
{
    return _emulation_type.get_kc_type();
}

kc_variant_t
SystemType::get_kc_variant(void) const
{
    return _kc_variant;
}

const char *
SystemType::get_kc_variant_name(void) const
{
    return get_name();
}

const char *
SystemType::get_description(void) const
{
    return _description.c_str();
}

const EmulationType &
SystemType::get_emulation_type(void) const
{
    return _emulation_type;
}

SystemInformation *SystemInformation::_instance = 0;

SystemInformation::SystemInformation(void) {
    /*
     *  Z1013
     */
    add_system_type(101, "z1013.01", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_01,
            N_("    Z1013 with 16k RAM and 2k ROM, running at 1MHz. This was the\n"
            "    first version with cheap circuits so the CPU was clocked at only 1MHz\n"
            "    The ROM contains the Z1013 monitor version 2.02.\n"))
        .set_display_name("Z1013.01")
        .set_ui_callback_value(20000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "c3a45ea5cc4da2b7c270068ba1e2d75916960709", _("System-ROM V2.0"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(102, "z1013.12", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_12,
            N_("    Z1013 with 1k sRAM and 2k ROM, running at 2MHz. Industry\n"
            "    version with only 1k static RAM. Monitor version 2.02.\n"))
        .set_display_name("Z1013.12")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "c3a45ea5cc4da2b7c270068ba1e2d75916960709", _("System-ROM V2.0"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(103, "z1013.16", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_16,
            N_("    Z1013 with 16k RAM and 2k ROM, running at 2MHz. Successor of\n"
            "    the Z1013.01 but with industry grade circuits clocked at 2MHz.\n"))
        .set_display_name("Z1013.16")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "c3a45ea5cc4da2b7c270068ba1e2d75916960709", _("System-ROM V2.0"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(104, "z1013.64", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_64,
            N_("    Z1013 with 64k RAM and 2x2k ROM, running at 2MHz. The ROM\n"
            "    contains two system images with 2k each. Loaded is the first\n"
            "    image with version 2.02 that is supposed to work with the foil keypad\n"
            "    shipped by the manufacturer.\n"))
        .set_display_name("Z1013.64")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "c3a45ea5cc4da2b7c270068ba1e2d75916960709", _("System-ROM V2.0"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(105, "z1013.a2", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_A2,
            N_("    Z1013 with 64k RAM and 2k ROM, running at 2MHz. The ROM\n"
            "    contains two system images with 2k each. Loaded is the second\n"
            "    image with version A.2 that supports an extended keyboard with an\n"
            "    8x8 matrix.\n"))
        .set_display_name("Z1013.A2")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "97e158f589198cb96aae1567ee0aa6e47824027e", _("System-ROM A2"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(106, "z1013.rb", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_RB,
            N_("    Z1013 with 64k RAM and 4k ROM. The ROM is the extended\n"
            "    version tagged 2.028 RB that has a different keyboard driver for\n"
            "    the keyboard K7659.\n"))
        .set_display_name("Z1013 (Brosig-ROM)")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000, "0bcd20338cf0706b384f40901b7f8498c6f6c320", _("Brosig-ROM K7659/2.028"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(107, "z1013.surl", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_SURL,
            N_("    Z1013 with 64k RAM and 4k ROM. The ROM contains the 'Super\n"
            "    Ur-Lader 5.0' that can read arbitrary tape files as system monitor.\n"
            "    The hardware modification port at address 4h is emulated which allows\n"
            "    programs to switch off the system ROM.\n"))
        .set_display_name("Z1013 (Super Ur-Lader 5.0)")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "26d3c8352e57bccb9324ff5e89c0b557d78a9aad", _("Super Ur-Lader 5.0"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    add_system_type(108, "z1013.bl4", -1, EmulationType::_emulation_type_z1013, KC_VARIANT_Z1013_BL4,
            N_("    Z1013 with 64k RAM and 4k ROM. The ROM contains the Bootloader\n"
            "    BL4 that can load monitor images from floppy disk or can start the\n"
            "    CP/M system.\n"))
        .set_display_name("Z1013 (Bootloader BL4)")
        .set_ui_callback_value(40000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/z1013")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 2432, "eb973eb1bb736ab93e0538a1008a681624c90564", _("Bootlader BL4"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x1000, "c34da6acfb278dfaebd0efd2882a116f18780a50", _("Z1013 Charset-ROM"), NULL);
    /*
     *  KC 85/1
     */
    add_system_type(201, "z9001.10", -1, EmulationType::_emulation_type_kc85_1, KC_VARIANT_85_1_10,
            N_("    Z9001 (later called KC 85/1) with monochrome display, no internal BASIC\n"
            "    ROM. This computer has 16k RAM and 4k ROM with the robotron z9001 system.\n"))
        .set_display_name("KC 85/1.10")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "553609631f5eaa7d6758a73f56c613e280a5b310", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    add_system_type(202, "z9001.11", -1, EmulationType::_emulation_type_kc85_1, KC_VARIANT_85_1_11,
            N_("    Like the z9001.10 but with color extension card.\n"))
        .set_display_name("KC 85/1.11")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "553609631f5eaa7d6758a73f56c613e280a5b310", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    /*
     *  KC 85/2
     */
    add_system_type(302, "kc85/2", 2, EmulationType::_emulation_type_kc85_2, KC_VARIANT_NONE,
            N_("    HC900 (later called KC 85/2) with 16k RAM and 8k ROM. System is called\n"
            "    HC-CAOS 2.2 (home computer - cassette aided operating system). BASIC\n"
            "    is only available as external module.\n"))
        .set_ui_callback_value(35000)
        .set_rom_directory("/roms/kc85")
        .add_rom(SystemROM::ROM_KEY_CAOSE, 0x2000,
                 "242a777788c774c5f764313361b1e0a65139ab32", _("HC-900 CAOS"),
                 "568dd59bfad4c604ba36bc05b094fc598a642f85", _("CAOS 2.2"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_D004_FC, 0x2000,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_D004_F8, 0x2000, 3,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL);
    /*
     *  KC 85/3
     */
    add_system_type(400, "kc85/3", 3, EmulationType::_emulation_type_kc85_3, KC_VARIANT_NONE,
            N_("    KC 85/3 with 16k RAM, 8k system ROM with HC-CAOS 3.1 and 8k BASIC ROM.\n"))
        .set_ui_callback_value(35000)
        .set_rom_directory("/roms/kc85")
        .add_rom(SystemROM::ROM_KEY_CAOSE, 0x2000,
                 "efd002fc9146116936e6e6be0366d2afca33c1ab", _("CAOS 3.1"),
                 "20447d27c9aa41a1c7a3d6ad0699edb06a207aa6", _("CAOS 3.3"),
                 "ee9f8e7427b9225ae2cecbcfb625d629ab6a601d", _("CAOS 3.4"),
                 "b8373a44e4553197e3dd23008168d5214b878837", _("OS PI/88 (yellow/blue)"),
                 "9b5c068f10ff34bc3253f5b51abad51c8da9dd5d", _("OS PI/88 (black/white)"),
                 "b8b6f606b76bce9fb7fcd61a14120e5e026b6b6e", _("OS PI/88 (white/blue)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2000, "c2e3af55c79e049e811607364f88c703b0285e2e", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_D004_FC, 0x2000,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_D004_F8, 0x2000, 3,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL);
    /*
     *  KC 85/4
     */
    add_system_type(500, "kc85/4", 4, EmulationType::_emulation_type_kc85_4, KC_VARIANT_NONE,
            N_("    KC 85/4 with 64k RAM, 64k screen memory, 12k system ROM with HC-CAOS 4.2\n"
            "    and 8k BASIC ROM.\n"))
        .set_ui_callback_value(35000)
        .set_rom_directory("/roms/kc85")
        .add_rom(SystemROM::ROM_KEY_CAOSC, 0x1000, 1,
                 "acd998e3d9e8f592cd884aafc8ac4d291e40e097", _("CAOS 4.1 (C)"),
                 "774fc2496a59b77c7c392eb5aa46420e7722797e", _("CAOS 4.2 (C)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_CAOSE, 0x2000, 1,
                 "e19819fb477dcb742a13729a9bf5943d63abe863", _("CAOS 4.1 (E)"),
                 "4300f7ff813c1fb2d5c928dbbf1c9e1fe52a9577", _("CAOS 4.2 (E)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2000, "c2e3af55c79e049e811607364f88c703b0285e2e", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_D004_FC, 0x2000,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_D004_F8, 0x2000, 3,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL);
    /*
     *  KC 85/5
     */
    add_system_type(600, "kc85/5", 5, EmulationType::_emulation_type_kc85_5, KC_VARIANT_NONE,
            N_("    Unofficial successor of the KC 85/4 with 256k RAM, 64k screen memory,\n"
            "    16k system ROM with HC-CAOS 4.4, 8k BASIC ROM and 24k User ROM. It is\n"
            "    intended to be used with a modified D004 floppy device with additional\n"
            "    hard-disk support.\n"))
        .set_ui_callback_value(35000)
        .set_rom_directory("/roms/kc85")
        .add_rom(SystemROM::ROM_KEY_CAOSC, 0x2000, 1,
                 "5342be5104206d15e7471b094c7749a8a3d708ad", _("CAOS 4.3 (C)"),
                 "2cf8023ee71ca50b92f9f151b7519f59727d1c79", _("CAOS 4.4 (C)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_CAOSE, 0x2000, 1,
                 "521ac2fbded4148220f8af2d5a5ab99634364079", _("CAOS 4.3 (E)"),
                 "53ba4394d96e287ff8af01322af1e9879d4e77c4", _("CAOS 4.4 (E)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x8000, 1,
                 "be2c68a5b461014c57e33a127c3ffb32b0ff2346", _("KC-BASIC (EDAS/TEMO/FORTH)"),
                 "dcd3b44a4ef116c5981fb932dff1aa5f57735f42", _("KC-BASIC (EDAS/TEMO/USB)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_D004_FC, 0x2000, 3,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_D004_F8, 0x2000, 0,
                 "66f476de78fb474d9ac61c6eaffce3354fd66776", _("D004 Version 2.0 (02.01.1989)"),
                 "5582b2541a34a90c7a9516a6a222d4961fc54fcf", _("D004 Version 3.0 (07.12.1997)"),
                 "38b3164dce23573375fc0237f348d9a699fc6f9f", _("D004 Version 3.1 (22.01.2003)"),
                 "8232adb5e5f0b25b52f9873cff14831da3a0398a", _("D004 Version 3.2 (28.10.2008)"),
                 NULL);
    /*
     *  Polycomputer 880
     */
    add_system_type(700, "poly880", 6, EmulationType::_emulation_type_poly880, KC_VARIANT_NONE,
            N_("    Polycomputer 880.\n"))
        .set_display_name("Polycomputer 880")
        .set_ui_callback_value(18000)
        .set_rom_directory("/roms/poly880")
        .add_rom(SystemROM::ROM_KEY_SYSTEM1, 0x0400, "323f9d3165ed8e95fd530967d8d2f44928015bbd", _("System-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM2, 0x0400, "274f1dbe58560b1e0a135be6efb59cffdfd78dde", _("System-ROM 2"), NULL)
        .add_optional_rom(SystemROM::ROM_KEY_SYSTEM3, 0x0400, "poly880c.rom", _("Optional-ROM 1"), NULL)
        .add_optional_rom(SystemROM::ROM_KEY_SYSTEM4, 0x0400, "poly880d.rom", _("Optional-ROM 1"), NULL);
    add_system_type(701, "poly880.sc1", -6, EmulationType::_emulation_type_poly880, KC_VARIANT_POLY880_SC1,
            N_("    Polycomputer 880 with chess program rom from SC1.\n"))
        .set_display_name("Polycomputer 880 + SC1 ROM")
        .set_ui_callback_value(18000)
        .set_rom_directory("/roms/poly880")
        .add_rom(SystemROM::ROM_KEY_SYSTEM1, 0x1000, "01568911446eda9f05ec136df53da147b7c6f2bf", _("SC1-ROM"), NULL);
    /*
     *  KC 87
     */
    add_system_type(801, "kc87.10", -7, EmulationType::_emulation_type_kc87, KC_VARIANT_87_10,
            N_("    Like the KC 87.11 but without the color extension card.\n"))
        .set_display_name("KC 87.10")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "553609631f5eaa7d6758a73f56c613e280a5b310", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2800, "ea85b53e21429c4cb85cdb81b92f278a8f4eb574", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    add_system_type(802, "kc87.11", 7, EmulationType::_emulation_type_kc87, KC_VARIANT_87_11,
            N_("    Successor of the Z9001 with internal 10k BASIC ROM.\n"))
        .set_display_name("KC 87.11")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "553609631f5eaa7d6758a73f56c613e280a5b310", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2800, "ea85b53e21429c4cb85cdb81b92f278a8f4eb574", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    add_system_type(803, "kc87.20", -8, EmulationType::_emulation_type_kc87, KC_VARIANT_87_20,
            N_("    Like the KC 87.21 but without the color extension card.\n"))
        .set_display_name("KC 87.20")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "b1df6b499517c8366a0795030ee800e8a258e938", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2800, "8ffecc64ba35c953c93738f8568c83dc6af1ae72", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    add_system_type(804, "kc87.21", -8, EmulationType::_emulation_type_kc87, KC_VARIANT_87_21,
            N_("    New series with extended BASIC ROM (still 10k but with some routines for\n"
            "    plotter support were added).\n"))
        .set_display_name("KC 87.21")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "b1df6b499517c8366a0795030ee800e8a258e938", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2800, "8ffecc64ba35c953c93738f8568c83dc6af1ae72", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    add_system_type(805, "kc87.30", -8, EmulationType::_emulation_type_kc87, KC_VARIANT_87_30,
            N_("    New batch of KC 87.20 with no known hardware differences.\n"))
        .set_display_name("KC 87.30")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "b1df6b499517c8366a0795030ee800e8a258e938", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2800, "8ffecc64ba35c953c93738f8568c83dc6af1ae72", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    add_system_type(806, "kc87.31", -8, EmulationType::_emulation_type_kc87, KC_VARIANT_87_31,
            N_("    New batch of KC 87.21 with no known hardware differences.\n"))
        .set_display_name("KC 87.31")
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/z9001")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x1000,
                 "b1df6b499517c8366a0795030ee800e8a258e938", _("System-ROM"),
                 "c2893ce5bb23b280ba4e982e860586d21de2469b", _("ROM-Bank System without menu"),
                 "5dbb661bdf4daf92d6c4ffbbdec674e57917e9eb", _("ROM-Bank System with menu"),
                 NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x2800, "8ffecc64ba35c953c93738f8568c83dc6af1ae72", _("KC-BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800,
                 "2e4928ba7161f5cce7173b7d2ded3d6596ae2aa2", _("Z9001 Charset-ROM"),
                 "912bb7d1f8b4582894125e82da080bd9c3b88f34", _("Z9001 Charset-ROM (german)"),
                 "f57a78a928fe1151b2fedb7f1a93a141195422ff", _("CGA Charset-ROM"),
                 "4d36fefd335903680c45a5e3f38b969d2e9bb621", _("CGA Charset-ROM (extended)"),
                 NULL);
    /*
     *  LC 80
     */
    add_system_type(900, "lc80", 8, EmulationType::_emulation_type_lc80, KC_VARIANT_LC80_1k,
            N_("    Single board computer LC 80 with 1k RAM and 2 x 1k ROM.\n"))
        .set_display_name("LC 80 (2 x U505)")
        .set_ui_callback_value(18000)
        .set_rom_directory("/roms/lc80")
        .add_rom(SystemROM::ROM_KEY_SYSTEM1, 0x0400, "044440b13e62addbc3f6a77369cfd16f99b39752", _("System-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM2, 0x0400, "3a6cbd0c57c38eadb7055dca4b396c348567d1d5", _("System-ROM 2"), NULL);
    add_system_type(902, "lc80.2k", -8, EmulationType::_emulation_type_lc80, KC_VARIANT_LC80_2k,
            N_("    Single board computer LC 80 with 1k RAM and 1 x 2k ROM.\n"))
        .set_display_name("LC 80 (1 x U2716)")
        .set_ui_callback_value(18000)
        .set_rom_directory("/roms/lc80")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0800, "6fff953f0f1eee829fd774366313ab7e8053468c", _("System-ROM"), NULL);
    add_system_type(903, "lc80e", -8, EmulationType::_emulation_type_lc80, KC_VARIANT_LC80e,
            N_("    Export variant of the LC 80 with 12k ROM including chess program.\n"))
        .set_display_name("LC 80e")
        .set_ui_callback_value(66000)
        .set_rom_directory("/roms/lc80")
        .add_rom(SystemROM::ROM_KEY_SYSTEM1, 0x1000, "f2be3f2a9d3780d59657e49b3abeffb0fc13db89", _("System-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM2, 0x1000, "0ea019b0944736ae5b842bf9aa3537300f259b98", _("System-ROM 2"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM3, 0x1000, "2f7b3fd046c965185606253f6cd9372da289ca6f", _("System-ROM 3"), NULL);
    /*
     *  A5105
     */
    add_system_type(1000, "bic", -9, EmulationType::_emulation_type_a5105, KC_VARIANT_A5105_K1505,
            N_("    BIC/A5105, only the base device is emulated, no floppy device.\n"))
        .set_display_name("K1505 (BIC Basis Device)")
        .set_ui_callback_value(60000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/a5105")
        .add_rom(SystemROM::ROM_KEY_SYSTEM1, 0x8000, "5c33139db9f59e50da5c76729752f8e653ae34ae", _("System-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM2, 0x2000, "7e5b587c59676e8549561117ea0b70234f439a94", _("System-ROM 2"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM3, 0x2000, "9a7bbe6f0d180dd513c7854f441cee986c8d9765", _("DSE-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM4, 0x2000, "7c16226be6c4c71013e8008fba9d2e9c5640b6a7", _("DSE-ROM 2"), NULL);
    add_system_type(1002, "a5105", 9, EmulationType::_emulation_type_a5105, KC_VARIANT_A5105_A5105,
            N_("    BIC/A5105, the full system including the floppy device.\n"))
        .set_display_name("A5105 (BIC Basis Device + Floppy Device)")
        .set_ui_callback_value(60000)
        .set_ui_callback_retrace_value(1000)
        .set_rom_directory("/roms/a5105")
        .add_rom(SystemROM::ROM_KEY_SYSTEM1, 0x8000, "5c33139db9f59e50da5c76729752f8e653ae34ae", _("System-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM2, 0x2000, "7e5b587c59676e8549561117ea0b70234f439a94", _("System-ROM 2"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM3, 0x2000, "9a7bbe6f0d180dd513c7854f441cee986c8d9765", _("DSE-ROM 1"), NULL)
        .add_rom(SystemROM::ROM_KEY_SYSTEM4, 0x2000, "7c16226be6c4c71013e8008fba9d2e9c5640b6a7", _("DSE-ROM 2"), NULL);
    /*
     *  Kramer MC
     */
    add_system_type(1100, "kramer-mc", -100, EmulationType::_emulation_type_kramermc, KC_VARIANT_NONE,
            N_("    Micro-Computer presented in the book \"Praktische Microcomputertechnik\"\n"
            "    by Manfred Kramer.\n"))
        .set_ui_callback_value(30000)
        .set_rom_directory("/roms/kramermc")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0400, "197d4ede31ee8768dd4a17854ee21c468e98b3d6", _("IO-Monitor"), NULL)
        .add_rom(SystemROM::ROM_KEY_BASIC, 0x3000, "61d055495ffcc4a281ef0abc3e299ea95f42544b", _("BASIC"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800, "71bbad90dd427d0132c871a4d3848ab3d4d84b8a", _("Charset-ROM"), NULL)
        .add_rom(SystemROM::ROM_KEY_DEBUGGER, 0x0400, "42e5ced4f965124ae50ec7ac9861d6b668cfab99", _("Debugger"), NULL)
        .add_rom(SystemROM::ROM_KEY_REASSEMBLER, 0x0400, "3319a96aad710441af30dace906b9725e07ca92c", _("Reassembler"), NULL)
        .add_rom(SystemROM::ROM_KEY_EDITOR, 0x0400, "505615a218865aa8becde13848a23e1241a14b96", _("Editor"), NULL)
        .add_rom(SystemROM::ROM_KEY_ASSEMBLER, 0x1c00, "a578d2cf0ea6eb35dcd13e4107e15187de906097", _("Assembler"), NULL);
    /*
     *  PC/M
     */
    add_system_type(1200, "mugler-pc", -100, EmulationType::_emulation_type_muglerpc, KC_VARIANT_NONE,
            N_("    CP/M based Micro-Computer presented in the magazine \"Funkamateur\"\n"
            "    by A. Mugler and H. Mathes.\n"))
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/muglerpc")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x2000, "ed45100afde4fa807557bb2c98bedb8f83d7e7e0", _("System-ROM"), NULL)
        .add_rom(SystemROM::ROM_KEY_CHARGEN, 0x0800, "71bbad90dd427d0132c871a4d3848ab3d4d84b8a", _("Charset-ROM"), NULL);
    /*
     *  VCS 80
     */
    add_system_type(1300, "vcs80", -100, EmulationType::_emulation_type_vcs80, KC_VARIANT_NONE,
            N_("    Minimal Z80 learning system presented in the magazine \"rfe\"\n"
            "    by Eckhard Schiller.\n"))
        .set_ui_callback_value(25000)
        .set_ui_callback_retrace_value(4000)
        .set_rom_directory("/roms/vcs80")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0200, "3472e5a9357eaba3ed6de65dee2b1c6b29349dd2", _("System-ROM"), NULL);
    /*
     *  C 80
     */
    add_system_type(1400, "c80", -100, EmulationType::_emulation_type_c80, KC_VARIANT_NONE,
            N_("    Minimal Z80 learning system designed by Dipl.-Ing. Joachim Czepa.\n"))
        .set_ui_callback_value(50000)
        .set_rom_directory("/roms/c80")
        .add_rom(SystemROM::ROM_KEY_SYSTEM, 0x0400, "14f72cb73a4068b7a5d763cc0e254639c251ce2e", _("System-ROM"), NULL)
        .add_optional_rom(SystemROM::ROM_KEY_USER, 0x0400, "user.rom", _("User-ROM"), NULL);

    _system_type_list.sort(less_system_type());
}

SystemInformation::~SystemInformation(void) {
}

SystemType &
SystemInformation::add_system_type(int sort, string name, int type, EmulationType& emulation_type, kc_variant_t kc_variant, string description) {
    SystemType *system_type = new SystemType(sort, name, type, emulation_type, kc_variant, description);
    _system_type_list.push_back(system_type);
    return *system_type;
}

SystemInformation *
SystemInformation::instance(void) {
    if (_instance == NULL)
        _instance = new SystemInformation();
    
    return _instance;
}

system_type_list_t &
SystemInformation::get_system_types(void) {
    return _system_type_list;
}

const SystemType *
SystemInformation::get_system_type(kc_type_t kc_type, kc_variant_t kc_variant) const {
    for (system_type_list_t::const_iterator it = _system_type_list.begin();it != _system_type_list.end();it++) {
        if ((*it)->get_kc_type() != kc_type)
          continue;
        
        if ((*it)->get_kc_variant() != kc_variant)
          continue;
        
        return (*it);
    }
    
    return NULL;
}

void
SystemInformation::show_types(void) {
    cout << "available emulations:";
    
    int width = 0;
    kc_type_t old_type = KC_TYPE_NONE;
    for (system_type_list_t::iterator it = _system_type_list.begin();it != _system_type_list.end();it++) {
        int type = abs((*it)->get_type());
        kc_type_t kc_type = (*it)->get_kc_type();
        
        width++;
        if (old_type != kc_type)
            cout << endl << "  ";
        else
            cout << ",";
        
        if (kc_type == KC_TYPE_NONE)
            break;
        
        if (old_type != kc_type) {
            if (type >= 100) {
                cout << "-";
            } else {
                cout << type;
            }
            cout << ": ";
            width = 5;
        }
        
        if (width > 60) {
            cout << endl << "     ";
            width = 5;
        }
        
        const char *name = (*it)->get_name();
        cout << " " << name;
        width += strlen(name) + 1;
        
        if ((*it)->get_type() >= 0) {
            width++;
            cout << "*";
        }
        
        old_type = kc_type;
    }
    cout << endl;
}

void
SystemInformation::show_types_with_description(void) {
    cout << _("available emulations:") << endl << endl;
    
    for (system_type_list_t::iterator it = _system_type_list.begin();it != _system_type_list.end();it++) {
        const char *ptr = "";
        if (strcmp((*it)->get_name(), (*it)->get_kc_variant_name()) == 0) {
            /*
             *  find all references
             */
            for (system_type_list_t::iterator it2 = _system_type_list.begin();it2 != _system_type_list.end();it2++) {
                if ((*it)->get_kc_variant_name() == (*it2)->get_kc_variant_name()) {
                    cout << ptr << (*it2)->get_name();
                    ptr = ", ";
                }
            }
            /*
             *  call gettext() here because static initialization of
             *  internationalized strings doesn't work.
             */
            cout << endl << _((*it)->get_description()) << endl;
        }
    }
}
