/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: module.cc,v 1.12 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/rc.h"
#include "kc/mod_list.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/module.h"
#include "ui/gtk/gtkledline.h"

class CMD_ui_module_window_toggle : public CMD
{
private:
  ModuleWindow *_w;
  
public:
  CMD_ui_module_window_toggle(ModuleWindow *w) : CMD("ui-module-window-toggle")
    {
      _w = w;
      register_cmd("ui-module-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};


ModuleWindow::ModuleWindow(void)
{
  init();
}

ModuleWindow::~ModuleWindow(void)
{
}

void
ModuleWindow::sf_color_expansion(GtkWidget *widget, gpointer data)
{
  if (GTK_TOGGLE_BUTTON(widget)->active)
    module_list->insert(61, module_list->get_color_expansion());
  else
    module_list->insert(61, NULL);
}

void
ModuleWindow::sf_activate(GtkWidget *widget, gpointer data)
{
  int slot = (int)data;

  if (GTK_CHECK_MENU_ITEM(widget)->active)
    {
      ModuleListEntry *e;

      e = (ModuleListEntry *)gtk_object_get_user_data(GTK_OBJECT(widget));
      module_list->insert(slot, e);
    }
}

GtkWidget *
ModuleWindow::create_menu(int slot)
{
  GtkWidget *menu;
  GtkWidget *menuitem;
  GSList *group;
  ModuleList::iterator it;
  kc_type_t type;

  type = get_kc_type();
  menu = gtk_menu_new();

  group = NULL;
  for (it = module_list->begin();it != module_list->end();it++)
    {
      if (((*it)->get_kc_type() & type) == 0)
	continue;

      menuitem = gtk_radio_menu_item_new_with_label(group, (*it)->get_name());
      gtk_object_set_user_data(GTK_OBJECT(menuitem), (*it));
      gtk_signal_connect(GTK_OBJECT(menuitem), "activate",
                         GTK_SIGNAL_FUNC(sf_activate), (gpointer)slot);
      group = gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(menuitem));
      gtk_menu_append(GTK_MENU(menu), menuitem);
      gtk_widget_show(menuitem);
    }
  _w.g[slot] = group;
  return menu;
}

void
ModuleWindow::init_device(const char *name, int base, int active_slots)
{
  char buf[10];
  int a, b, c, idx, slot;

  a = base / 16;

  /*
   *  frame
   */
  _w.frame[a] = gtk_frame_new(name);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.frame[a], FALSE, FALSE, 0);
  gtk_widget_show(_w.frame[a]);
  
  /*
   *  table
   */
  _w.table[a] = gtk_table_new(2, 6, 0);
  gtk_container_border_width(GTK_CONTAINER(_w.table[a]), 4);
  gtk_table_set_row_spacings(GTK_TABLE(_w.table[a]), 2);
  gtk_table_set_col_spacings(GTK_TABLE(_w.table[a]), 4);
  gtk_table_set_col_spacing(GTK_TABLE(_w.table[a]), 2, 16);
  gtk_container_add(GTK_CONTAINER(_w.frame[a]), _w.table[a]);
  gtk_widget_show(_w.table[a]);
      
  for (b = 0;b < 4;b++)
    {
      /*
       *  skip slots that are not active for the
       *  current device (e.g. slot 0 and 4 for the
       *  basis device)
       */
      if ((active_slots & (1 << b)) == 0)
	continue;
      
      c = b ^ 3;
      slot = 16 * a + 4 * b;
      idx = slot / 4 - 2;
      sprintf(buf, "%X", 4 * b);
      
      _w.l[idx] = gtk_label_new(buf);
      gtk_misc_set_alignment(GTK_MISC(_w.l[idx]), 0, 0.5);
      gtk_table_attach(GTK_TABLE(_w.table[a]), _w.l[idx],
		       3 * (c & 1), 3 * (c & 1) + 1,
		       (c & 2) / 2, (c & 2) / 2 + 1,
		       GTK_FILL, GTK_FILL,
		       0, 0);
      gtk_widget_show(_w.l[idx]);
      
      _w.m[idx] = gtk_option_menu_new();
      gtk_option_menu_set_menu(GTK_OPTION_MENU(_w.m[idx]),
			       create_menu(idx));
      gtk_table_attach(GTK_TABLE(_w.table[a]), _w.m[idx],
		       3 * (c & 1) + 1, 3 * (c & 1) + 2,
		       (c & 2) / 2, (c & 2) / 2 + 1,
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL,
		       0, 0);
      gtk_widget_show(_w.m[idx]);
      
      _w.led[idx] = gtk_led_line_new(1);
      gtk_table_attach(GTK_TABLE(_w.table[a]), _w.led[idx],
		       3 * (c & 1) + 2, 3 * (c & 1) + 3,
		       (c & 2) / 2, (c & 2) / 2 + 1,
		       (GtkAttachOptions)0, GTK_FILL,
		       0, 0);
      gtk_widget_show(_w.led[idx]);
    }
}

void
ModuleWindow::init_device_1(const char *name)
{
  int a;
  char buf[10];

  /*
   *  frame
   */
  _w.frame[0] = gtk_frame_new(name);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.frame[0], FALSE, FALSE, 0);
  gtk_widget_show(_w.frame[0]);
  
  /*
   *  table
   */
  _w.table[0] = gtk_table_new(5, 3, 0);
  gtk_container_border_width(GTK_CONTAINER(_w.table[0]), 4);
  gtk_table_set_row_spacings(GTK_TABLE(_w.table[0]), 2);
  gtk_table_set_col_spacings(GTK_TABLE(_w.table[0]), 4);
  gtk_table_set_col_spacing(GTK_TABLE(_w.table[0]), 2, 16);
  gtk_container_add(GTK_CONTAINER(_w.frame[0]), _w.table[0]);
  gtk_widget_show(_w.table[0]);

  for (a = 0;a < 4;a++)
    {
      snprintf(buf, 10, _("Slot %d"), 4 - a);

      _w.l[a] = gtk_label_new(buf);
      gtk_misc_set_alignment(GTK_MISC(_w.l[a]), 0, 0.5);
      gtk_table_attach(GTK_TABLE(_w.table[0]), _w.l[a],
		       0, 1,
		       a, a + 1,
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL,
		       0, 0);
      gtk_widget_show(_w.l[a]);

      _w.m[a] = gtk_option_menu_new();
      gtk_option_menu_set_menu(GTK_OPTION_MENU(_w.m[a]), create_menu(a));
      gtk_table_attach(GTK_TABLE(_w.table[0]), _w.m[a],
		       1, 2,
		       a, a + 1,
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL,
		       0, 0);
      gtk_widget_show(_w.m[a]);

      _w.led[a] = gtk_led_line_new(1);
#if 0
      gtk_table_attach(GTK_TABLE(_w.table[0]), _w.led[a],
		       2, 3,
		       a, a + 1,
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL,
		       0, 0);
#endif
      gtk_widget_show(_w.led[a]);
    }

  if (get_kc_type() & KC_TYPE_85_1)
    {
      _w.color_exp = gtk_check_button_new_with_label(_("IRM Color Expansion"));
      gtk_table_attach(GTK_TABLE(_w.table[0]), _w.color_exp,
		       1, 2,
		       4, 5,
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_FILL,
		       0, 0);
      gtk_signal_connect(GTK_OBJECT(_w.color_exp), "toggled",
                         GTK_SIGNAL_FUNC(sf_color_expansion), NULL);
      gtk_widget_show(_w.color_exp);
    }
}

void
ModuleWindow::init(void)
{
  int a;
  char buf[100];

  memset(_w.l,   0, sizeof(GtkWidget *) * NR_OF_SLOTS);
  memset(_w.m,   0, sizeof(GtkWidget *) * NR_OF_SLOTS);
  memset(_w.led, 0, sizeof(GtkWidget *) * NR_OF_SLOTS);
  memset(_w.g,   0, sizeof(GSList *)    * NR_OF_SLOTS);

  _nr_of_bd = module_list->get_nr_of_busdrivers();

  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_widget_set_name(_window, "ModuleWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Module"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-module-window-toggle"); // FIXME:

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  if (RC::instance()->get_int("Floppy Disk Basis") && (get_kc_type() & KC_TYPE_85_2_CLASS))
    init_device(_("Floppy Disk Basis [F0]"), 0xf0, 3);

  for (a = _nr_of_bd;a > 0;a--)
    {
      sprintf(buf, _("D002: Busdriver [%02X]"), 16 * a);
      init_device(buf, 16 * a, 15);
    }

  if (get_kc_type() & KC_TYPE_85_1_CLASS)
    init_device_1(_("Basis Device"));
  else
    init_device(_("Basis Device"), 0, 12);

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
  
  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close,
		     FALSE, FALSE, 5);
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-module-window-toggle");
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);
  
  CMD *cmd;
  cmd = new CMD_ui_module_window_toggle(this);
}

/*
 *  UI_ModuleInterface
 */
void
ModuleWindow::insert(int slot, ModuleInterface *m)
{
  int a;
  GSList *e;
  GtkObject *o;
  ModuleInterface *m2;
  
  e = _w.g[slot];
  /*
   *  FIXME: This depends on the fact that the GSList used for
   *         radio_option_menu_items has the items stored in
   *         reverse order!
   */
  a = g_slist_length(e) - 1;
  while (e != 0)
    {
      o = GTK_OBJECT(e->data);
      m2 = ((ModuleListEntry *)gtk_object_get_user_data(o))->get_mod();
      if (m2 == m)
        {
          gtk_option_menu_set_history(GTK_OPTION_MENU(_w.m[slot]), a);
          return;
        }
      a--;
      e = e->next;
    }
}

void
ModuleWindow::activate(int slot, byte_t value)
{
  if (_w.m[slot])
    {
      gtk_widget_set_sensitive(_w.m[slot], !(value & 1));
      gtk_led_line_set_led(GTK_LED_LINE(_w.led[slot]), 0, value & 1);
    }
}
