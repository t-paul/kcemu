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

#include "sys/sysdep.h"
#include "kc/prefs/prefs.h"

#include "libdbg/dbg.h"

using namespace std;

int
main(void) {
  string home = sys_gethome();
  string sys_config_dir = home + "/sys";
  string user_config_dir = home + "/config";
  string user_profiles_dir = home + "/profiles";

  DBGI()->add_path("KCemu/Preferences", true);

  Preferences::init(sys_config_dir.c_str(), user_config_dir.c_str(), user_profiles_dir.c_str());
  Preferences::instance()->set_current_profile("kc85-4", 0);

  Profile *p = Preferences::instance()->find_profile("kc85-4");
  if (p == NULL)
    return 1;

  p->set_int_value("test", 1);
  Preferences::instance()->save();

  return 0;
}
