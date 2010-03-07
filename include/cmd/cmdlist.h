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

#ifndef __cmd_cmdlist_h
#define __cmd_cmdlist_h

#define __CMD_COMMAND_SHIFT	(16)
#define __CMD_VALUE_MASK	(0xffff)
#define __CMD_TYPE		unsigned int

#include "cmd/cmdargs.h"

#define KCCMD(name,val)	  (new CMD_Val((__CMD_TYPE)(((KCCMD_##name##_##val) & __CMD_VALUE_MASK) | \
                            (KCCMD_##name << __CMD_COMMAND_SHIFT)), this))

#define KCCMD_0(name,val) (new CMD_Val((__CMD_TYPE)(((KCCMD_##name##_##val) & __CMD_VALUE_MASK) | \
                            (KCCMD_##name << __CMD_COMMAND_SHIFT)), 0))

class CMD_Val
{
 public:
  __CMD_TYPE _type;
  CMD_Caller *_caller;

  CMD_Val(__CMD_TYPE type, CMD_Caller *caller)
    {
      _type = type;
      _caller = caller;
    }
};

#define KCCMD_RESET	     (0x0001)
#define KCCMD_RESET_0          (0x0000)
#define KCCMD_RESET_RESET      (0x0001)
#define KCCMD_RESET_POWER_ON   (0x0002)
#define KCCMD_QUIT	     (0x0002)
#define KCCMD_QUIT_0	       (0x0000)

#define KCCMD_LOAD           (0x0003)
#define KCCMD_LOAD_0           (0x0000)
#define KCCMD_LOAD_IMAGE       (0x0001)
#define KCCMD_LOAD_IMAGE_OK    (0x0002)

/*
 *  User Interface Callbacks
 */

/*****
 *  this one is used to fetch a filename from the user interface
 *  by setting the 'file-name' argument
 */
#define KCCMD_UI_BROWSE      (0x0101)
#define KCCMD_UI_BROWSE_0      (0x0000)

#endif /* __cmd_list_h */
