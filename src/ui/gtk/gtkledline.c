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
 *  This is a LED line. (c) 1998 Torsten Paul
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
#if 0
static gint gtk_led_line_button_press             (GtkWidget          *widget,
						   GdkEventButton     *event);
static gint gtk_led_line_button_release           (GtkWidget          *widget,
						   GdkEventButton     *event);
static void gtk_led_line_update_mouse             (GtkLedLine         *led_line,
						   gint                x,
						   gint                y);
static void gtk_led_line_update                   (GtkLedLine         *led_line);
static void gtk_led_line_adjustment_changed       (GtkAdjustment      *adjustment,
						   gpointer            data);
static void gtk_led_line_adjustment_value_changed (GtkAdjustment      *adjustment,
						   gpointer            data);
#endif

/* Local data */

static GtkWidgetClass *parent_class = NULL;

guint
gtk_led_line_get_type ()
{
  static guint led_line_type = 0;

  if (!led_line_type)
    {
      GtkTypeInfo led_line_info =
      {
	"GtkLedLine",
	sizeof (GtkLedLine),
	sizeof (GtkLedLineClass),
	(GtkClassInitFunc) gtk_led_line_class_init,
	(GtkObjectInitFunc) gtk_led_line_init,
	(GtkArgSetFunc) NULL,
	(GtkArgGetFunc) NULL,
      };

      led_line_type = gtk_type_unique (gtk_widget_get_type (), &led_line_info);
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
  /* widget_class->button_press_event = gtk_led_line_button_press; */
  /* widget_class->button_release_event = gtk_led_line_button_release; */
  /* widget_class->motion_notify_event = gtk_led_line_motion_notify; */
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

#if 0
static gint
gtk_dial_button_press (GtkWidget      *widget,
		       GdkEventButton *event)
{
  GtkDial *dial;
  gint dx, dy;
  double s, c;
  double d_parallel;
  double d_perpendicular;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  dial = GTK_DIAL (widget);

  /* Determine if button press was within pointer region - we 
     do this by computing the parallel and perpendicular distance of
     the point where the mouse was pressed from the line passing through
     the pointer */
  
  dx = event->x - widget->allocation.width / 2;
  dy = widget->allocation.height / 2 - event->y;
  
  s = sin(dial->angle);
  c = cos(dial->angle);
  
  d_parallel = s*dy + c*dx;
  d_perpendicular = fabs(s*dx - c*dy);
  
  if (!dial->button &&
      (d_perpendicular < dial->pointer_width/2) &&
      (d_parallel > - dial->pointer_width))
    {
      gtk_grab_add (widget);

      dial->button = event->button;

      gtk_dial_update_mouse (dial, event->x, event->y);
    }

  return FALSE;
}

static gint
gtk_dial_button_release (GtkWidget      *widget,
			  GdkEventButton *event)
{
  GtkDial *dial;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  dial = GTK_DIAL (widget);

  if (dial->button == event->button)
    {
      gtk_grab_remove (widget);

      dial->button = 0;

      if (dial->policy == GTK_UPDATE_DELAYED)
	gtk_timeout_remove (dial->timer);
      
      if ((dial->policy != GTK_UPDATE_CONTINUOUS) &&
	  (dial->old_value != dial->adjustment->value))
	gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");
    }

  return FALSE;
}

static gint
gtk_dial_motion_notify (GtkWidget      *widget,
			 GdkEventMotion *event)
{
  GtkDial *dial;
  GdkModifierType mods;
  gint x, y, mask;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  dial = GTK_DIAL (widget);

  if (dial->button != 0)
    {
      x = event->x;
      y = event->y;

      if (event->is_hint || (event->window != widget->window))
	gdk_window_get_pointer (widget->window, &x, &y, &mods);

      switch (dial->button)
	{
	case 1:
	  mask = GDK_BUTTON1_MASK;
	  break;
	case 2:
	  mask = GDK_BUTTON2_MASK;
	  break;
	case 3:
	  mask = GDK_BUTTON3_MASK;
	  break;
	default:
	  mask = 0;
	  break;
	}

      if (mods & mask)
	gtk_dial_update_mouse (dial, x,y);
    }

  return FALSE;
}

static gint
gtk_dial_timer (GtkDial *dial)
{
  g_return_val_if_fail (dial != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_DIAL (dial), FALSE);

  if (dial->policy == GTK_UPDATE_DELAYED)
    gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");

  return FALSE;
}

static void
gtk_dial_update_mouse (GtkDial *dial, gint x, gint y)
{
  gint xc, yc;
  gfloat old_value;

  g_return_if_fail (dial != NULL);
  g_return_if_fail (GTK_IS_DIAL (dial));

  xc = GTK_WIDGET(dial)->allocation.width / 2;
  yc = GTK_WIDGET(dial)->allocation.height / 2;

  old_value = dial->adjustment->value;
  dial->angle = atan2(yc-y, x-xc);

  if (dial->angle < -M_PI/2.)
    dial->angle += 2*M_PI;

  if (dial->angle < -M_PI/6)
    dial->angle = -M_PI/6;

  if (dial->angle > 7.*M_PI/6.)
    dial->angle = 7.*M_PI/6.;

  dial->adjustment->value = dial->adjustment->lower + (7.*M_PI/6 - dial->angle) *
    (dial->adjustment->upper - dial->adjustment->lower) / (4.*M_PI/3.);

  if (dial->adjustment->value != old_value)
    {
      if (dial->policy == GTK_UPDATE_CONTINUOUS)
	{
	  gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");
	}
      else
	{
	  gtk_widget_draw (GTK_WIDGET(dial), NULL);

	  if (dial->policy == GTK_UPDATE_DELAYED)
	    {
	      if (dial->timer)
		gtk_timeout_remove (dial->timer);

	      dial->timer = gtk_timeout_add (SCROLL_DELAY_LENGTH,
					     (GtkFunction) gtk_dial_timer,
					     (gpointer) dial);
	    }
	}
    }
}

static void
gtk_dial_update (GtkDial *dial)
{
  gfloat new_value;
  
  g_return_if_fail (dial != NULL);
  g_return_if_fail (GTK_IS_DIAL (dial));

  new_value = dial->adjustment->value;
  
  if (new_value < dial->adjustment->lower)
    new_value = dial->adjustment->lower;

  if (new_value > dial->adjustment->upper)
    new_value = dial->adjustment->upper;

  if (new_value != dial->adjustment->value)
    {
      dial->adjustment->value = new_value;
      gtk_signal_emit_by_name (GTK_OBJECT (dial->adjustment), "value_changed");
    }

  dial->angle = 7.*M_PI/6. - (new_value - dial->adjustment->lower) * 4.*M_PI/3. /
    (dial->adjustment->upper - dial->adjustment->lower);

  gtk_widget_draw (GTK_WIDGET(dial), NULL);
}

static void
gtk_dial_adjustment_changed (GtkAdjustment *adjustment,
			      gpointer       data)
{
  GtkDial *dial;

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  dial = GTK_DIAL (data);

  if ((dial->old_value != adjustment->value) ||
      (dial->old_lower != adjustment->lower) ||
      (dial->old_upper != adjustment->upper))
    {
      gtk_dial_update (dial);

      dial->old_value = adjustment->value;
      dial->old_lower = adjustment->lower;
      dial->old_upper = adjustment->upper;
    }
}

static void
gtk_dial_adjustment_value_changed (GtkAdjustment *adjustment,
				    gpointer       data)
{
  GtkDial *dial;

  g_return_if_fail (adjustment != NULL);
  g_return_if_fail (data != NULL);

  dial = GTK_DIAL (data);

  if (dial->old_value != adjustment->value)
    {
      gtk_dial_update (dial);

      dial->old_value = adjustment->value;
    }
}

#endif
