/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: disass.cc,v 1.5 2002/06/09 14:24:33 torsten_paul Exp $
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/disass.h"
#include "kc/memory.h"

#include "kc/op.h"
#include "kc/opcb.h"
#define GEN_DD
#include "kc/opxx.h"
#define GEN_FD
#include "kc/opxx.h"
#define GEN_DD
#include "kc/opxxxx.h"
#define GEN_FD
#include "kc/opxxxx.h"

#define STRING_PC "%04xh:"
#define STRING_OP "%-8s"
#define S_END(x) strchr(x, '\0')

extern "C" int disass(int, char **); /* FIXME */

static char *_args[] = {
	"",
	"A", "B", "C", "D", "E", "H", "L",
	"BC", "DE", "HL", "SP", "AF",
	"(BC)", "(DE)", "(HL)", "(SP)",
        "(IX%c%02xh)", "(IY%c%02xh)", "(%04xh)",
	"%02xh", "%04xh", "%04xh",
	"0", "1", "2", "3", "4", "5", "6", "7",
        "00h", "08h", "10h", "18h", "20h", "28h", "30h", "38h"
};

static unsigned char
rd_z80(int pc)
{
  return memory->memRead8(pc);
}

static int
get_arg_string(int pc, arg_t arg, char **ptr)
{
  int ret;
  int d, e;
  char buf[10];
  
  ret = 0;
  switch (arg)
    {
    case _i_n:
      ret++;
      sprintf(buf, _args[arg], rd_z80(pc));
      break;
    case _i_nn:
    case _p_nn:
      ret += 2;
      sprintf(buf, _args[arg], rd_z80(pc) + (rd_z80(pc + 1) << 8));
      break;
    case _i_e:
      ret += 1;
      e = (signed char)rd_z80(pc);
      sprintf(buf, _args[arg], pc + e + 1);
      break;
    case _p_IXd:
    case _p_IYd:
      d = (signed char)rd_z80(pc);
      sprintf(buf, _args[arg], (d < 0) ? '-' : '+', (d < 0) ? -d : d);
      break;
    default:
      strcpy(buf, _args[arg]);
      break;
    }

  *ptr = strdup(buf);
  return ret;
}

static int
fd_cb_opcode_func(char *buf, int pc, int c)
{
  char *dptr, *sptr;

  get_arg_string(pc + 2, opcodes_fd_cb[c].dest, &dptr);
  get_arg_string(pc + 2, opcodes_fd_cb[c].src, &sptr);

  sprintf(buf, STRING_PC " fd cb %02x %02x   ", pc, rd_z80(pc + 2), c);
  sprintf(S_END(buf), STRING_OP, opcodes_fd_cb[c].op);

  if (*dptr) sprintf(S_END(buf), "    %s", dptr);
  if (*sptr) sprintf(S_END(buf), ", %s", sptr);

  free(dptr);
  free(sptr);

  return 4;
}

static int
fd_opcode_func(char *buf, int pc, int c)
{
  char *dptr, *sptr;

  get_arg_string(pc + 2, opcodes_fd[c].dest, &dptr);
  get_arg_string(pc + 2, opcodes_fd[c].src, &sptr);

  sprintf(buf, STRING_PC " fd %02x %02x      ", pc, c, rd_z80(pc + 2));
  sprintf(S_END(buf), STRING_OP, opcodes_fd[c].op);
  if (*dptr) sprintf(S_END(buf), "    %s", dptr);
  if (*sptr) sprintf(S_END(buf), ", %s", sptr);

  free(dptr);
  free(sptr);

  return 3;
}

static int
dd_cb_opcode_func(char *buf, int pc, int c)
{
  char *dptr, *sptr;

  get_arg_string(pc + 2, opcodes_dd_cb[c].dest, &dptr);
  get_arg_string(pc + 2, opcodes_dd_cb[c].src, &sptr);

  sprintf(buf, STRING_PC " dd cb %02x %02x   ", pc, rd_z80(pc + 2), c);
  sprintf(S_END(buf), STRING_OP, opcodes_dd_cb[c].op);

  if (*dptr) sprintf(S_END(buf), "    %s", dptr);
  if (*sptr) sprintf(S_END(buf), ", %s", sptr);

  free(dptr);
  free(sptr);

  return 4;
}

static int
dd_opcode_func(char *buf, int pc, int c)
{
  char *dptr, *sptr;

  get_arg_string(pc + 2, opcodes_dd[c].dest, &dptr);
  get_arg_string(pc + 2, opcodes_dd[c].src, &sptr);

  sprintf(buf, STRING_PC " dd %02x %02x      ", pc, c, rd_z80(pc + 2));
  sprintf(S_END(buf), STRING_OP, opcodes_dd[c].op);
  if (*dptr) sprintf(S_END(buf), "    %s", dptr);
  if (*sptr) sprintf(S_END(buf), ", %s", sptr);

  free(dptr);
  free(sptr);

  return 3;
}

static int
cb_opcode_func(char *buf, int pc, int c)
{
  char *dptr, *sptr;

  get_arg_string(pc + 1, opcodes_cb[c].dest, &dptr);
  get_arg_string(pc + 1, opcodes_cb[c].src, &sptr);

  sprintf(buf, STRING_PC " cb %02x         ", pc, c);
  sprintf(S_END(buf), STRING_OP, opcodes_cb[c].op);
  if (*sptr)
    sprintf(S_END(buf), "    %s, %s", sptr, dptr);
  else
    sprintf(S_END(buf), "    %s", dptr);

  free(dptr);
  free(sptr);

  return 2;
}

static int
std_opcode_func(char *buf, int pc, int c)
{
  int a, ret;
  char *dptr, *sptr;

  ret = 0;
  ret += get_arg_string(pc + 1, opcodes[c].dest, &dptr);
  ret += get_arg_string(pc + 1, opcodes[c].src, &sptr);

  sprintf(buf, STRING_PC " %02x", pc, c);
  for (a = 1;a < 5;a++)
    {
      if (a > ret)
	sprintf(S_END(buf), "   ");
      else
	sprintf(S_END(buf), " %02x", rd_z80(pc + a));
    }

  sprintf(S_END(buf), STRING_OP, opcodes[c].op);
  if (*dptr) sprintf(S_END(buf), "    %s", dptr);
  if (*sptr) sprintf(S_END(buf), ", %s", sptr);

  free(dptr);
  free(sptr);

  return ret + 1;
}

int
disass(int pc, char **ptr)
{
  int c;
  char buf[80];
  opcode_func_t func;
  
  func = std_opcode_func;

  c = rd_z80(pc);
      
  switch (c)
    {
    case 0xcb:
      func = cb_opcode_func;
      c = rd_z80(pc + 1);
      break;
    case 0xdd:
      func = dd_opcode_func;
      c = rd_z80(pc + 1);
      if (c == 0xcb)
        {
          func = dd_cb_opcode_func;
          c = rd_z80(pc + 3); /* pc + 2 is the displacement for (ix + d) */
        }
      break;
    case 0xed:
      break;
    case 0xfd:
      func = fd_opcode_func;
      c = rd_z80(pc + 1);
      if (c == 0xcb)
        {
          func = fd_cb_opcode_func;
          c = rd_z80(pc + 3); /* pc + 2 is the displacement for (iy + d) */
        }
      break;
    }
  
  pc += (*func)(buf, pc, c);

  *ptr = strdup(buf);
  return pc;
}
