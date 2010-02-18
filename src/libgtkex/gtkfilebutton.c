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

#include <string.h>

#include "libgtkex/libgtkex.h"

enum {
  FILEBUTTON_CHANGED_SIGNAL,
  LAST_SIGNAL
};

static guint filebutton_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE(GtkFilebutton, gtk_filebutton, GTK_TYPE_TABLE);

static GtkWidget *
create_button(const gchar *stock_id)
{
  GtkWidget *button, *image;

  button = gtk_button_new();
  image = gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_BUTTON);
  gtk_container_add(GTK_CONTAINER(button), image);
  gtk_widget_show(image);
  gtk_widget_show(button);
  return button;
}

static GtkWidget *
create_comboboxentry(void)
{
  GtkWidget *comboboxentry, *entry;

  comboboxentry = gtk_combo_box_entry_new();
  gtk_combo_box_set_active(GTK_COMBO_BOX(comboboxentry), -1);
  gtk_combo_box_entry_set_text_column(GTK_COMBO_BOX_ENTRY(comboboxentry), 0);
  entry = gtk_bin_get_child(GTK_BIN(comboboxentry));
  gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
  gtk_widget_show(comboboxentry);
  return comboboxentry;
}

static void
add_recent_info(const gchar *filename, const gchar *group)
{
  GtkRecentManager *manager = gtk_recent_manager_get_default();

  GFile *file = g_file_new_for_path(filename);
  gchar *uri = g_file_get_uri(file);
  g_object_unref(file);

  gchar *groups[2];
  GtkRecentData recent_data;
  memset(&recent_data, 0, sizeof(recent_data));
  recent_data.mime_type = "application/octet-stream";
  recent_data.app_name = "KCemu";
  recent_data.app_exec = "xdg-open %u";
  if (group != NULL)
    {
      recent_data.groups = groups;
      recent_data.groups[0] = group;
      recent_data.groups[1] = NULL;
    }

  gtk_recent_manager_add_full(manager, uri, &recent_data);
  g_free(uri);
}

static GtkListStore *
get_list_store(const gchar *recent_group)
{
  GtkRecentManager *manager = gtk_recent_manager_get_default();
  
  GtkListStore *store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

  GList *ptr;
  int count = 0;
  GtkTreeIter iter;
  GList *list = gtk_recent_manager_get_items(manager);
  for (ptr = list; (ptr != NULL) && (count < 10); ptr = ptr->next)
    {
      GtkRecentInfo *info = ptr->data;
      if (!gtk_recent_info_has_application(info, "KCemu"))
        continue;
      if ((recent_group != NULL) && !gtk_recent_info_has_group(info, recent_group))
        continue;

      GFile *file = g_file_new_for_uri(gtk_recent_info_get_uri(info));
      if (g_file_is_native(file) /* && g_file_query_exists(file, NULL) */)
        {
          gchar *path = g_file_get_path(file);
          gchar *basename = g_file_get_basename(file);

          if ((path != NULL) && (basename != NULL))
            {
              gtk_list_store_append(store, &iter);
              gtk_list_store_set(store, &iter, 0, basename, 1, path, -1);
              count++;
            }

          g_free(path);
          g_free(basename);
        }
      g_object_unref(file);
    }
  
  return store;
}

GtkWidget*
gtk_filebutton_new(const gchar *recent_group)
{
  GtkWidget *widget = GTK_WIDGET(g_object_new(gtk_filebutton_get_type(), NULL));
  gtk_filebutton_set_recent_group(GTK_FILEBUTTON(widget), recent_group);
  return widget;
}

gchar *
gtk_filebutton_get_filename(GtkFilebutton *filebutton)
{
  g_return_val_if_fail(filebutton != NULL, NULL);
  g_return_val_if_fail(GTK_IS_FILEBUTTON(filebutton), NULL);

  return g_strdup(filebutton->filename);
}

void
gtk_filebutton_set_recent_group(GtkFilebutton *filebutton, const gchar *recent_group)
{
  g_return_if_fail(filebutton != NULL);
  g_return_if_fail(GTK_IS_FILEBUTTON(filebutton));

  // Prepare for the case that we are called with the old value of
  // filebutton->recent_group. In this case we need to prevent to
  // access the old value after it is already feed.
  gchar *old = filebutton->recent_group;
  filebutton->recent_group = g_strdup(recent_group);
  g_free(old);

  GtkListStore *store = get_list_store(filebutton->recent_group);
  gtk_combo_box_set_model(GTK_COMBO_BOX(filebutton->comboboxentry), GTK_TREE_MODEL(store));
  g_object_unref(store);
}

static void
gtk_filebutton_set_entry_text(GtkFilebutton *filebutton, const gchar *text)
{
  GtkEntry *entry = GTK_ENTRY(gtk_bin_get_child(GTK_BIN(filebutton->comboboxentry)));
  
  g_signal_handler_block(filebutton->comboboxentry, filebutton->changed_id);
  gtk_entry_set_text(entry, text);
  g_signal_handler_unblock(filebutton->comboboxentry, filebutton->changed_id);
}

static void
gtk_filebutton_set_filename(GtkFilebutton *filebutton, const gchar *filename)
{
  if (g_strcmp0(filebutton->filename, filename) == 0)
    return;

  g_free(filebutton->filename);
  filebutton->filename = g_strdup(filename);

  const gchar *basename;
  if (filename == NULL)
    {
      filename = "";
      basename = "";
    }
  else
    {
      GFile *file = g_file_new_for_path(filename);
      basename = g_file_get_basename(file);
      g_object_unref(file);

      add_recent_info(filename, filebutton->recent_group);
      gtk_filebutton_set_recent_group(filebutton, filebutton->recent_group);
    }

  gtk_filebutton_set_entry_text(filebutton, basename);
  gtk_widget_set_tooltip_text(GTK_WIDGET(filebutton), filename);
  gtk_widget_set_sensitive(GTK_WIDGET(filebutton->button_clear), filebutton->filename != NULL);

  g_signal_emit(G_OBJECT(filebutton), filebutton_signals[FILEBUTTON_CHANGED_SIGNAL], 0);
}

static void
gtk_filebutton_class_init(GtkFilebuttonClass *class)
{
  filebutton_signals[FILEBUTTON_CHANGED_SIGNAL] = g_signal_new("changed",
                  G_TYPE_FROM_CLASS(class),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET(GtkFilebuttonClass, filebutton),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}

static void
on_comboboxentry_changed(GtkComboBoxEntry *comboboxentry, GtkFilebutton *filebutton)
{
  GtkTreeIter iter;
  if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(comboboxentry), &iter))
    {
      gchar *filename;
      GtkTreeModel *model = GTK_TREE_MODEL(gtk_combo_box_get_model(GTK_COMBO_BOX(comboboxentry)));
      gtk_tree_model_get(model, &iter, 1, &filename, -1);
      gtk_filebutton_set_filename(filebutton, filename);
      g_free(filename);
    }
  else
    {
      gtk_filebutton_set_filename(filebutton, NULL);
    }
}

static void
on_filebutton_clear(GtkButton *button, GtkFilebutton *filebutton)
{
  gtk_filebutton_set_filename(filebutton, NULL);
}

static void
on_filebutton_open(GtkButton *button, GtkFilebutton *filebutton)
{
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new("Select file",
                                       GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(filebutton))),
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
      gtk_filebutton_set_filename(filebutton, filename);
      g_free(filename);
    }
  gtk_widget_destroy(dialog);
}

static void
gtk_filebutton_init(GtkFilebutton *filebutton)
{
  filebutton->button_open = create_button(GTK_STOCK_OPEN);
  filebutton->button_clear = create_button(GTK_STOCK_CLOSE);
  filebutton->comboboxentry = create_comboboxentry();

  gtk_widget_set_sensitive(GTK_WIDGET(filebutton->button_clear), FALSE);

  filebutton->changed_id = g_signal_connect(filebutton->comboboxentry, "changed", G_CALLBACK(on_comboboxentry_changed), filebutton);
  g_signal_connect(filebutton->button_open, "clicked", G_CALLBACK(on_filebutton_open), filebutton);
  g_signal_connect(filebutton->button_clear, "clicked", G_CALLBACK(on_filebutton_clear), filebutton);
  
  gtk_table_resize(GTK_TABLE(filebutton), 1, 3);
  gtk_table_attach(GTK_TABLE(filebutton), filebutton->comboboxentry, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
  gtk_table_attach(GTK_TABLE(filebutton), filebutton->button_clear, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 2, 0);
  gtk_table_attach(GTK_TABLE(filebutton), filebutton->button_open, 2, 3, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
}