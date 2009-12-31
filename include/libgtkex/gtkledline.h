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
