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

#ifndef __kc_prefs_profile_h
#define __kc_prefs_profile_h

#include <map>
#include <list>
#include <string>

using namespace std;

typedef enum {
    PROFILE_LEVEL_ROOT = 0,
    PROFILE_LEVEL_DEFAULT = 1,
    PROFILE_LEVEL_SYSTEM = 2,
    PROFILE_LEVEL_SYSTEM_USER = 3,
    PROFILE_LEVEL_USER = 4,
} profile_level_t;

class ProfileValue {
private:
    typedef enum {
        PROFILE_VALUE_TYPE_NULL = 0,
        PROFILE_VALUE_TYPE_INT = 1,
        PROFILE_VALUE_TYPE_STRING = 2,
    } profile_value_type_t;
    
    profile_level_t _level;
    profile_value_type_t _type;
    int _int_value;
    string _string_value;
    
public:
    ProfileValue(ProfileValue *value);
    ProfileValue(profile_level_t level);
    ProfileValue(profile_level_t level, int value);
    ProfileValue(profile_level_t level, string value);
    virtual ~ProfileValue(void);
    
    virtual profile_level_t get_level(void) const;
    
    bool equals(ProfileValue *value) const;
    
    virtual int get_int_value(void) const;
    virtual const char * get_string_value(void) const;
    virtual char * get_encoded_value(void) const;
};

class Profile {
private:
    typedef map<string, ProfileValue *> pref_map_t;

    bool _changed;
    bool _deleted;
    profile_level_t _level;
    string _name;
    string _config_name;
    string _path;
    Profile * _parent;
    pref_map_t _pref_map;
    pref_map_t _pref_map_old;

protected:
    void set_value_with_backup(string key, ProfileValue *value, ProfileValue *old_value);

public:
    Profile(profile_level_t level, string path, string config_name, string name);
    virtual ~Profile(void);
    
    const char * get_name(void) const;
    void set_name(string name);
    
    const char * get_config_name(void) const;
    
    profile_level_t get_level(void) const;
    const char *get_path(void) const;
    
    bool is_changed(void) const;
    void set_changed(bool changed);
    
    bool is_deleted(void) const;
    void set_deleted(bool deleted);
    
    Profile *get_parent(void) const;
    void set_parent(Profile *parent);
    
    const char * get_comment(void) const;

    ProfileValue * get_value(string key) const;
    char * get_encoded_value(string key) const;
    void set_value(string key, ProfileValue *value);

    void reject_changes(void);
    
    int get_int_value(string key, int default_value) const;
    void set_int_value(string key, int value);

    const char * get_string_value(string key, const char *default_value) const;
    void set_string_value(string key, string value);
    
    void set_null_value(string key);
    void remove_value(string key);
    bool contains_key(string key) const;
    list<string> * get_keys(void);
};

#endif /* __kc_prefs_profile_h */
