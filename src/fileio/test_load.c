#include <stdio.h>

#include "fileio/load.h"

#define FILE_TAP  "/home/tp/projects/KCemu-data/KC-Office/wp1990/wp1990.tap"
#define FILE_GPF  "/home/tp/projects/kc85/emu/PRG/ladder2.gpf"
#define FILE_IMG  "/home/tp/projects/KCemu/PRG/BENNION.tape"
#define FILE_BAS  "/home/tp/projects/KCemu/PRG/kc-basic-programs/labyrint"
#define FILE_PBAS "/home/tp/projects/KCemu/PRG/ts+ve.prg"
#define FILE_XXX  "/home/tp/projects/KCemu/PRG/bennion_rbs_0x0200_0x46c0.prg"

int
main(int argc, char **argv)
{
  int a, ret;
  fileio_prop_t *prop;

  fileio_init();

  if (argc > 1)
    {
      for (a = 1;a < argc;a++)
        {
          ret = fileio_load_file(argv[a], &prop);
          fileio_debug_dump(prop, 1);
        }
    }
  else
    {
      ret = fileio_load_file(FILE_TAP, &prop);
      fileio_debug_dump(prop, 1);
      ret = fileio_load_file(FILE_GPF, &prop);
      fileio_debug_dump(prop, 1);
      ret = fileio_load_file(FILE_IMG, &prop);
      fileio_debug_dump(prop, 1);
      ret = fileio_load_file(FILE_BAS, &prop);
      fileio_debug_dump(prop, 1);
      ret = fileio_load_file(FILE_PBAS, &prop);
      fileio_debug_dump(prop, 1);
      ret = fileio_load_file(FILE_XXX, &prop);
      fileio_debug_dump(prop, 1);
    }

  return 0;
}
