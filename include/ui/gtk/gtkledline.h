/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GTK_LED_LINE_H__
#define __GTK_LED_LINE_H__


#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_LED_LINE            (gtk_led_line_get_type())
#define GTK_LED_LINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_LED_LINE, GtkLedLine))
#define GTK_LED_LINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_LED_LINE, GtkLedLineClass))
#define GTK_IS_LED_LINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_LED_LINE))
#define GTK_IS_LED_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_LED_LINE))
#define GTK_LED_LINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_LED_LINE, GtkLedLineClass))

typedef struct _GtkLedLine        GtkLedLine;
typedef struct _GtkLedLineClass   GtkLedLineClass;

struct _GtkLedLine
{
  GtkWidget widget;

  gint    nr;
  guint32 value;
};

struct _GtkLedLineClass
{
  GtkWidgetClass parent_class;
};


GtkWidget*     gtk_led_line_new                 (gint nr);
GtkType        gtk_led_line_get_type            (void);
void           gtk_led_line_set_value           (GtkLedLine *led_line,
                                                 guint32 value);
void           gtk_led_line_set_led             (GtkLedLine *led_line,
                                                 int led_nr,
                                                 gboolean led_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_LED_LINE_H__ */
