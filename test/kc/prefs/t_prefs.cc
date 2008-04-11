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
