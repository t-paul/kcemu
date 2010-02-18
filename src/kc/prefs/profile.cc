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
#include <stdlib.h>
#include <string.h>
#include <typeinfo>

#include "kc/prefs/profile.h"

ProfileValue::ProfileValue(ProfileValue *value) : _level(value->_level), _type(value->_type), _int_value(value->_int_value), _string_value(value->_string_value) {
}

ProfileValue::ProfileValue(profile_level_t level) : _level(level), _type(PROFILE_VALUE_TYPE_NULL), _int_value(0), _string_value("") {
}

ProfileValue::ProfileValue(profile_level_t level, int value) : _level(level), _type(PROFILE_VALUE_TYPE_INT), _int_value(value), _string_value("") {
}

ProfileValue::ProfileValue(profile_level_t level, string value) : _level(level), _type(PROFILE_VALUE_TYPE_STRING), _int_value(0), _string_value(value) {
}

ProfileValue::~ProfileValue(void) {
}

bool
ProfileValue::equals(ProfileValue *value) const {
    if (_type != value->_type)
        return false;
    
    switch (_type) {
        case PROFILE_VALUE_TYPE_NULL:
            return true;
        case PROFILE_VALUE_TYPE_INT:
            return _int_value == value->_int_value;
        case PROFILE_VALUE_TYPE_STRING:
            return _string_value == value->_string_value;
    }
    
    abort();
}

profile_level_t
ProfileValue::get_level(void) const {
    return _level;
}

int
ProfileValue::get_int_value(void) const {
    return _int_value;
}

const char *
ProfileValue::get_string_value(void) const {
    if (_type == PROFILE_VALUE_TYPE_NULL)
        return NULL;
    
    return _string_value.c_str();
}

char *
ProfileValue::get_encoded_value(void) const {
    char buf[2048];
    
    switch (_type) {
        case PROFILE_VALUE_TYPE_NULL:
            return strdup("null");
        case PROFILE_VALUE_TYPE_INT:
            snprintf(buf, sizeof(buf), "%d", _int_value);
            return strdup(buf);
        case PROFILE_VALUE_TYPE_STRING:
            string str;
            str.reserve(_string_value.length() + 10);
            str.push_back('"');
            for (string::const_iterator it = _string_value.begin();it != _string_value.end();it++) {
                char c = (*it);
                if ((c == '"') || (c == '\\'))
                    str.push_back('\\');
                str.push_back(c);
            }
            str.push_back('"');
            return strdup(str.c_str());
    }
    
    abort();
}

Profile::Profile(profile_level_t level, string path, string config_name, string name) {
    _level = level;
    _path = path;
    _name = name;
    _config_name = config_name;
    
    _parent = NULL;
    set_changed(false);
    set_deleted(false);
}

Profile::~Profile(void) {
}

const char *
Profile::get_name(void) const {
    return _name.c_str();
}

void
Profile::set_name(string name) {
    _name = name;
}

const char *
Profile::get_config_name(void) const {
    return _config_name.c_str();
}

profile_level_t
Profile::get_level(void) const {
    return _level;
}

const char *
Profile::get_path(void) const {
    return _path.c_str();
}

bool
Profile::is_changed(void) const {
    return _changed;
}

void
Profile::set_changed(bool changed) {
    if (!changed) {
        for (pref_map_t::iterator it = _pref_map_old.begin();it != _pref_map_old.end();) {
            ProfileValue *value = _pref_map_old[(*it).first];
            
            // Need to increment it before actually erasing the value from the map
            // but pass the old value to the erase call!
            _pref_map_old.erase(it++);
            if (value != NULL) {
                delete value;
            }
        }
    }
    _changed = changed;
}

bool
Profile::is_deleted(void) const
{
    return _deleted;
}

void
Profile::set_deleted(bool deleted)
{
    _deleted = deleted;
    set_changed(true);
}

Profile *
Profile::get_parent(void) const {
    return _parent;
}

void
Profile::set_parent(Profile *parent) {
    _parent = parent;
}

/**
 *  Return the comment depending on the hierarchy level.
 *  The user created profiles do not inherit the comment from the levels above.
 */
const char *
Profile::get_comment(void) const {
    const char *key = "comment";
    if ((get_level() == PROFILE_LEVEL_USER) && (!contains_key(key)))
        return "";

    return get_string_value(key, "");
}

int
Profile::get_int_value(string key, int default_value) const
{
    ProfileValue *value = get_value(key);
    if (value == NULL)
        return default_value;
    
    return value->get_int_value();
}

void
Profile::set_int_value(string key, int value) {
    set_value(key, new ProfileValue(get_level(), value));
}

const char *
Profile::get_string_value(string key, const char *default_value) const
{
    ProfileValue *value = get_value(key);
    if (value == NULL)
        return default_value;
    
    return value->get_string_value();
}

void
Profile::set_string_value(string key, string value) {
    set_value(key, new ProfileValue(get_level(), value));
}

void
Profile::set_null_value(string key) {
    set_value(key, new ProfileValue(get_level()));
}

void
Profile::remove_value(string key) {
    if (!contains_key(key))
        return;
    
    ProfileValue *old_value = _pref_map[key];
    set_value_with_backup(key, NULL, old_value);
}

ProfileValue *
Profile::get_value(string key) const {
    pref_map_t::const_iterator it = _pref_map.find(key);
    if (it == _pref_map.end()) {
        if (_parent == NULL) {
            return NULL;
        } else {
            return _parent->get_value(key);
        }
    }
    return (*it).second;
}

void
Profile::set_value(string key, ProfileValue *value) {
    pref_map_t::iterator it = _pref_map.find(key);

    ProfileValue *old_value = NULL;
    if (it != _pref_map.end()) {
        old_value = (*it).second;
        
        if (old_value->equals(value)) {
            delete value;
            return;
        }
        
        _pref_map.erase(it);
    }
    
    set_value_with_backup(key, value, old_value);
}

/**
 *  set new value and backup the old value if there is not already a backup.
 *  parameter old_value might be NULL if the key was not yet set in profile.
 */
void
Profile::set_value_with_backup(string key, ProfileValue *value, ProfileValue *old_value) {
    pref_map_t::iterator it = _pref_map_old.find(key);
    if (it == _pref_map_old.end()) {
        _pref_map_old[key] = old_value;
    } else {
        delete old_value;
    }

    if (value == NULL) {
        _pref_map.erase(key);
    } else {
        _pref_map[key] = value;
    }
    set_changed(true);
}

list<string> *
Profile::get_keys(void) {
    list<string> *key_list = new list<string>();
    for (pref_map_t::iterator it = _pref_map.begin();it != _pref_map.end();it++)
        key_list->push_back((*it).first);
    
    return key_list;
}

void
Profile::reject_changes(void) {
    for (pref_map_t::iterator it = _pref_map_old.begin();it != _pref_map_old.end();it++) {
        string key = (*it).first;
        ProfileValue *value = _pref_map_old[key];
        if (value == NULL) {
            _pref_map.erase(key);
        } else {
            _pref_map[key] = value;
        }
        _pref_map_old.erase(it);
    }
    set_changed(false);
}

bool
Profile::contains_key(string key) const {
    pref_map_t::const_iterator it = _pref_map.find(key);
    return it != _pref_map.end();
}

char *
Profile::get_encoded_value(string key) const {
    pref_map_t::const_iterator it = _pref_map.find(key);
    if (it == _pref_map.end())
        return NULL;
    
    ProfileValue *value = (*it).second;
    return value->get_encoded_value();
}
