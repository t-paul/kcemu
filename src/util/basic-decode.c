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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fileio/load.h>

static int _binary = 0;
static int _verbose = 0;

static struct token
{
  int val;
  const char *text;
} token[] = {
  { 0x80, "END" },
  { 0x81, "FOR" },
  { 0x82, "NEXT" },
  { 0x83, "DATA" },
  { 0x84, "INPUT" },
  { 0x85, "DIM" },
  { 0x86, "READ" },
  { 0x87, "LET" },
  { 0x88, "GOTO" },
  { 0x89, "RUN" },
  { 0x8a, "IF" },
  { 0x8b, "RESTORE" },
  { 0x8c, "GOSUB" },
  { 0x8d, "RETURN" },
  { 0x8e, "REM" },
  { 0x8f, "STOP" },
  { 0x90, "OUT" },
  { 0x91, "ON" },
  { 0x92, "NULL" },
  { 0x93, "WAIT" },
  { 0x94, "DEF" },
  { 0x95, "POKE" },
  { 0x96, "DOKE" },
  { 0x97, "AUTO" },
  { 0x98, "LINES" },
  { 0x99, "CLS" },
  { 0x9a, "WIDTH" },
  { 0x9b, "BYE" },
  { 0x9c, "!" },
  { 0x9d, "CALL" },
  { 0x9e, "PRINT" },
  { 0x9f, "CONT" },
  { 0xa0, "LIST" },
  { 0xa1, "CLEAR" },
  { 0xa2, "CLOAD" },
  { 0xa3, "CSAVE" },
  { 0xa4, "NEW" },
  { 0xa5, "TAB(" },
  { 0xa6, "TO" },
  { 0xa7, "FN" },
  { 0xa8, "SPC(" },
  { 0xa9, "THEN" },
  { 0xaa, "NOT" },
  { 0xab, "STEP" },
  { 0xac, "+" },
  { 0xad, "-" },
  { 0xae, "*" },
  { 0xaf, "/" },
  { 0xb0, "^" },
  { 0xb1, "AND" },
  { 0xb2, "OR" },
  { 0xb3, ">" },
  { 0xb4, "=" },
  { 0xb5, "<" },
  { 0xb6, "SGN" },
  { 0xb7, "INT" },
  { 0xb8, "ABS" },
  { 0xb9, "USR" },
  { 0xba, "FRE" },
  { 0xbb, "INP" },
  { 0xbc, "POS" },
  { 0xbd, "SQR" },
  { 0xbe, "RND" },
  { 0xbf, "LN" },
  { 0xc0, "EXP" },
  { 0xc1, "COS" },
  { 0xc2, "SIN" },
  { 0xc3, "TAN" },
  { 0xc4, "ATN" },
  { 0xc5, "PEEK" },
  { 0xc6, "DEEK" },
  { 0xc7, "PI" },
  { 0xc8, "LEN" },
  { 0xc9, "STR$" },
  { 0xca, "VAL" },
  { 0xcb, "ASC" },
  { 0xcc, "CHR$" },
  { 0xcd, "LEFT$" },
  { 0xce, "RIGHT$" },
  { 0xcf, "MID$" },
  { 0xd0, "LOAD" },
  { 0xd1, "TRON" },
  { 0xd2, "TROFF" },
  { 0xd3, "EDIT" },
  { 0xd4, "ELSE" },
  { 0xd5, "INKEY$" },
  { 0xd6, "JOYST" },
  { 0xd7, "STRING$" },
  { 0xd8, "INSTR" },
  { 0xd9, "RENUM" },
  { 0xda, "DELETE" },
  { 0xdb, "PAUSE" },
  { 0xdc, "BEEP" },
  { 0xdd, "WINDOW" },
  { 0xde, "BORDER" },
  { 0xdf, "INK" },
  { 0xe0, "PAPER" },
  { 0xe1, "AT" },
  { 0xe2, "PSET" },
  { 0xe3, "LINE" },
  { 0xe4, "CIRCLE" },
  { 0xe5, "!" },
  { 0xe6, "PAINT" },
  { 0xe7, "LABEL" },
  { 0xe8, "SIZE" },
  { 0xe9, "ZERO" },
  { 0xea, "HOME" },
  { 0xeb, "!" },
  { 0xec, "GCLS" },
  { 0xed, "SCALE" },
  { 0xee, "SCREEN" },
  { 0xef, "POINT" },
  { 0xf0, "XPOS" },
  { 0xf1, "!" },
  { 0xf2, "YPOS" },
  { 0x00, NULL }
};

typedef struct decode_state {
  int state;
  int size;
  int addr;
  int last;
  int len;
  int lineno;
  unsigned char *line;
  char *text;
  int cnt;
} decode_state_t;

static decode_state_t decode_state;

const char *
decode_token(int byte)
{
  int a;
  static char buf[5];

  for (a = 0;token[a].val != 0;a++)
    if (token[a].val == byte)
      return token[a].text;

  //fprintf(stderr, "can't decode token %02x\n", byte);
  snprintf(buf, 5, "[%02x]", byte);
  return buf;
}

void
print_line(decode_state_t *s)
{
  if (s->last != 0)
    if (s->last != (s->addr - s->len - 5))
      fprintf(stderr, "decode error, address calculation error\n");

  if (_verbose > 1)
    printf("[%04xh-%04xh] : %5d %s\n",
	   s->addr - s->len - 5,
	   s->addr - 1,
	   s->lineno,
	   s->text);
  else
    printf("%5d %s\n",
	   s->lineno,
	   s->text);

  s->last = s->addr;
}

void
decode_byte(int byte)
{
  char *ptr;
  decode_state_t *s = &decode_state;

  if (byte == EOF)
    {
      if (s->state == 8)
	{
	  if (_verbose > 0)
	    printf("%d bytes after basic end\n", s->cnt);
	}
      else
	{
	  fprintf(stderr, "decode error, reached EOF\n");
	}

      return;
    }

  switch (s->state)
    {
    case 0:
      s->cnt = 0;
      s->last = 0;
      s->size = byte;
      s->state++;
      break;
    case 1:
      s->size += 256 * byte;
      s->state++;
      break;
    case 2:
      s->addr = byte;
      s->state++;
      break;
    case 3:
      s->addr += 256 * byte;
      s->state++;
      if (s->addr == 0)
	s->state = 8;
      break;
    case 4:
      s->lineno = byte;
      s->state++;
      break;
    case 5:
      s->lineno += 256 * byte;
      s->state++;
      s->len = 0;
      s->line = (unsigned char *)malloc(160);
      s->text = (char *)malloc(1000);
      s->text[0] = '\0';
      break;
    case 6:
      if (byte == 0)
	{
	  s->state = 2;
	  print_line(s);
	}
      else
	{
	  s->line[s->len++] = byte;
	  if (byte & 0x80)
	    strcat(s->text, decode_token(byte));
	  else
	    {
	      ptr = strrchr(s->text, '\0');
	      ptr[0] = isprint(byte) ? byte : (_binary ? byte : '.');
	      ptr[1] = '\0';
	      ptr++;
	    }

	  if (byte == '"')
	    s->state = 7;
	}
      break;
    case 7:
      if (byte == 0)
	{
	  s->state = 2;
	  print_line(s);
	}
      else
	{
	  s->line[s->len++] = byte;
	  ptr = strrchr(s->text, '\0');
	  ptr[0] = isprint(byte) ? byte : (_binary ? byte : '.');
	  ptr[1] = '\0';
	  ptr++;

	  if (byte == '"')
	    s->state = 6;
	}
      break;
    case 8:
      s->cnt++;
      break;
    }
}

void
decode(fileio_prop_t *prop)
{
  int a;
  int size;
  unsigned char *d = prop->data;

  if ((prop->type != FILEIO_TYPE_BAS) && (prop->type != FILEIO_TYPE_PROT_BAS))
    {
      printf("file '%s' is not a basic file.\n", prop->name);
      return;
    }

  size = prop->size;

  if (_verbose > 0)
    {
      printf("======================\n");
      printf("filename  : '%s'\n", prop->name);
      printf("file size : %d\n", size);
      printf("basic size: %d\n", 256 * d[13] + d[12]);
      printf("----------------------\n\n");
    }

  a = 12;
  decode_state.state = 0;
  while (a <= size)
    {
      decode_byte(d[a++]);
      if ((a % 129) == 0)
	a++;
    }
  decode_byte(EOF);
  
  if (_verbose > 0)
    printf("======================\n\n");
}

int
main(int argc, char **argv)
{
  int a;
  fileio_prop_t *prop;

  fileio_init();
  
  for (a = 1;a < argc;a++)
    {
      if (strcmp("-1", argv[a]) == 0)
	{
	  fileio_set_kctype(FILEIO_KC85_1);
	  continue;
	}
      if (strcmp("-3", argv[a]) == 0)
	{
	  fileio_set_kctype(FILEIO_KC85_3);
	  continue;
	}
      if (strcmp("-a", argv[a]) == 0)
	{
	  _binary = 0;
	  continue;
	}
      if (strcmp("-b", argv[a]) == 0)
	{
	  _binary = 1;
	  continue;
	}
      if (strcmp("-v", argv[a]) == 0)
	{
	  _verbose++;
	  continue;
	}

      if (fileio_load_file(argv[a], &prop) == 0)
	decode(prop);
    }
  
  return 0;
}
