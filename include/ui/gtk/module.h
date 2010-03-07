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

#ifndef __ui_gtk_module_h
#define __ui_gtk_module_h

#include <gtk/gtk.h>

#include "kc/system.h"

#include "ui/module.h"
#include "ui/gtk/window.h"

class ModuleWindow : public UI_Gtk_Window, public UI_ModuleInterface
{
private:
  int _nr_of_bd;
  enum {
    MAX_BD = 63,
    NR_OF_SLOTS = 4 * MAX_BD + 2,
  };

  struct {
    GtkWidget *vbox;
    GtkWidget *frame[MAX_BD + 1];
    GtkWidget *table[MAX_BD + 1];
    GtkWidget *l[NR_OF_SLOTS];
    GtkWidget *m[NR_OF_SLOTS];
    GtkWidget *led[NR_OF_SLOTS];
    GtkWidget *color_exp;
    GSList    *g[NR_OF_SLOTS];
  } _w;

  CMD *_cmd;

protected:
  void init(void);
  void init2(void);
  void init_device(const char *name, int base, int active_slots);
  void init_device_1(const char *name, int nr_of_slots);

  GtkWidget *create_menu(int slot);
  static void sf_activate(GtkWidget *widget, gpointer data);
  static void sf_color_expansion(GtkWidget *widget, gpointer data);

 public:
  ModuleWindow(const char *ui_xml_file);
  virtual ~ModuleWindow(void);

  /*
   *  UI_ModuleInterface
   */
  virtual void insert(int slot, ModuleInterface *m);
  virtual void activate(int slot, byte_t value);
};

#endif /* __ui_gtk_module_h */
