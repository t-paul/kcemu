#include <stdio.h>

static int verbose = 0;

int
check(const char *filename)
{
  int c;
  int crc;
  int last;
  int bytes;
  FILE *f;

  f = fopen(filename, "rb");
  if (f == NULL)
    return 0;

  printf("\n* checking file '%s'...\n", filename);

  crc = 0;
  last = 0;
  bytes = 0;
  while (242)
    {
      c = fgetc(f);
      if (c == EOF)
	break;

      bytes++;
      crc ^= (0x80 ^ last);
      last = c;

      if (verbose > 0)
	printf("  %8d, %02x, %02x\n", bytes, c, crc ^ c);
    }

  printf("  %d bytes.\n", bytes);
  if (crc == last)
    printf("  CRC (%02x) ok.\n", crc);
  else
    printf("  CRC ERROR: file crc is %02x, calculated %02x!\n", last, crc);

  return crc == last;
}

int
main(int argc, char **argv)
{
  int a;

  if (argc == 1)
    {
      printf("usage: %s file [file ...]\n", argv[0]);
      exit(1);
    }

  for (a = 1;a < argc;a++)
    {
      if (strcmp("-v", argv[a]) == 0)
	{
	  verbose = 1;
	  continue;
	}

      if (check(argv[a]))
	printf("  all ok.\n");
      else
	printf("  has errors!\n");
    }

  return 0;
}
