#include <stdio.h>
#include <stdlib.h>

#include <ui/font1.h>

int
reverse(int byte)
{
  int a, ret;

  ret = 0;
  for (a = 0;a < 8;a++)
    {
      ret <<= 1;
      ret |= byte & 1;
      byte >>= 1;
    }

  return ret;
}

void
print_char(int c)
{
  int a;

  printf("STARTCHAR char%d\n", c);
  printf("ENCODING %d\n", c);
  printf("SWIDTH 1000 0\n");
  printf("DWIDTH 8 0\n");
  printf("BBX 8 8 0 -2\n");
  printf("BITMAP\n");
  for (a = 0;a < 8;a++)
    printf("%02X\n", reverse(__font[8 * c + a]));
  printf("ENDCHAR\n");
}

int
main(int argc, char **argv)
{
  int a, size, b1, b2;

  size = 8;
  if (argc >= 2)
    size = atoi(argv[1]);
  if (size < 8)
    size = 8;

  b1 = (size - 8) / 2;
  b2 = (size - 8) - b1;

  printf("STARTFONT 2.1\n"
	 "FONT kc87\n"
	 "SIZE %d 72 72\n"
	 "FONTBOUNDINGBOX 8 %d 0 -2\n"
	 "STARTPROPERTIES 9\n"
	 "POINT_SIZE 8\n"
	 "PIXEL_SIZE 1\n"
	 "RESOLUTION_X 72\n"
	 "RESOLUTION_Y 72\n"
	 "FONT_ASCENT %d\n"
	 "FONT_DESCENT %d\n"
	 "AVERAGE_WIDTH 8\n"
	 "SPACING \"M\"\n"
	 "DEFAULT_CHAR 32\n"
	 "ENDPROPERTIES\n"
	 "CHARS 256\n",
	 size, size, 7 + b1, 1 + b2);

  for (a = 0;a < 256;a++)
    print_char(a);

  printf("ENDFONT\n");

  return 0;
}
