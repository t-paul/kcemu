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

#ifndef __libgtkex_gtkfilebutton_h
#define __libgtkex_gtkfilebutton_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_FILEBUTTON (gtk_filebutton_get_type())
#define GTK_FILEBUTTON(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_FILEBUTTON, GtkFilebutton))
#define GTK_FILEBUTTON_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST((obj), GTK_FILEBUTTON, GtkFilebuttonClass))
#define GTK_IS_FILEBUTTON(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_FILEBUTTON))
#define GTK_IS_FILEBUTTON_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((obj), GTK_TYPE_FILEBUTTON))
#define GTK_FILEBUTTON_GET_CLASS (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_FILEBUTTON, GtkFilebuttonClass))

typedef struct _GtkFilebutton GtkFilebutton;
typedef struct _GtkFilebuttonClass GtkFilebuttonClass;

struct _GtkFilebutton
{
  GtkTable parent;

  GtkWidget *button_open;
  GtkWidget *button_clear;
  GtkWidget *comboboxentry;

  GtkListStore *store;

  gchar *filename;

  gchar *recent_group;

  guint changed_id;
};

struct _GtkFilebuttonClass
{
  GtkTableClass parent_class;

  void (* filebutton) (GtkFilebutton *filebutton);
};

GType gtk_filebutton_get_type(void);

GtkWidget *gtk_filebutton_new(const gchar *recent_group);
gchar * gtk_filebutton_get_filename(GtkFilebutton *filebutton);
void gtk_filebutton_set_recent_group(GtkFilebutton *filebutton, const gchar *recent_group);

G_END_DECLS

#endif /* __libgtkex_gtkfilebutton_h */