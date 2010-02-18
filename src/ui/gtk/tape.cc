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
#include <unistd.h>

#include "kc/system.h"

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

TapeWindow::TapeWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
   init(); 
}

TapeWindow::~TapeWindow(void)
{
}

void
TapeWindow::sf_tape_file_select(GtkTreeSelection *selection, gpointer data)
{
  TapeWindow *self = (TapeWindow *)data;
  self->set_selected_index(selection);
}

void
TapeWindow::sf_tape_archive_select(GtkWidget *widget, gpointer data)
{
  CMD_Args *args;
  const gchar *filename;

  /* for some reason this is called twice */
  filename = gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(data)->entry));
  if (strlen(filename) == 0)
    return;

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

  width = self->_w.led_power->allocation.width;
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
  GtkItemFactory *ifactP;
  GtkAccelGroup *agroupP;
  static const char *callbacks[] = {
    "ui-tape-run-selected",
    "ui-tape-load-selected",
    "ui-tape-export-selected",
    "ui-tape-export-wav-selected",
    "ui-edit-header-selected",
    "ui-tape-rename-selected",
    "ui-tape-delete-selected",
    "tape-add-file",
  };
  GtkItemFactoryEntry entriesP[] = {
    { _("/_Run File"),      "R",  CF(cmd_exec_mci), 0, NULL },
    { _("/_Load File"),     "L",  CF(cmd_exec_mci), 1, NULL },
    { _("/_Export File"),   "E",  CF(cmd_exec_mci), 2, NULL },
    { _("/Export _Wav"),    "W",  CF(cmd_exec_mci), 3, NULL },
    { _("/sep1"),           NULL, NULL,         0,    "<Separator>" },
    { _("/Edit _Header"),   "H",  CF(cmd_exec_mci), 4, NULL },
    { _("/Re_name File"),   "N",  CF(cmd_exec_mci), 5, NULL },
    { _("/_Delete File"),   "D",  CF(cmd_exec_mci), 6, NULL },
    { _("/sep2"),           NULL, NULL,         0,    "<Separator>" },
    { _("/_Add File"),      "A",  CF(cmd_exec_mci), 7, NULL },
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
  gtk_item_factory_create_items(ifactP, nentriesP, entriesP, callbacks);

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
  _w.m_wav    = gtk_item_factory_get_widget(ifactP, _("/Export Wav"));
  /*
   *  tape window
   */
  _window = get_widget("tape_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-tape-window-toggle"); // FIXME:
  
  _w.treeview = get_widget("main_treeview");
  gtk_signal_connect(GTK_OBJECT(_w.treeview), "button_press_event",
		     GTK_SIGNAL_FUNC(sf_tape_button_press),
		     this);

  _w.b_play = get_widget("control_button_play");
  gtk_signal_connect(GTK_OBJECT(_w.b_play), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sftb),
                     (gpointer)"ui-tape-play-selected");

  _w.b_stop = get_widget("control_button_stop");
  gtk_signal_connect(GTK_OBJECT(_w.b_stop), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-tape-stop");

  _w.b_record = get_widget("control_button_record");
  gtk_signal_connect(GTK_OBJECT(_w.b_record), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sftb),
                     (gpointer)"tape-record");

  _w.b_prev = get_widget("control_button_previous");
  _w.b_next = get_widget("control_button_next");

  _w.b_attach = get_widget("file_button_open");
  gtk_signal_connect(GTK_OBJECT(_w.b_attach), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-tape-attach");

  _w.b_detach = get_widget("file_button_close");;
  gtk_signal_connect(GTK_OBJECT(_w.b_detach), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (gpointer)"ui-tape-detach");

  _w.led_power = get_widget("progress_led");
  gtk_signal_connect_object(GTK_OBJECT(_w.led_power), "expose_event",
                            GTK_SIGNAL_FUNC(TapeWindow::sf_power_expose),
                            (GtkObject *)this);

  _w.list_store = gtk_list_store_new(TREEVIEW_N_COLUMNS,
				     G_TYPE_STRING,
				     G_TYPE_STRING,
				     G_TYPE_STRING,
				     G_TYPE_STRING,
				     G_TYPE_STRING);

  for (int a = 0;a < TREEVIEW_N_COLUMNS;a++)
    {
      GtkTreeViewColumn *column = gtk_tree_view_column_new();
      GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

      gtk_tree_view_column_set_title(column, titles[a]);

      if (a < 2)
	{
	  gtk_tree_view_column_pack_start(column, renderer, FALSE);
	}
      else
	{
	  gtk_tree_view_column_pack_end(column, renderer, FALSE);
	}
	
      gtk_tree_view_column_set_attributes(column, renderer, "text", a, NULL);
      gtk_tree_view_column_set_resizable(column, TRUE);
      gtk_tree_view_column_set_expand(column, TRUE);
      gtk_tree_view_append_column(GTK_TREE_VIEW(_w.treeview), column);
    }

  gtk_tree_view_set_model(GTK_TREE_VIEW(_w.treeview), GTK_TREE_MODEL(_w.list_store));
  gtk_tree_view_columns_autosize(GTK_TREE_VIEW(_w.treeview));

  GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
  gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
  g_signal_connect(G_OBJECT(select), "changed",
                  G_CALLBACK(sf_tape_file_select),
                  this);
  /*
   *  Scale
   */
  _w.scale = get_widget("progress_hscale");
  _w.scale_adj = gtk_range_get_adjustment(GTK_RANGE(_w.scale));

  allocate_colors();
  init_dialog("ui-tape-window-toggle", "window-tape");

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
  // gtk_signal_handler_block_by_data(GTK_OBJECT(GTK_COMBO(_w.combo)->entry), _w.combo);
  // gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(_w.combo)->entry), name);
  // gtk_signal_handler_unblock_by_data(GTK_OBJECT(GTK_COMBO(_w.combo)->entry), _w.combo);
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
  TapeWindow *self = (TapeWindow *)data;
  
  GtkTreePath *path;
  gboolean ret = gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(self->_w.treeview), 
					       (int)event->x, (int)event->y,
					       &path, NULL, NULL, NULL);

  if (ret)
    {
      gint *indices = gtk_tree_path_get_indices(path);
      self->_selected = indices[0];
    }

  if ((event->button == 1) && (event->type == GDK_2BUTTON_PRESS))
    {
      CMD_EXEC("ui-tape-run-selected");
    }
  else if (event->button == 3)
    {
      gtk_widget_set_sensitive(self->_w.m_run, ret);
      gtk_widget_set_sensitive(self->_w.m_load, ret);
      gtk_widget_set_sensitive(self->_w.m_edit, ret);
      gtk_widget_set_sensitive(self->_w.m_delete, ret);
      gtk_widget_set_sensitive(self->_w.m_rename, ret);
      gtk_widget_set_sensitive(self->_w.m_export, ret);
      gtk_widget_set_sensitive(self->_w.m_wav, ret);
      gtk_menu_popup(GTK_MENU(self->_w.menu), NULL, NULL, NULL, NULL, 3,
                     event->time);
    }

  /*
   *  run other event handlers too...
   */
  return false;
}

void
TapeWindow::set_selected_index(GtkTreeSelection *selection)
{
  GtkTreeModel *model;
  GList* path_list = gtk_tree_selection_get_selected_rows(selection, &model);
  if (path_list == NULL)
    return;
  
  GtkTreePath *path = (GtkTreePath *)g_list_nth_data(path_list, 0);
  gint *indices = gtk_tree_path_get_indices(path);
  if (indices == NULL)
    return;
  
  _selected = indices[0];
  
  g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
  g_list_free(path_list);
}

void
TapeWindow::allocate_colors(void)
{
    int a;
    const char *color_names[] = {
      "#000000",
      "#d00000",
      "#00d000",
      "#a0a000"
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
  char *name;
  
  GtkTreePath *path = gtk_tree_path_new_from_indices(idx, -1);
  g_assert(path != NULL);

  GtkTreeIter iter;
  gtk_tree_model_get_iter(GTK_TREE_MODEL(_w.list_store), &iter, path);
  gtk_tree_path_free(path);
  gtk_tree_model_get(GTK_TREE_MODEL(_w.list_store), &iter, 0, &name, -1);

  return name;
}

void
TapeWindow::tapeProgress(int val)
{
  _scale_value = val;
  GTK_ADJUSTMENT(_w.scale_adj)->value = val;
  gtk_signal_emit_by_name(GTK_OBJECT(_w.scale_adj), "value_changed");
}

void
TapeWindow::tapeNext(void)
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(_w.treeview));
  if (!gtk_tree_selection_get_selected(selection, &model, &iter))
    return;

  if (gtk_tree_model_iter_next(model, &iter))
    {
      // sets _selected via callback! (sf_tape_file_select)
      gtk_tree_selection_select_iter(selection, &iter);
      CMD_Args *a = new CMD_Args();
      a->set_long_arg("tape-play-delay", 10);
      CMD_EXEC_ARGS("ui-tape-play-selected", a);
    }
}

void
TapeWindow::clear_list(void)
{
  _nr_of_files = 0;
  gtk_list_store_clear(_w.list_store);
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
  char buf_load[40];
  char buf_start[40];
  char buf_size[40];

  const char *ptr_type;
  const char *ptr_load;
  const char *ptr_start;
  kct_file_type_t t = (kct_file_type_t)type;

  snprintf(buf_load, 40, "%04lxh", load);
  if (start != 0xffff)
    snprintf(buf_start, 40, "%04lxh", start);
  else
    snprintf(buf_start, 40, "-");
  snprintf(buf_size, 40, "%ld", size);

  ptr_load = "-";
  ptr_start = "-";
  switch (t) {
  case KCT_TYPE_COM:
    ptr_type = "COM";
    ptr_load = buf_load;
    ptr_start = buf_start;
    break;
  case KCT_TYPE_BAS:
    ptr_type = "BAS";
    break;
  case KCT_TYPE_DATA:
    ptr_type = "DATA";
    break;
  case KCT_TYPE_LIST:
    ptr_type = "LIST";
    break;
  case KCT_TYPE_BAS_P:
    ptr_type = "BAS*";
    break;
  case KCT_TYPE_BIN:
    ptr_type = "BIN";
    break;
  case KCT_TYPE_BASICODE:
    ptr_type = "BAC";
    break;
  default:
    ptr_type = "???";
    break;
  }

  GtkTreeIter iter;
  gtk_list_store_append(_w.list_store, &iter);
  gtk_list_store_set(_w.list_store, &iter,
		     TREEVIEW_COLUMN_NAME, name,
		     TREEVIEW_COLUMN_TYPE, ptr_type,
		     TREEVIEW_COLUMN_LOAD, ptr_load,
		     TREEVIEW_COLUMN_START, ptr_start,
		     TREEVIEW_COLUMN_SIZE, buf_size,
		     -1);

  _nr_of_files++;
  if (_nr_of_files == 1)
    {
      gtk_widget_set_sensitive(_w.b_stop, TRUE);
      gtk_widget_set_sensitive(_w.b_play, TRUE);
      gtk_widget_set_sensitive(_w.b_record, TRUE);
    }
}

void
TapeWindow::tapeRemoveFile(int idx)
{
  if (_nr_of_files == 1)
    clear_list();
  else
    {
      GtkTreePath *path = gtk_tree_path_new_from_indices(idx, -1);
      g_assert(path != NULL);

      GtkTreeIter iter;
      gtk_tree_model_get_iter(GTK_TREE_MODEL(_w.list_store), &iter, path);
      gtk_tree_path_free(path);
      gtk_list_store_remove(_w.list_store, &iter);

      _nr_of_files--;
    }
}
