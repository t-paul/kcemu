/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: cmd.cc,v 1.3 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "ui/gtk/cmd.h"

void
cmd_exec_sf(GtkWidget *widget, gpointer data)
{
  const char *name = (const char *)data;
  CMD_EXEC(name);
}

int
cmd_exec_sft(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  const char *name = (const char *)data;
  CMD_EXEC(name);
  return true;
}

void
cmd_exec_sftb(GtkWidget *widget, gpointer data)
{
  const char *name = (const char *)data;
  if (GTK_TOGGLE_BUTTON(widget)->active)
    CMD_EXEC(name);
}

void
cmd_exec_mc(gpointer data, guint action, GtkWidget *widget)
{
  const char *name = (const char *)action;
  CMD_EXEC(name);
}
