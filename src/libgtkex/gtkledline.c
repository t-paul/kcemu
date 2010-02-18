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

#include <math.h>
#include <cairo/cairo.h>

#include "libgtkex/gtkledline.h"

G_DEFINE_TYPE(GtkLedLine, gtk_led_line, GTK_TYPE_DRAWING_AREA);

GtkWidget *
gtk_led_line_new(guint number_of_leds)
{
  if (number_of_leds == 0)
    number_of_leds = 1;
  
  GtkLedLine *led_line = GTK_LED_LINE(g_object_new(GTK_TYPE_LED_LINE, NULL));
  led_line->number_of_leds = number_of_leds;
  return GTK_WIDGET(led_line);
}

static void
draw(GtkWidget *widget, cairo_t *cr)
{
  int a;
  GtkLedLine *led_line;
  double x, y, radius, rx, ry;

  led_line = GTK_LED_LINE(widget);

  rx = widget->allocation.width / led_line->number_of_leds / 2 - 4;
  ry = widget->allocation.height / 2 - 4;

  radius = MIN(rx, ry);

  x = radius + 2;
  y = widget->allocation.height / 2;
  for (a = 0; a < led_line->number_of_leds; a++)
    {
      cairo_arc(cr, x, y, radius, 0, 2 * M_PI);
      cairo_set_source_rgb(cr, 1, 1, 1);
      cairo_fill_preserve(cr);
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_stroke(cr);
      x += 2 * radius + led_line->spacing;
    }
}

static void
gtk_led_line_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_LED_LINE(widget));
  g_return_if_fail(requisition != NULL);

  GtkLedLine *led_line = GTK_LED_LINE(widget);
  
  requisition->width = (20 + led_line->spacing) * led_line->number_of_leds - led_line->spacing;
  requisition->height = 20;
}

static void
gtk_led_line_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_LED_LINE(widget));
  g_return_if_fail(allocation != NULL);

  printf("%dx%d+%d+%d\n", allocation->width, allocation->height, allocation->x, allocation->y);

  widget->allocation = *allocation;
  if (GTK_WIDGET_REALIZED(widget))
    gdk_window_move_resize(widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
}

static gboolean
gtk_led_line_expose(GtkWidget *widget, GdkEventExpose *event)
{
  cairo_t *cr;

  cr = gdk_cairo_create(widget->window);

  cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
  cairo_clip(cr);

  draw(widget, cr);

  cairo_destroy(cr);

  return FALSE;
}


static void
gtk_led_line_class_init(GtkLedLineClass *class)
{
  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS(class);

  widget_class->size_request = gtk_led_line_size_request;
  widget_class->size_allocate = gtk_led_line_size_allocate;
  widget_class->expose_event = gtk_led_line_expose;
}

static void
gtk_led_line_init(GtkLedLine *led_line)
{
  led_line->spacing = 2;
}
