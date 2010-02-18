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

#ifndef __kc_prefs_types_h
#define __kc_prefs_types_h

#include <stdarg.h>
#include <list>
#include <string>

#include "kc/system.h"

#include "kc/prefs/profile.h"

using namespace std;

typedef enum {
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
    KC_TYPE_VCS80      = (1 << 12),
    KC_TYPE_C80        = (1 << 13),
    KC_TYPE_85_1_CLASS = KC_TYPE_85_1 | KC_TYPE_87,
    KC_TYPE_85_2_CLASS = KC_TYPE_85_2 | KC_TYPE_85_3 | KC_TYPE_85_4 | KC_TYPE_85_5,
    KC_TYPE_ALL        = (KC_TYPE_85_1_CLASS |
            KC_TYPE_85_2_CLASS |
            KC_TYPE_LC80 |
            KC_TYPE_Z1013 |
            KC_TYPE_A5105 |
            KC_TYPE_POLY880 |
            KC_TYPE_KRAMERMC |
            KC_TYPE_MUGLERPC |
            KC_TYPE_VCS80 |
            KC_TYPE_C80)
} kc_type_t;

typedef enum {
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
    KC_VARIANT_LC80_1k     = 0,
    KC_VARIANT_LC80_2k     = 1,
    KC_VARIANT_LC80e       = 2,
    KC_VARIANT_A5105_K1505 = 0,
    KC_VARIANT_A5105_A5105 = 1,
    KC_VARIANT_POLY880     = 0,
    KC_VARIANT_POLY880_SC1 = 1,
} kc_variant_t;

class EmulationType;
typedef list<EmulationType *> emulation_type_list_t;

class EmulationType {
private:
    
    string _name;
    string _short_name;
    string _config_name;
    string _icon_name;
    string _image_name;
    string _keyboard_filename;
    string _help_topic;
    string _help_topic_module;
    
    int    _module_slots;

    word_t _power_on_addr;
    word_t _reset_addr;
    
    /** emulation type as given on the commandline, or -1 if no commandline parameter exists */
    int _type;
    
    kc_type_t _kc_type;
        
    static emulation_type_list_t _emulation_type_list;
    
public:
    static EmulationType _emulation_type_kc85_1;
    static EmulationType _emulation_type_kc85_2;
    static EmulationType _emulation_type_kc85_3;
    static EmulationType _emulation_type_kc85_4;
    static EmulationType _emulation_type_kc85_5;
    static EmulationType _emulation_type_kc87;
    static EmulationType _emulation_type_lc80;
    static EmulationType _emulation_type_z1013;
    static EmulationType _emulation_type_a5105;
    static EmulationType _emulation_type_poly880;
    static EmulationType _emulation_type_kramermc;
    static EmulationType _emulation_type_muglerpc;
    static EmulationType _emulation_type_vcs80;
    static EmulationType _emulation_type_c80;
    
    EmulationType(kc_type_t kc_type, int type, string name, string short_name, string config_name, string icon_name, string image_name, string keyboard_filename, string help_topic, string help_topic_module, int module_slots, word_t power_on_addr, word_t reset_addr);
    virtual ~EmulationType(void);
    
    virtual const char * get_name(void) const;
    virtual const char * get_short_name(void) const;
    virtual const char * get_config_name(void) const;
    virtual const char * get_icon_name(void) const;
    virtual const char * get_image_name(void) const;
    virtual const char * get_keyboard_filename(void) const;
    virtual const char * get_help_topic(void) const;
    virtual const char * get_help_topic_module(void) const;
    virtual const int    get_module_slots(void) const;
    virtual const word_t get_power_on_addr(void) const;
    virtual const word_t get_reset_addr(void) const;
    virtual int get_type(void) const;
    virtual kc_type_t get_kc_type(void) const;
    
    static emulation_type_list_t & get_emulation_types(void);
};

class ROMEntry {
private:
    string _filename;
    string _description;
    bool   _is_default;
public:
    ROMEntry(const char *filename, const char *description, bool is_default = false);
    virtual ~ROMEntry(void);
    
    const string get_filename(void) const;
    const string get_description(void) const;
    const bool is_default(void) const;
};

typedef list<ROMEntry *> rom_entry_list_t;

class SystemROM {
private:
    int              _size;
    string           _name;
    bool             _mandatory;
    rom_entry_list_t _roms;

public:
    static const char * ROM_KEY_CAOSC;
    static const char * ROM_KEY_CAOSE;
    static const char * ROM_KEY_SYSTEM;
    static const char * ROM_KEY_SYSTEM1;
    static const char * ROM_KEY_SYSTEM2;
    static const char * ROM_KEY_SYSTEM3;
    static const char * ROM_KEY_SYSTEM4;
    static const char * ROM_KEY_USER;
    static const char * ROM_KEY_BASIC;
    static const char * ROM_KEY_DEBUGGER;
    static const char * ROM_KEY_REASSEMBLER;
    static const char * ROM_KEY_EDITOR;
    static const char * ROM_KEY_ASSEMBLER;
    static const char * ROM_KEY_CHARGEN;
    static const char * ROM_KEY_D004_FC;
    static const char * ROM_KEY_D004_F8;

public:
    SystemROM(const char *name, bool mandatory, int size);
    virtual ~SystemROM(void);

    int get_size(void) const;
    bool is_mandatory(void) const;
    const string get_name(void) const;
    const rom_entry_list_t & get_roms(void) const;
    const ROMEntry * get_default_rom(void) const;

    void add_rom(const char *name, const char *filename, bool is_default);
};

typedef list<SystemROM *> system_rom_list_t;

class SystemType {
    int               _sort;
    int               _type;
    string            _name;
    string            _display_name;
    bool              _is_default;
    kc_variant_t      _kc_variant;
    string            _description;
    EmulationType&    _emulation_type;
    unsigned long long _ui_callback_value;
    unsigned long long _ui_callback_retrace_value;

    string            _rom_dir;
    system_rom_list_t _rom_list;

    virtual void add_rom(const char *name, bool mandatory, int size, int default_idx, const char *filename, const char *description, va_list ap);

public:
    SystemType(int sort, string name, int type, EmulationType &emulation_type, kc_variant_t kc_variant, string description);
    virtual ~SystemType(void);

    virtual const char * get_name(void) const;

    virtual const char * get_display_name(void) const;
    virtual SystemType & set_display_name(const char *display_name);

    virtual const string get_rom_directory(void) const;
    virtual SystemType & set_rom_directory(const char *romdir);

    virtual const unsigned long long get_ui_callback_value(void) const;
    virtual SystemType & set_ui_callback_value(unsigned long long value);

    virtual const unsigned long long get_ui_callback_retrace_value(void) const;
    virtual SystemType & set_ui_callback_retrace_value(unsigned long long value);

    virtual const system_rom_list_t & get_rom_list(void) const;
    virtual const SystemROM * get_rom(const char *key) const;
    virtual SystemType & add_rom(const char *name, int size, const char *filename, const char *description, ...);
    virtual SystemType & add_rom(const char *name, int size, int default_idx, const char *filename, const char *description, ...);
    virtual SystemType & add_optional_rom(const char *name, int size, const char *filename, const char *description, ...);

    virtual int get_sort(void) const;
    virtual int get_type(void) const;
    virtual bool is_default(int type) const;
    
    virtual kc_type_t get_kc_type(void) const;
    virtual kc_variant_t get_kc_variant(void) const;
    virtual const char * get_kc_variant_name(void) const;
    virtual const char * get_description(void) const;
    virtual const EmulationType & get_emulation_type(void) const;
};

typedef list<SystemType *> system_type_list_t;

class SystemInformation {
private:    
    static SystemInformation *_instance;

    system_type_list_t _system_type_list;
    
protected:
    SystemType & add_system_type(int sort, string name, int type, EmulationType& emulation_type, kc_variant_t kc_variant, string description);

public:
    SystemInformation();
    virtual ~SystemInformation(void);
    
    static SystemInformation * instance(void);
    
    virtual void show_types(void);
    virtual void show_types_with_description(void);
    
    virtual system_type_list_t & get_system_types(void);
    virtual const SystemType * get_system_type(kc_type_t kc_type, kc_variant_t kc_variant) const;
};

#endif /* __kc_prefs_types_h */
