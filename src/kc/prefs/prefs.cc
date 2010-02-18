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

#include <fstream>
#include <algorithm>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/prefs/types.h"
#include "kc/prefs/prefs.h"
#include "kc/prefs/parser.h"
#include "kc/prefs/dirlist.h"
#include "kc/prefs/strlist.h"

#include "sys/sysdep.h"

#include "libdbg/dbg.h"

using namespace std;

struct find_by_type : public unary_function<EmulationType *, void> {
    int _type;
    EmulationType *_result;
    find_by_type(int type) : _type(type), _result(NULL) {}
    void operator()(EmulationType *emulation_type) {
        if ((_result == NULL) && emulation_type->get_type() == _type)
            _result = emulation_type;
    }
};

struct find_by_type_and_variant : public unary_function<SystemType *, void> {
    int _type;
    int _variant;
    SystemType *_result;
    find_by_type_and_variant(int type, int variant) : _type(type), _variant(variant), _result(NULL) {}
    void operator()(SystemType *system) {
        if ((_result == NULL) && (system->get_kc_type() == _type) && (system->get_kc_variant() == _variant))
            _result = system;
    }
};

struct SaveVisitor : ProfileVisitor {
    Preferences *_preferences;
    SaveVisitor(Preferences *preferences) : _preferences(preferences) {}
    void handle_profile(Profile *profile) {
        _preferences->save_profile(profile);
    }
};

struct RejectVisitor : ProfileVisitor {
    Preferences *_preferences;
    RejectVisitor(Preferences *preferences) : _preferences(preferences) {}
    void handle_profile(Profile *profile) {
        _preferences->reject_changes(profile);
    }
};

Preferences *Preferences::_instance = 0;

const char *Preferences::PROFILE_NAME_ROOT = "root";
const char *Preferences::PROFILE_NAME_DEFAULT = "default";
const char *Preferences::PROFILE_KEY_NAME = "name";
const char *Preferences::PROFILE_KEY_SYSTEM = "system";
const char *Preferences::PROFILE_KEY_VARIANT = "variant";
const char *Preferences::CONFIG_FILE_EXTENSION = ".conf";
const char *Preferences::USER_CONFIG_PREFIX = "_";

Preferences::Preferences(const char *sys_dir, const char *usr_dir, const char *add_dir) : _sys_dir(sys_dir), _usr_dir(usr_dir), _add_dir(add_dir) {
    load_default_profiles();
    load_system_profiles();
    load_user_profiles();
}

void
Preferences::load_default_profiles(void)
{
    _root_profile = get_profile(get_profile_path(_sys_dir, PROFILE_NAME_DEFAULT), PROFILE_LEVEL_ROOT, PROFILE_NAME_DEFAULT, PROFILE_NAME_ROOT);
    _default_profile = get_profile(get_profile_path(_usr_dir, PROFILE_NAME_DEFAULT), PROFILE_LEVEL_DEFAULT, PROFILE_NAME_DEFAULT, PROFILE_NAME_DEFAULT);
    
    _default_profile->set_parent(_root_profile);
    _root_profile->set_changed(false);
    _default_profile->set_changed(false);
}

void
Preferences::load_system_profiles(void)
{
    emulation_type_list_t list = EmulationType::get_emulation_types();
    for (emulation_type_list_t::iterator it = list.begin();it != list.end();it++) {
        string config_name = (*it)->get_config_name();
        
        Profile *sys_profile = get_profile(get_profile_path(_sys_dir, config_name), PROFILE_LEVEL_SYSTEM, config_name, (*it)->get_name());
        Profile *usr_profile = get_profile(get_profile_path(_usr_dir, config_name), PROFILE_LEVEL_SYSTEM_USER, config_name, (*it)->get_name());
        
        sys_profile->set_int_value(PROFILE_KEY_SYSTEM, (*it)->get_kc_type());
        usr_profile->set_int_value(PROFILE_KEY_SYSTEM, (*it)->get_kc_type());
        
        sys_profile->set_parent(_default_profile);
        usr_profile->set_parent(sys_profile);
        
        sys_profile->set_changed(false);
        usr_profile->set_changed(false);
        
        _sys_profiles[config_name] = sys_profile;
        _usr_profiles[config_name] = usr_profile;
        
        _usr_profiles_by_type[(*it)->get_kc_type()]  = usr_profile;
    }
}

void
Preferences::load_user_profiles(void)
{
    DirectoryList dir(_add_dir);
    for (DirectoryList::iterator it = dir.begin();it != dir.end();it++) {
        string config_name = string(USER_CONFIG_PREFIX) + (*it)->get_filename();
        Profile *profile = get_profile((*it)->get_path(), PROFILE_LEVEL_USER, config_name, (*it)->get_filename());
        
        kc_type_t kc_type = (kc_type_t)profile->get_int_value(PROFILE_KEY_SYSTEM, KC_TYPE_NONE);
        map<kc_type_t, Profile *>::iterator i = _usr_profiles_by_type.find(kc_type);
        if (i == _usr_profiles_by_type.end()) {
            delete profile;
            continue;
        }
        
        if (profile->contains_key(PROFILE_KEY_NAME)) {
            const char *name = profile->get_string_value(PROFILE_KEY_NAME, "");
            if (strlen(name) > 0)
                profile->set_name(name);
        }
        
        profile->set_parent((*i).second);
        profile->set_changed(false);
        _add_profiles[config_name] = profile;
    }
}

string
Preferences::get_profile_path(string dir, string config_name) {
    string path = dir + "/" + config_name + CONFIG_FILE_EXTENSION;
    return path;
}

Profile *
Preferences::get_profile(string path, profile_level_t level, string config_name, string name) {
    ProfileParser *parser = new SingleProfileParser(path.c_str(), level, config_name, name);
    parser->parse();
    profile_list_t *list = parser->get_profiles();
    assert(list->size() == 1);
    Profile *profile = list->front();
    delete parser;
    return profile;
}

Preferences::~Preferences(void) {
}

void
Preferences::init(const char *system_dir, const char *user_dir, const char *add_dir) {
    if (_instance == 0)
        _instance = new Preferences(system_dir, user_dir, add_dir);
}

Preferences *
Preferences::instance(void) {
    if (_instance == 0) {
        cerr << "Preferences::instance(void): Preferences::init() not called!" << endl;
        exit(1);
    }
    return _instance;
}

Profile *
Preferences::find_profile(const char *key) {
    if (strcmp(key, PROFILE_NAME_DEFAULT) == 0)
        return _default_profile;
    
    profile_map_t::iterator it1 = _usr_profiles.find(key);
    if (it1 != _usr_profiles.end())
        return (*it1).second;
        
    profile_map_t::iterator it2 = _add_profiles.find(key);
    if (it2 != _add_profiles.end())
        return (*it2).second;

    return NULL;
}

Profile *
Preferences::find_profile_by_name(const char *name) {
    if (strcmp(name, PROFILE_NAME_DEFAULT) == 0)
        return _default_profile;

    for (profile_map_t::iterator it1 = _usr_profiles.begin();it1 != _usr_profiles.end();it1++) {
        if (strcmp(name, (*it1).second->get_name()) == 0)
            return (*it1).second;
    }

    for (profile_map_t::iterator it2 = _add_profiles.begin();it2 != _add_profiles.end();it2++) {
        if (strcmp(name, (*it2).second->get_name()) == 0)
            return (*it2).second;
    }
    
    return NULL;
}

list<Profile *>
Preferences::find_child_profiles(const char *key) {
    list<Profile *> child_profiles;
    for (profile_map_t::iterator it = _add_profiles.begin();it != _add_profiles.end();it++) {
        Profile *child_profile = (*it).second;
        const char *config_name = child_profile->get_parent()->get_config_name();
        if (strcmp(key, config_name) != 0)
            continue;
        
        child_profiles.push_back(child_profile);
    }
    return child_profiles;
}

Profile *
Preferences::create_user_profile(Profile *parent) {
    char name[1024];
    struct stat statbuf;
    
    if (parent == NULL)
        return NULL;
    
    kc_type_t kc_type = (kc_type_t)parent->get_int_value(PROFILE_KEY_SYSTEM, KC_TYPE_NONE);
    if (kc_type == KC_TYPE_NONE)
        return NULL;
    
    for (int a = 0;a < 10000;a++) {
        snprintf(name, sizeof(name), "profile_%04d", a);
        string config_name = string(name) + CONFIG_FILE_EXTENSION;
        string path = _add_dir + "/" + config_name;

        if (Preferences::instance()->find_profile(config_name.c_str()) != NULL)
            continue;
        
        if (stat(path.c_str(), &statbuf) == 0)
            continue;

        Profile *profile = new Profile(PROFILE_LEVEL_USER, path, config_name, name);
        profile->set_int_value(PROFILE_KEY_SYSTEM, kc_type);
        profile->set_parent(parent);
        _add_profiles[config_name] = profile;
        return profile;
    }
    
    return NULL;
}

Profile *
Preferences::copy_user_profile(Profile *profile)
{
    Profile *copy = create_user_profile(profile->get_parent());
    if (copy == NULL)
        return NULL;

    list<string> *keys = profile->get_keys();
    for (list<string>::iterator it = keys->begin();it != keys->end();it++) {
        const char *key = (*it).c_str();
        if (strcmp(key, PROFILE_KEY_NAME) == 0)
            continue;
        
        char *encoded_value = profile->get_encoded_value(key);
        copy->set_value(key, new ProfileValue(profile->get_value(key)));
        free(encoded_value);
    }
    
    return copy;
}

bool
Preferences::has_changed_profiles(void) {
    if (_default_profile->is_changed())
        return true;
    
    for (profile_map_t::iterator it = _usr_profiles.begin();it != _usr_profiles.end();it++)
        if ((*it).second->is_changed())
            return true;

    for (profile_map_t::iterator it = _add_profiles.begin();it != _add_profiles.end();it++)
        if ((*it).second->is_changed())
            return true;
        
    return false;
}

void
Preferences::visit_changed(ProfileVisitor &visitor) {
    DBG(1, form("KCemu/Preferences",
                "Preferences::visit_changed(): Start visiting changed profiles:\n"));

    if (_default_profile->is_changed()) {
        DBG(1, form("KCemu/Preferences",
                    "Preferences::visit_changed(): ... default profile changed\n"));
        visitor.handle_profile(_default_profile);
    }
    
    for (profile_map_t::iterator it = _usr_profiles.begin();it != _usr_profiles.end();it++) {
        Profile *profile = (*it).second;
        if (profile->is_changed()) {
            DBG(1, form("KCemu/Preferences",
                        "Preferences::visit_changed(): ... system profile '%s' changed\n",
                        profile->get_name()));
            visitor.handle_profile(profile);
        }
    }
    
    for (profile_map_t::iterator it = _add_profiles.begin();it != _add_profiles.end();it++) {
        Profile *profile = (*it).second;
        if (profile->is_changed()) {
            DBG(1, form("KCemu/Preferences",
                        "Preferences::visit_changed(): ... user profile '%s' changed\n",
                        profile->get_name()));
            visitor.handle_profile(profile);
        }
    }

    DBG(1, form("KCemu/Preferences",
                "Preferences::visit_changed(): End visiting changed profiles.\n"));
}

void
Preferences::save(void) {
    DBG(1, form("KCemu/Preferences",
                "Preferences::save(): Start save profiles:\n"));

    mkdirs(_usr_dir);
    mkdirs(_add_dir);

    SaveVisitor visitor(this);
    visit_changed(visitor);

    DBG(1, form("KCemu/Preferences",
                "Preferences::save(): End save profiles.\n"));
}

void
Preferences::reject(void) {
    DBG(1, form("KCemu/Preferences",
                "Preferences::reject(): Start rejecting changes\n"));
    
    RejectVisitor visitor(this);
    visit_changed(visitor);

    DBG(1, form("KCemu/Preferences",
                "Preferences::reject(): End rejecting changes\n"));
}

bool
Preferences::mkdirs(string dir) {
    struct stat statbuf;
    
    if (stat(dir.c_str(), &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
            DBG(1, form("KCemu/Preferences",
                   "Preferences::mkdirs(): Directory '%s' exists.\n",
                   dir.c_str()));
            return true;
        }
    }

    DBG(1, form("KCemu/Preferences",
           "Preferences::mkdirs(): Directory '%s' doesn't exist, trying to create...\n",
           dir.c_str()));

    StringList dirs(dir, '/');

    string path;
    for (StringList::const_iterator it = dirs.begin();it != dirs.end();it++) {
	string entry = (*it);

	if ((path.size() == 0) && (entry.size() == 2) && (entry[1] == ':')) {
	    DBG(1, form("KCemu/Preferences",
	   		"Preferences::mkdirs(): Detected windows style path, skipping drive letter '%s'...\n",
			entry.c_str()));
	    path += entry;
	    continue;
	}

        path += "/";
        path += entry;

        if (stat(path.c_str(), &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                DBG(1, form("KCemu/Preferences",
                       "Preferences::mkdirs(): .. Directory '%s' already exists.\n",
                       path.c_str()));
                continue;
            } else {
                DBG(1, form("KCemu/Preferences",
                       "Preferences::mkdirs(): Path '%s' already exists, but it is not a directory. Create failed!\n",
                       path.c_str()));
                return false;
            }
        }

        if (sys_mkdir(path.c_str(), 0700) != 0) {
            DBG(1, form("KCemu/Preferences",
                   "Preferences::mkdirs(): Could not create directory '%s' (%s)!\n",
                   path.c_str(), strerror(errno)));
            return false;
        }

        DBG(1, form("KCemu/Preferences",
               "Preferences::mkdirs(): .. Created directory '%s'.\n",
               path.c_str()));
    }
    
    DBG(1, form("KCemu/Preferences",
           "Preferences::mkdirs(): Done.\n"));

    return true;
}

bool
Preferences::save_profile(Profile *profile) {
    dump_profile(profile);
    
    if (profile->is_deleted()) {
        DBG(1, form("KCemu/Preferences",
                    "Preferences::save_profile(): Deleting profile '%s'\n",
                    profile->get_name()));
        if (unlink(profile->get_path()) != 0) {
            DBG(1, form("KCemu/Preferences",
                        "Preferences::save_profile(): Could not delete file '%s' (%s)!\n",
                        profile->get_path(), strerror(errno)));
        }
        if (_add_profiles.erase(profile->get_config_name()) == 1) {
            delete profile;
            return true;
        }
        return false;
    }
    
    ofstream os(profile->get_path(), ios_base::trunc);
    if (!os) {
        DBG(1, form("KCemu/Preferences",
                    "Preferences::save_profile(): could not create file '%s'\n",
                    profile->get_path()));
        return false;
    }
    
    os << ProfileParser::signature << endl;
    
    list<string> *keys = profile->get_keys();
    for (list<string>::iterator it = keys->begin();it != keys->end();it++) {
        char *value = profile->get_encoded_value(*it);
        os << (*it).c_str() << " = " << value << endl;
        free(value);
    }
    profile->set_changed(false);
    
    return true;
}

void
Preferences::reject_changes(Profile *profile) {
    profile->reject_changes();
}

void
Preferences::dump_profile(Profile *profile) {
    DBG(1, form("KCemu/Preferences",
                "Preferences::dump_profile(): dumping profile '%s' (flags: %c%c)\n",
                profile->get_name(),
                profile->is_changed() ? 'C' : 'c',
                profile->is_deleted() ? 'D' : 'd'));
    
    string path = get_profile_path(_usr_dir, profile->get_config_name());
    DBG(1, form("KCemu/Preferences",
                "Preferences::dump_profile(): path is '%s'\n",
                path.c_str()));
    
    list<string> *keys = profile->get_keys();
    for (list<string>::iterator it = keys->begin();it != keys->end();it++) {
        char *value = profile->get_encoded_value(*it);
        DBG(1, form("KCemu/Preferences",
                    "Preferences::dump_profile(): .. %s = %s\n",
                    (*it).c_str(), value));
        free(value);
    }
    DBG(1, form("KCemu/Preferences",
                "Preferences::dump_profile(): end.\n"));
}

/**
 *  find the profile to select as current profile:
 *
 *  1.) check for the given name as config_name in the profile
 *  2.) check for the given name as display name of the profile
 *  3.) check for the profile matching the given type
 *  4.) revert to "default" profile
 */
void
Preferences::set_current_profile(const char *name, int type) {
    Profile *profile = NULL;
    
    if (name != NULL) {
        profile = Preferences::instance()->find_profile(name);
    }
    if ((profile == NULL) && (name != NULL)) {
        profile = Preferences::instance()->find_profile_by_name(name);
    }
    if ((profile == NULL) && (type >= 0)) {
        emulation_type_list_t& list = EmulationType::get_emulation_types();
        find_by_type ft = for_each(list.begin(), list.end(), find_by_type(type));
        if (ft._result != NULL) {
            profile = Preferences::instance()->find_profile(ft._result->get_config_name());
        }
    }
    if (profile == NULL) {
        profile = Preferences::instance()->find_profile(PROFILE_NAME_DEFAULT);
    }
    
    if (profile != NULL) {
        kc_type_t kc_type = (kc_type_t)profile->get_int_value(PROFILE_KEY_SYSTEM, -1);
        kc_variant_t kc_variant = (kc_variant_t)profile->get_int_value(PROFILE_KEY_VARIANT, 0);
        
        SystemType *system_type = find_system_type(kc_type, kc_variant);
        if (system_type == NULL)
            system_type = find_system_type(kc_type, KC_VARIANT_NONE);
        if (system_type == NULL)
            system_type = find_system_type(KC_TYPE_85_4, KC_VARIANT_NONE);
        
        if (system_type != NULL) {
            _current_profile = profile;
            _current_system_type = system_type;
            return;
        }
    }

    abort();
}

SystemType *
Preferences::find_system_type(kc_type_t kc_type, kc_variant_t kc_variant) {
    system_type_list_t& list = SystemInformation::instance()->get_system_types();
    find_by_type_and_variant f = for_each(list.begin(), list.end(), find_by_type_and_variant(kc_type, kc_variant));
    return f._result;
}

int
Preferences::get_int_value(string key, int default_value) {
    return _current_profile->get_int_value(key, default_value);
}

const char *
Preferences::get_string_value(string key, const char *default_value) {
    return _current_profile->get_string_value(key, default_value);
}

SystemType *
Preferences::get_system_type(void) {
    return _current_system_type;
}

kc_type_t
Preferences::get_kc_type(void) {
    return _current_system_type->get_kc_type();
}

kc_variant_t
Preferences::get_kc_variant(void) {
    return _current_system_type->get_kc_variant();
}

const char *
Preferences::get_kc_type_name(void) {
    return _current_system_type->get_emulation_type().get_name();
}

const char *
Preferences::get_kc_variant_name(void) {
    return _current_system_type->get_kc_variant_name();
}
