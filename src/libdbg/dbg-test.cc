#define DBG_LEVEL 3
#include "libdbg/dbg.h"

int
main(void)
{
  DBG_add_path("test/path/s1");
  cout << *DBGI();
  DBG_add_path("test/path/s2");
  cout << *DBGI();

  DBG_add_path("test/path2/s3");
  cout << *DBGI();

  DBG(0, form("test/path/s1", "%s -> %d\n", "hallo", 3));
  DBG(1, form("test/path/s4", "%s -> %d\n", "hallo", 3));
}
