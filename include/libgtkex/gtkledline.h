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

#ifndef __libgtkex_gtkledline_h
#define __libgtkex_gtkledline_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_LED_LINE (gtk_led_line_get_type())
#define GTK_LED_LINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_LED_LINE, GtkLedLine))
#define GTK_LED_LINE_CLASS(obj) (G_TYPE_CHECK_CLASS_CAST((obj), GTK_LED_LINE, GtkLedLineClass))
#define GTK_IS_LED_LINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_LED_LINE))
#define GTK_IS_LED_LINE_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((obj), GTK_TYPE_LED_LINE))
#define GTK_LED_LINE_GET_CLASS (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_LED_LINE, GtkLedLineClass))

typedef struct _GtkLedLine GtkLedLine;
typedef struct _GtkLedLineClass GtkLedLineClass;

struct _GtkLedLine
{
  GtkDrawingArea parent;

  guint spacing;
  
  guint number_of_leds;
};

struct _GtkLedLineClass
{
  GtkDrawingAreaClass parent_class;
};

GtkWidget *gtk_led_line_new(guint number_of_leds);

G_END_DECLS

#endif /* __libgtkex_gtkledline_h */
