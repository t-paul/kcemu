/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: tape.cc,v 1.22 2002/10/31 01:38:12 torsten_paul Exp $
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

#include <ctype.h>

#include "kc/system.h"

#include "kc/rc.h"
#include "kc/tape.h"
#include "ui/error.h"

#include "ui/status.h"
#include "ui/gtk/tape.h"

#include "ui/gtk/cmd.h"

class CMD_ui_tape_window_toggle : public CMD
{
private:
  TapeWindow *_w;
  
public:
  CMD_ui_tape_window_toggle(TapeWindow *w) : CMD("ui-tape-window-toggle")
    {
      _w = w;
      register_cmd("ui-tape-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

class CMD_ui_tape_rename_selected : public CMD
{
private:
  TapeWindow *_w;
public:
  CMD_ui_tape_rename_selected(TapeWindow *w) : CMD("ui-tape-rename-selected")
    {
      _w = w;
      register_cmd("ui-tape-rename-selected");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *name;

      switch (context)
        {
	case 0:
	  if (args == NULL)
	    args = new CMD_Args();

	  name = _w->tapeGetName(_w->tapeGetSelected());
	  args->set_string_arg("filename", name);
	  args->set_string_arg("tape-filename", name);
	  args->set_string_arg("tape-rename-title",
			       _("Please enter the new name for the file."));
	  args->add_callback("ui-tape-name-edit-CB-ok", this, 1);
	  CMD_EXEC_ARGS("ui-tape-name-edit", args);
	  break;
	case 1:
	  if (args == NULL)
	    return;

	  /*
	   *  rename callback
	   */
	  CMD_EXEC_ARGS("tape-rename-file", args);
	  break;
	}
    }
};

class CMD_ui_tape_load_selected : public CMD
{
private:
  TapeWindow *_w;
public:
  CMD_ui_tape_load_selected(TapeWindow *w) : CMD("ui-tape-load-selected")
    {
      _w = w;
      register_cmd("ui-tape-load-selected", 0);
      register_cmd("ui-tape-run-selected", 1);
      register_cmd("ui-tape-export-selected", 2);
      register_cmd("ui-tape-export-wav-selected", 3);
      register_cmd("ui-tape-delete-selected", 4);
      register_cmd("ui-tape-play-selected", 5);
      register_cmd("ui-edit-header-selected", 6);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      if (args == NULL)
	args = new CMD_Args();

      args->set_string_arg("tape-filename",
			   _w->tapeGetName(_w->tapeGetSelected()));

      switch (context)
        {
        case 0:
          CMD_EXEC_ARGS("tape-load", args);
          break;
        case 1:
          CMD_EXEC_ARGS("tape-run", args);
          break;
        case 2:
          CMD_EXEC_ARGS("tape-export", args);
          break;
        case 3:
          CMD_EXEC_ARGS("tape-export-wav", args);
          break;
        case 4:
          CMD_EXEC_ARGS("tape-delete", args);
          break;
        case 5:
          CMD_EXEC_ARGS("tape-play", args);
          break;
	case 6:
	  CMD_EXEC_ARGS("ui-edit-header", args);
	  break;
        }
    }
};

class CMD_ui_tape_attach : public CMD
{
private:
  TapeWindow *_w;
public:
  CMD_ui_tape_attach(TapeWindow *w) : CMD("ui-tape-attach")
    {
      _w = w;
      register_cmd("ui-tape-attach", 0);
      register_cmd("ui-tape-detach", 1);
      register_cmd("ui-tape-stop", 2);
      register_cmd("ui-tape-attached", 3);
      register_cmd("ui-tape-detached", 4);
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      const char *filename = NULL;

      switch (context)
        {
        case 0:
          CMD_EXEC("tape-attach");
          break;
        case 1:
          CMD_EXEC("tape-detach");
          break;
        case 2:
          _w->stop();
          CMD_EXEC("tape-stop");
	  break;
	case 3:
	  if (args)
	    filename = args->get_string_arg("filename");
	  /* fall through */
	case 4:
	  _w->attached(filename); // filename == NULL if detached called!
	  break;
	}
    }
};

void
TapeWindow::sf_tape_file_select(GtkWidget *widget, gint row, gint column, 
                                GdkEventButton * bevent, gpointer data)
{
  TapeWindow *self = (TapeWindow *)data;
  
  self->_selected = row;
  if (bevent && (bevent->type == GDK_2BUTTON_PRESS))
    CMD_EXEC("ui-tape-run-selected");
}

void
TapeWindow::sf_tape_archive_select(GtkWidget *widget, gpointer data)
{
  CMD_Args *args;
  char *filename;

  filename = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(data)->entry));
  args = (new CMD_Args())->set_string_arg("filename", (const char *)filename);
  CMD_EXEC_ARGS("tape-attach", args);
}

void
TapeWindow::sf_power_expose(TapeWindow *self)
{

  int width, height;
  GdkColor *col;
  static GdkGC *gc = NULL;

  if (self->_w.led_power->window == NULL)
    return;

  if (gc == NULL)
    gc = gdk_gc_new(self->_w.led_power->window);

  width = self->_w.led_power->allocation.width ;
  height = self->_w.led_power->allocation.height;

  if (self->_power)
    if (self->_play)
      col = &self->_col[2];
    else if (self->_record)
      col = &self->_col[1];
    else
      col = &self->_col[3];
  else
    col = &self->_col[0];
  
  gdk_gc_set_foreground(gc, col);
  gdk_draw_rectangle(self->_w.led_power->window, gc,
                     TRUE, width / 2 - 6, height / 2 - 2, 12, 4);
}

/*
 *  define some ungly typecasts to suppress some
 *  compiler warnings :-(
 */
#define CF(func)   ((GtkItemFactoryCallback)(func))
#define CD(string) ((unsigned int)(string))

void
TapeWindow::init(void)
{
  int a;
  GList *popdown;
  const char *fname;
  GtkTooltips *tips;

  tips = gtk_tooltips_new();
  gtk_tooltips_enable(tips);

  GtkItemFactory *ifactP;
  GtkAccelGroup *agroupP;
  GtkItemFactoryEntry entriesP[] = {
    { _("/_Run File"),      "R",  CF(cmd_exec_mc), CD("ui-tape-run-selected"),        NULL },
    { _("/_Load File"),     "L",  CF(cmd_exec_mc), CD("ui-tape-load-selected"),       NULL },
    { _("/_Export File"),   "E",  CF(cmd_exec_mc), CD("ui-tape-export-selected"),     NULL },
    { _("/Export _Wav"),    "W",  CF(cmd_exec_mc), CD("ui-tape-export-wav-selected"), NULL },
    { _("/sep1"),           NULL, NULL,         0,                                    "<Separator>" },
    { _("/Edit _Header"),   "H",  CF(cmd_exec_mc), CD("ui-edit-header-selected"),     NULL },
    { _("/Re_name File"),   "N",  CF(cmd_exec_mc), CD("ui-tape-rename-selected"),     NULL },
    { _("/_Delete File"),   "D",  CF(cmd_exec_mc), CD("ui-tape-delete-selected"),     NULL },
    { _("/sep2"),           NULL, NULL,         0,                                    "<Separator>" },
    { _("/_Add File"),      "A",  CF(cmd_exec_mc), CD("tape-add-file"),               NULL },
  };
  int nentriesP = sizeof(entriesP) / sizeof(entriesP[0]);
  static char *titles[] =
  {
    _("Name"),
    _("Type"),
    _("Load"),
    _("Start"),
    _("Size")
  };
  
  _play = 0;
  _record = 0;
  _power = 0;
  _selected = 0;

  /*
   *  popup item factory
   */
  agroupP = gtk_accel_group_new();
  ifactP = gtk_item_factory_new(GTK_TYPE_MENU, _("<TapeP>"), agroupP);
  gtk_item_factory_create_items(ifactP, nentriesP, entriesP, NULL);

  /*
   *  popup menu
   */
  _w.menu = gtk_item_factory_get_widget(ifactP, _("<TapeP>"));
  _w.m_run    = gtk_item_factory_get_widget(ifactP, _("/Run File"));
  _w.m_load   = gtk_item_factory_get_widget(ifactP, _("/Load File"));
  _w.m_edit   = gtk_item_factory_get_widget(ifactP, _("/Edit Header"));
  _w.m_delete = gtk_item_factory_get_widget(ifactP, _("/Delete File"));
  _w.m_rename = gtk_item_factory_get_widget(ifactP, _("/Rename File"));
  _w.m_export = gtk_item_factory_get_widget(ifactP, _("/Export File"));
  
  /*
   *  tape window
   */
  _window = gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_widget_set_name(_window, "TapeWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Tape"));
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-tape-window-toggle"); // FIXME:
  
  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  combo hbox, combo label, combo box
   */
  _w.combo_hbox = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_w.vbox), _w.combo_hbox);
  gtk_widget_show(_w.combo_hbox);

  _w.combo_label = gtk_label_new(_("Tape:"));
  gtk_box_pack_start(GTK_BOX(_w.combo_hbox), _w.combo_label, FALSE, FALSE, 6);
  gtk_widget_show(_w.combo_label);

  popdown = NULL;
  for (a = 0;;a++)
    {
      fname = RC::instance()->get_string_i(a, "Tape File List");
      if (fname == NULL)
	break;
      popdown = g_list_append(popdown, (void *)fname);
    }
  _w.combo = gtk_combo_new();
  gtk_box_pack_start(GTK_BOX(_w.combo_hbox), _w.combo, TRUE, TRUE, 0);
  gtk_combo_set_value_in_list(GTK_COMBO(_w.combo), FALSE, TRUE);
  gtk_combo_set_use_arrows(GTK_COMBO(_w.combo), TRUE);
  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO(_w.combo)->entry), FALSE);
  if (popdown)
    gtk_combo_set_popdown_strings(GTK_COMBO(_w.combo), popdown);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(_w.combo)->entry), "changed",
		     GTK_SIGNAL_FUNC(sf_tape_archive_select), _w.combo);
  fname = RC::instance()->get_string_i(a, "Tape File List");
  gtk_widget_show(_w.combo);

  /*
   *  tape list
   */
  _w.sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_usize(_w.sw, 300, 400);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(_w.sw),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.sw, TRUE, TRUE, 0);
  gtk_widget_show(_w.sw);

  _w.clist = gtk_clist_new_with_titles(5, titles);
  gtk_clist_set_selection_mode(GTK_CLIST(_w.clist), GTK_SELECTION_BROWSE);
  gtk_clist_column_titles_passive(GTK_CLIST(_w.clist));
  gtk_clist_set_reorderable(GTK_CLIST(_w.clist), TRUE);
  gtk_signal_connect(GTK_OBJECT(_w.clist), "button_press_event",
                     GTK_SIGNAL_FUNC(sf_tape_button_press), this);
  gtk_signal_connect(GTK_OBJECT(_w.clist), "select_row",
                     GTK_SIGNAL_FUNC(sf_tape_file_select), this);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(_w.sw), _w.clist);
  gtk_widget_show(_w.clist);
  
  /*
   *  hbox
   */
  _w.hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.hbox, FALSE, TRUE, 0);
  gtk_widget_show(_w.hbox);

  /*
   *  power drawing area
   */
  _w.led_power = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(_w.led_power), 20, 5);
  gtk_signal_connect_object(GTK_OBJECT(_w.led_power), "expose_event",
                            GTK_SIGNAL_FUNC(TapeWindow::sf_power_expose),
                            (GtkObject *)this);
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.led_power, FALSE, TRUE, 0);
  gtk_widget_show(_w.led_power);
  
  /*
   *  scale
   */
  _scale_value = 0.0;
  _w.scale_adj = gtk_adjustment_new (_scale_value,
                                   0.0, 101.0, 0.1, 1.0, 1.0);
  gtk_signal_connect(GTK_OBJECT(_w.scale_adj), "value_changed",
                     GTK_SIGNAL_FUNC(TapeWindow::sf_tape_scale_changed), this);
  _w.scale = gtk_hscale_new(GTK_ADJUSTMENT(_w.scale_adj));
  //gtk_range_set_update_policy(GTK_RANGE(_w.scale), GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value(GTK_SCALE(_w.scale), FALSE);
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.scale, TRUE, TRUE, 0);
  gtk_widget_show(_w.scale);
  
  /*
   *  button box
   */
  _w.bbox = gtk_hbutton_box_new();
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(_w.bbox), 0);
  gtk_button_box_set_layout(GTK_BUTTON_BOX(_w.bbox), GTK_BUTTONBOX_SPREAD);
  gtk_button_box_set_child_size(GTK_BUTTON_BOX(_w.bbox), 0, 0);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.bbox, FALSE, TRUE, 0);
  gtk_widget_show(_w.bbox);
  
  /*
   *  buttons
   */
  _w.b_stop = gtk_button_new_with_label(_("Stop"));
  gtk_signal_connect(GTK_OBJECT(_w.b_stop), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-tape-stop");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_stop);
  gtk_widget_show(_w.b_stop);
  
  _w.b_record = gtk_toggle_button_new_with_label(_("Record"));
  gtk_signal_connect(GTK_OBJECT(_w.b_record), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sftb),
                     (gpointer)"tape-record");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_record);
  gtk_widget_show(_w.b_record);
  
  _w.b_play = gtk_toggle_button_new_with_label(_("Play"));
  gtk_signal_connect(GTK_OBJECT(_w.b_play), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sftb),
                     (gpointer)"ui-tape-play-selected");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_play);
  gtk_widget_show(_w.b_play);
  
  _w.b_attach = gtk_button_new_with_label(_("Attach"));
  gtk_signal_connect(GTK_OBJECT(_w.b_attach), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-tape-attach");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_attach);
  gtk_widget_show(_w.b_attach);
  
  _w.b_detach = gtk_button_new_with_label(_("Detach"));
  gtk_signal_connect(GTK_OBJECT(_w.b_detach), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-tape-detach");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_detach);
  gtk_widget_show(_w.b_detach);
  
  _w.b_close = gtk_button_new_with_label(_("Close"));
  gtk_signal_connect(GTK_OBJECT(_w.b_close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-tape-window-toggle");
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_close);
  gtk_widget_show(_w.b_close);
  
  gtk_widget_set_sensitive(_w.b_stop, FALSE);
  gtk_widget_set_sensitive(_w.b_play, FALSE);
  gtk_widget_set_sensitive(_w.b_record, FALSE);
  gtk_widget_set_sensitive(_w.b_detach, FALSE);

  allocate_colors();

  CMD *cmd;
  cmd = new CMD_ui_tape_window_toggle(this);
  cmd = new CMD_ui_tape_load_selected(this);
  cmd = new CMD_ui_tape_rename_selected(this);
  cmd = new CMD_ui_tape_attach(this);
}

void
TapeWindow::stop(void)
{
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(_w.b_play), FALSE);
  gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(_w.b_record), FALSE);
}

void
TapeWindow::attached(const char *name)
{
  /*
   *  block changed signal when setting the entry field!
   */
  gtk_signal_handler_block_by_data(GTK_OBJECT(GTK_COMBO(_w.combo)->entry), _w.combo);
  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(_w.combo)->entry), name);
  gtk_signal_handler_unblock_by_data(GTK_OBJECT(GTK_COMBO(_w.combo)->entry), _w.combo);
}

void
TapeWindow::sf_tape_scale_changed(GtkAdjustment *adjustment, gpointer data)
{
  TapeWindow *self = (TapeWindow *)data;

  if (self->_scale_value != adjustment->value)
    tape->seek((int)adjustment->value);
}

int
TapeWindow::sf_tape_button_press(GtkWidget */*widget*/, GdkEventButton *event,
                                 gpointer data)
{
  gint ret, row, column;
  TapeWindow *self = (TapeWindow *)data;

  if (event->button == 3)
    {
      ret = gtk_clist_get_selection_info(GTK_CLIST(self->_w.clist),
                                         (int)event->x, (int)event->y,
                                         &row, &column);
      gtk_widget_set_sensitive(self->_w.m_run, ret);
      gtk_widget_set_sensitive(self->_w.m_load, ret);
      gtk_widget_set_sensitive(self->_w.m_edit, ret);
      gtk_widget_set_sensitive(self->_w.m_delete, ret);
      gtk_widget_set_sensitive(self->_w.m_rename, ret);
      gtk_widget_set_sensitive(self->_w.m_export, ret);
      if (ret)
        gtk_clist_select_row(GTK_CLIST(self->_w.clist), row, 0);
      gtk_menu_popup(GTK_MENU(self->_w.menu), NULL, NULL, NULL, NULL, 3,
                     event->time);
    }
  /*
   *  run other event handlers too...
   */
  return true;
}

void
TapeWindow::allocate_colors(void)
{
    int a;
    char *color_names[] = {
      "rgb:00/00/00",
      "rgb:d0/00/00",
      "rgb:00/d0/00",
      "rgb:a0/a0/00"
    };

    _colormap = gdk_colormap_get_system();
    for (a = 0;a < 4;a++) {
	gdk_color_parse(color_names[a], &_col[a]);
	gdk_color_alloc(_colormap, &_col[a]);
    }
}

void
TapeWindow::tapeAttach(const char *name)
{
  clear_list();
  gtk_widget_set_sensitive(_w.b_stop, TRUE);
  gtk_widget_set_sensitive(_w.b_play, TRUE);
  gtk_widget_set_sensitive(_w.b_record, TRUE);
  gtk_widget_set_sensitive(_w.b_detach, TRUE);
  _play = 0;
  _record = 0;
  tapeProgress(0);
}

void
TapeWindow::tapeDetach(void)
{
  clear_list();
  gtk_widget_set_sensitive(_w.b_stop, FALSE);
  gtk_widget_set_sensitive(_w.b_play, FALSE);
  gtk_widget_set_sensitive(_w.b_record, FALSE);
  gtk_widget_set_sensitive(_w.b_detach, FALSE);
  _play = 0;
  _record = 0;
  tapeProgress(0);
}

void
TapeWindow::tapePower(bool power)
{
  _power = power;
  sf_power_expose(this);
#if 0
  if (!power)
    {
      stop();
    }
#endif
}

int
TapeWindow::tapeGetSelected(void)
{
  return _selected;
}

const char *
TapeWindow::tapeGetName(int idx)
{
  int ret;
  char *name;
  
  ret = gtk_clist_get_text(GTK_CLIST(_w.clist), idx, 0, &name);
  if (ret == 0)
    return NULL;

  /* cout.form("TapeWindow::tapeGetSelected(): %d -> %s\n", idx, name); */
  return name;
}

void
TapeWindow::tapeProgress(int val)
{
  _scale_value = val;
  GTK_ADJUSTMENT(_w.scale_adj)->value = val;
  gtk_signal_emit_by_name(_w.scale_adj, "value_changed");
}

void
TapeWindow::tapeNext(void)
{
  int i, old;
  GList *list;

  i = 0;
  old = _selected;

  list = GTK_CLIST(_w.clist)->selection;
  if (list)
    i = GPOINTER_TO_INT(list->data);

  /* sets _selected via callback! (sf_tape_file_select) */
  gtk_clist_select_row(GTK_CLIST(_w.clist), i + 1, 0);

  list = GTK_CLIST(_w.clist)->selection;
  if (list)
    i = GPOINTER_TO_INT(list->data);

  if (old != i)
    {
      CMD_Args *a = new CMD_Args();
      a->set_int_arg("tape-play-delay", 10);
      CMD_EXEC_ARGS("ui-tape-play-selected", a);
    }
}

void
TapeWindow::clear_list(void)
{
  _nr_of_files = 0;
  gtk_clist_clear(GTK_CLIST(_w.clist));
  gtk_widget_set_sensitive(_w.b_stop, FALSE);
  gtk_widget_set_sensitive(_w.b_play, FALSE);
  gtk_widget_set_sensitive(_w.b_record, FALSE);
}

void
TapeWindow::tapeAddArchive(const char *name)
{
}

void
TapeWindow::tapeAddFile(const char    *name,
			long           load,
			long           start,
			long           size,
			unsigned char  type)
{
  char buf2[40];
  char buf3[40];
  char buf4[40];
  const char *data[5];
  kct_file_type_t t = (kct_file_type_t)type;

  sprintf(buf2, "%04lxh", load);
  if (start != 0xffff)
    sprintf(buf3, "%04lxh", start);
  else
    sprintf(buf3, "-");
  sprintf(buf4, "%ld", size);

  data[2] = "-";
  data[3] = "-";
  switch (t) {
  case KCT_TYPE_COM:
    data[1] = "COM";
    data[2] = buf2;
    data[3] = buf3;
    break;
  case KCT_TYPE_BAS:
    data[1] = "BAS";
    break;
  case KCT_TYPE_DATA:
    data[1] = "DATA";
    break;
  case KCT_TYPE_LIST:
    data[1] = "LIST";
    break;
  case KCT_TYPE_BAS_P:
    data[1] = "BAS*";
    break;
  default:
    data[1] = "???";
    break;
  }
  data[0] = name;
  data[4] = buf4;
  gtk_clist_append(GTK_CLIST(_w.clist), (char **)data);
  _nr_of_files++;
  if (_nr_of_files == 1)
    {
      gtk_widget_set_sensitive(_w.b_stop, TRUE);
      gtk_widget_set_sensitive(_w.b_play, TRUE);
      gtk_widget_set_sensitive(_w.b_record, TRUE);
    }
  gtk_clist_columns_autosize(GTK_CLIST(_w.clist));
}

void
TapeWindow::tapeRemoveFile(int idx)
{
  if (_nr_of_files == 1)
    clear_list();
  else
    {
      gtk_clist_remove(GTK_CLIST(_w.clist), idx);
      _nr_of_files--;
    }
}
