#include <stdio.h>
#include <string.h>

int
check_file_type(char *name, char *type, unsigned char *buf)
{
  int a;
  const char *t = NULL;

  if ((buf[1] == 0xd3) && (buf[2] == 0xd3) && (buf[3] == 0xd3))
    {
      t = "BASIC";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[1] == 0xd4) && (buf[2] == 0xd4) && (buf[3] == 0xd4))
    {
      t = "DATA";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[1] == 0xd5) && (buf[2] == 0xd5) && (buf[3] == 0xd5))
    {
      t = "LIST";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[1] == 0xd7) && (buf[2] == 0xd7) && (buf[3] == 0xd7))
    {
      t = "BASIC*";
      memcpy(name, &buf[4], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'C') && (buf[10] == 'O') && (buf[11] == 'M'))
    {
      t = "COM";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'A') && (buf[10] == 'S') && (buf[11] == 'M'))
    {
      t = "ASM";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'T') && (buf[10] == 'X') && (buf[11] == 'T'))
    {
      t = "TXT";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'P') && (buf[10] == 'A') && (buf[11] == 'S'))
    {
      t = "PAS";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == 'S') && (buf[10] == 'C') && (buf[11] == 'R'))
    {
      t = "SCR";
      memcpy(name, &buf[1], 8);
      name[8] = '\0';
    }
  if ((buf[9] == '\0') && (buf[10] == '\0') && (buf[11] == '\0'))
    if (strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", buf[1]) != NULL)
      {
	t = "(none)";
	memcpy(name, &buf[1], 8);
	name[8] = '\0';
      }
  
  if (t == NULL)
    return 0;

  for (a = 7;(a > 0) && name[a] == ' ';a--)
    name[a] = '\0';

  strcpy(type, t);
  return 1;
}
