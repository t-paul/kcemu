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

/*
 *  This is a LED line, based on the gtkdial code. (c) 1998,2003 Torsten Paul
 */
#include <math.h>
#include <stdio.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

#include "ui/gtk/gtkledline.h"

#define LED_LINE_NR_OF_POINTS (21)
#define LED_LINE_DEFAULT_RADIUS (4)
#define LED_LINE_DEFAULT_SPACING (2)

/* Forward declararations */

static void gtk_led_line_class_init               (GtkLedLineClass    *klass);
static void gtk_led_line_init                     (GtkLedLine         *led_line);
static void gtk_led_line_destroy                  (GtkObject          *object);
static void gtk_led_line_realize                  (GtkWidget          *widget);
static void gtk_led_line_size_request             (GtkWidget          *widget,
					           GtkRequisition     *requisition);
static void gtk_led_line_size_allocate            (GtkWidget          *widget,
					           GtkAllocation      *allocation);
static gint gtk_led_line_expose                   (GtkWidget          *widget,
						   GdkEventExpose     *event);
/* Local data */

static GtkWidgetClass *parent_class = NULL;

GtkType
gtk_led_line_get_type ()
{
  static GtkType led_line_type = 0;

  if (!led_line_type)
    {
      static const GtkTypeInfo led_line_info =
      {
	"GtkLedLine",
	sizeof (GtkLedLine),
	sizeof (GtkLedLineClass),
	(GtkClassInitFunc) gtk_led_line_class_init,
	(GtkObjectInitFunc) gtk_led_line_init,
	/* reserved_1 */ NULL,
	/* reserved_1 */ NULL,
	(GtkClassInitFunc) NULL,
      };

      led_line_type = gtk_type_unique (GTK_TYPE_WIDGET, &led_line_info);
    }

  return led_line_type;
}

static void
gtk_led_line_class_init (GtkLedLineClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = gtk_type_class (gtk_widget_get_type ());

  object_class->destroy = gtk_led_line_destroy;

  widget_class->realize = gtk_led_line_realize;
  widget_class->expose_event = gtk_led_line_expose;
  widget_class->size_request = gtk_led_line_size_request;
  widget_class->size_allocate = gtk_led_line_size_allocate;
}

static void
gtk_led_line_init (GtkLedLine *led_line)
{
  led_line->value = 0;
}

GtkWidget*
gtk_led_line_new (gint nr)
{
  GtkLedLine *led_line;

  led_line = gtk_type_new (gtk_led_line_get_type ());

  g_return_val_if_fail (nr > 0, NULL);
  g_return_val_if_fail (nr <= 32, NULL);
  
  led_line->nr = nr;
  led_line->value = 0;
  
  return GTK_WIDGET (led_line);
}

static void
gtk_led_line_destroy (GtkObject *object)
{
  GtkLedLine *led_line;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_LED_LINE (object));

  led_line = GTK_LED_LINE (object);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_led_line_realize (GtkWidget *widget)
{
  GtkLedLine *led_line;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LED_LINE (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  led_line = GTK_LED_LINE (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = gtk_widget_get_events (widget) | 
    GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
    GDK_POINTER_MOTION_HINT_MASK;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

  gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
}

static void 
gtk_led_line_size_request (GtkWidget      *widget,
		       GtkRequisition *requisition)
{
  GtkLedLine *led_line;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LED_LINE (widget));

  led_line = GTK_LED_LINE (widget);

  requisition->width = (2 * led_line->nr * LED_LINE_DEFAULT_RADIUS +
                        (led_line->nr + 1) * LED_LINE_DEFAULT_SPACING) + 1;
  requisition->height = (2 * LED_LINE_DEFAULT_RADIUS +
                         2 * LED_LINE_DEFAULT_SPACING);
}

static void
gtk_led_line_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation)
{
  GtkLedLine *led_line;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_LED_LINE (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  led_line = GTK_LED_LINE (widget);

  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
                            allocation->x, allocation->y,
                            allocation->width, allocation->height);
}

static gint
gtk_led_line_expose (GtkWidget      *widget,
                     GdkEventExpose *event)
{
  gint a, b;
  GdkGC *gc;
  gint xc, yc;
  gdouble s,c;
  gdouble theta;
  GdkPoint points[LED_LINE_NR_OF_POINTS];
  GtkLedLine *led_line;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_LED_LINE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;
  
  led_line = GTK_LED_LINE (widget);

  gdk_window_clear_area (widget->window,
			 0, 0,
			 widget->allocation.width,
			 widget->allocation.height);
  /*
  gdk_draw_rectangle (widget->window,
                      widget->style->dark_gc[GTK_STATE_NORMAL],
                      TRUE,
                      0, 0,
                      widget->allocation.width,
                      widget->allocation.height);
                      */

  xc = LED_LINE_DEFAULT_SPACING + LED_LINE_DEFAULT_RADIUS;
  yc = widget->allocation.height / 2;

  for (a = 0;a < led_line->nr;a++)
    {
      for (b = 0;b < LED_LINE_NR_OF_POINTS;b++)
        {
          theta = (b * (2.0 * M_PI / (LED_LINE_NR_OF_POINTS - 1)));
          s = sin(theta);
          c = cos(theta);
          points[b].x = xc + c * LED_LINE_DEFAULT_RADIUS + .5;
          points[b].y = yc - s * LED_LINE_DEFAULT_RADIUS + .5;
        }
      xc += 2 * LED_LINE_DEFAULT_RADIUS + LED_LINE_DEFAULT_SPACING;
      if (led_line->value & (1 << a))
        gc = widget->style->fg_gc[GTK_STATE_NORMAL];
      else
        gc = widget->style->bg_gc[GTK_STATE_NORMAL];
      
      gdk_draw_polygon (widget->window,
                        gc,
                        TRUE,
                        points,
                        LED_LINE_NR_OF_POINTS);
      gdk_draw_polygon (widget->window,
                        widget->style->black_gc,
                        FALSE,
                        points,
                        LED_LINE_NR_OF_POINTS);
    }
  
  return FALSE;
}

void
gtk_led_line_set_value (GtkLedLine *led_line,
                        guint32 value)
{
  led_line->value = value;
  gtk_widget_queue_draw (GTK_WIDGET (led_line));
}

void
gtk_led_line_set_led(GtkLedLine *led_line,
                     int led_nr,
                     gboolean led_val)
{
  if (led_val)
    led_line->value |= (1 << led_nr);
  else
    led_line->value &= ~(1 << led_nr);
  gtk_widget_queue_draw (GTK_WIDGET (led_line));
}
