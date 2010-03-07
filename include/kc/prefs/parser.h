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

#ifndef __kc_prefs_parser_h
#define __kc_prefs_parser_h

#include <list>
#include <string>

#include <glib.h>

#include "kc/prefs/profile.h"

using namespace std;

typedef list<Profile *> profile_list_t;

class ProfileParser
{
public:
  static const char *signature;
  
private:    
  Profile *_current_profile;

protected:
  profile_level_t _level;
  string _filename;
  profile_list_t _profile_list;

protected:
  virtual bool is_newline(GScanner *scanner, GTokenType token_type);
  virtual GTokenType handle_line(GScanner *scanner, const char *key, GTokenType token_type);

public:
  ProfileParser(string filename, profile_level_t level);
  virtual ~ProfileParser(void);

  virtual void parse(void);
  virtual profile_list_t * get_profiles(void);

  virtual Profile * get_profile(void) = 0;
};

class SingleProfileParser : public ProfileParser
{
 private:
  string _name;
  Profile *_profile;
  
 public:
  SingleProfileParser(string filename, profile_level_t level, string config_name, string name);
  virtual ~SingleProfileParser(void);

  virtual Profile * get_profile(void);
};

class MultiProfileParser : public ProfileParser
{
 public:
  MultiProfileParser(string filename, profile_level_t level);
  virtual ~MultiProfileParser(void);

  virtual Profile * get_profile(void);
};

#endif /* __kc_prefs_parser_h */
