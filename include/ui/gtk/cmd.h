/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: cmd.h,v 1.4 2002/10/31 01:38:07 torsten_paul Exp $
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

#ifndef __ui_gtk_cmd_h
#define __ui_gtk_cmd_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "cmd/cmd.h"

/*
 *  cmd_exec as GTK+ signal function
 */
void cmd_exec_sf(GtkWidget *widget, gpointer data);

/*
 *  cmd_exec as GTK+ event function returning true
 *  (e.g. for "delete_event")
 */
int cmd_exec_sft(GtkWidget *widget, GdkEvent *event, gpointer data);

/*
 *  cmd_exec for toggle buttons
 *  this functions calls the command only if the button is pressed
 */
void cmd_exec_sftb(GtkWidget *widget, gpointer data);

/*
 *  cmd_exec as GTK+ menu callback
 */
void cmd_exec_mc(gpointer data, guint action, GtkWidget *widget);

#endif /* __ui_gtk_cmd_h */
