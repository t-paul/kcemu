/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2007 Torsten Paul
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

#ifndef __kc_prefs_types_h
#define __kc_prefs_types_h

#include <list>
#include <string>

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
    
    EmulationType(kc_type_t kc_type, int type, string name, string short_name, string config_name, string icon_name, string image_name);
    virtual ~EmulationType(void);
    
    virtual const char * get_name(void) const;
    virtual const char * get_short_name(void) const;
    virtual const char * get_config_name(void) const;
    virtual const char * get_icon_name(void) const;
    virtual const char * get_image_name(void) const;
    virtual int get_type(void) const;
    virtual kc_type_t get_kc_type(void) const;
    
    static emulation_type_list_t & get_emulation_types(void);
};

class SystemType {
    int           _sort;
    int           _type;
    string        _name;
    
public:
    SystemType(int sort, int type, string name);
    virtual ~SystemType(void);
    
    virtual const char *get_name(void);
    
    virtual int get_sort(void);
    virtual int get_type(void);
    virtual bool is_default(int type);
    
    virtual kc_type_t get_kc_type(void) = 0;
    virtual kc_variant_t get_kc_variant(void) = 0;
    virtual const char * get_kc_variant_name(void) = 0;
    virtual const char * get_description(void) = 0;
    virtual const EmulationType & get_emulation_type(void) = 0;
};

class RealSystemType : public SystemType {
private:
    bool           _is_default;
    kc_variant_t   _kc_variant;
    string         _description;
    EmulationType& _emulation_type;

public:
    RealSystemType(int sort, string name, int type, EmulationType& emulation_type, kc_variant_t kc_variant, string description);
    virtual ~RealSystemType(void);
    
    virtual kc_type_t get_kc_type(void);
    virtual kc_variant_t get_kc_variant(void);
    virtual const char * get_kc_variant_name(void);
    virtual const char * get_description(void);
    virtual const EmulationType & get_emulation_type(void);
};

class AliasSystemType : public SystemType {
private:
    string _name;
    SystemType *_base;
    
public:
    AliasSystemType(int sort, string name, int type, SystemType *base_system_type);
    virtual ~AliasSystemType(void);
    
    virtual kc_type_t get_kc_type(void);
    virtual kc_variant_t get_kc_variant(void);
    virtual const char * get_kc_variant_name(void);
    virtual const char * get_description(void);
    virtual const EmulationType & get_emulation_type(void);
};

typedef list<SystemType *> system_type_list_t;

class SystemInformation {
private:    
    static SystemInformation *_instance;

    system_type_list_t _system_type_list;
    
protected:
    void add_alias_type(int sort, string name, int type, string base_system_type_name);
    void add_system_type(int sort, string name, int type, EmulationType& emulation_type, kc_variant_t kc_variant, string description);

public:
    SystemInformation();
    virtual ~SystemInformation(void);
    
    static SystemInformation * instance(void);
    
    virtual void show_types(void);
    virtual void show_types_with_description(void);
    
    virtual system_type_list_t & get_system_types(void);
};

#endif /* __kc_prefs_types_h */
