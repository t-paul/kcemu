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

#ifndef __kc_prefs_prefs_h
#define __kc_prefs_prefs_h

#include <map>
#include <string>

#include "kc/prefs/types.h"
#include "kc/prefs/profile.h"

using namespace std;

struct ProfileVisitor {
    virtual void handle_profile(Profile *profile) = 0;
};

class Preferences {
private:
    typedef map<string, Profile *> profile_map_t;

    static Preferences *_instance;

    static const char * PROFILE_NAME_ROOT;
    static const char * PROFILE_NAME_DEFAULT;
    static const char * PROFILE_KEY_NAME;
    static const char * PROFILE_KEY_SYSTEM;
    static const char * PROFILE_KEY_VARIANT;
    static const char * CONFIG_FILE_EXTENSION;
    static const char * USER_CONFIG_PREFIX;
    
    string _sys_dir;
    string _usr_dir;
    string _add_dir;
    
    Profile *_root_profile;
    Profile *_default_profile;
    
    profile_map_t _sys_profiles;
    profile_map_t _usr_profiles;
    profile_map_t _add_profiles;
    
    map<kc_type_t, Profile *> _usr_profiles_by_type;
    
    Profile *_current_profile;
    SystemType *_current_system_type;

    void load_default_profiles(void);
    void load_system_profiles(void);
    void load_user_profiles(void);
    
protected:
    Preferences(const char *sys_dir, const char *usr_dir, const char *add_dir);
    virtual ~Preferences(void);
    
    void visit_changed(ProfileVisitor &visitor);

    string get_profile_path(string dir, string config_name);
    Profile * get_profile(string path, profile_level_t level, string config_name, string name);
    
public:
    static void init(const char *system_dir, const char *user_dir, const char *add_dir);
    static Preferences * instance(void);
    
    Profile * find_profile(const char *key);
    Profile * find_profile_by_name(const char *name);
    list<Profile *> find_child_profiles(const char *key);
    Profile * copy_user_profile(Profile *profile);
    Profile * create_user_profile(Profile *parent);
    
    void save(void);
    void reject(void);
    bool mkdirs(string dir);
    bool has_changed_profiles(void);
    bool save_profile(Profile *profile);
    void reject_changes(Profile *profile);
    void dump_profile(Profile *profile);
    
    virtual void set_current_profile(const char *name, int type);
    virtual SystemType * find_system_type(kc_type_t kc_type, kc_variant_t kc_variant);

    virtual SystemType * get_system_type(void);
    virtual kc_type_t get_kc_type(void);
    virtual kc_variant_t get_kc_variant(void);
    virtual const char * get_kc_type_name(void);
    virtual const char * get_kc_variant_name(void);

    virtual int get_int_value(string key, int default_value);
    virtual const char * get_string_value(string key, const char *default_value);
};

#endif /* __kc_prefs_prefs_h */
