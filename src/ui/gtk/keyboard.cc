/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: keyboard.cc,v 1.1 2002/10/31 01:38:12 torsten_paul Exp $
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

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/keys.h"
#include "kc/keyboard.h"

#include "cmd/cmd.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/keyboard.h"

#if 1
static struct
{
  int ch;
  const char *text;
  const char *color;
  GdkRectangle rect;
} __data[] = {
  { KC_KEY_CONTROL,    "CONTR",      "#a0a0a0", {   0,   0, 26, 8 } },
  { '1',               "1",          "#000000", {  42,   0, 26, 8 } },
  { '2',               "2",          "#000000", {  84,   0, 26, 8 } },
  { '3',               "3",          "#000000", { 126,   0, 26, 8 } },
  { '4',               "4",          "#000000", { 168,   0, 26, 8 } },
  { '5',               "5",          "#000000", { 210,   0, 26, 8 } },
  { '6',               "6",          "#000000", { 252,   0, 26, 8 } },
  { '7',               "7",          "#000000", { 294,   0, 26, 8 } },
  { '8',               "8",          "#000000", { 336,   0, 26, 8 } },
  { '9',               "9",          "#000000", { 378,   0, 26, 8 } },
  { '0',               "0",          "#000000", { 420,   0, 26, 8 } },
  { '@',               "@",          "#000000", { 462,   0, 26, 8 } },
  { KC_KEY_COLOR,      "COLOR",      "#a0a0a0", {  00,  40, 26, 8 } },
  { 'q',               "Q",          "#000000", {  42,  40, 26, 8 } },
  { 'w',               "W",          "#000000", {  84,  40, 26, 8 } },
  { 'e',               "E",          "#000000", { 126,  40, 26, 8 } },
  { 'r',               "R",          "#000000", { 168,  40, 26, 8 } },
  { 't',               "T",          "#000000", { 210,  40, 26, 8 } },
  { 'z',               "Z",          "#000000", { 252,  40, 26, 8 } },
  { 'u',               "U",          "#000000", { 294,  40, 26, 8 } },
  { 'i',               "I",          "#000000", { 336,  40, 26, 8 } },
  { 'o',               "O",          "#000000", { 378,  40, 26, 8 } },
  { 'p',               "P",          "#000000", { 420,  40, 26, 8 } },
  { '^',               "^",          "#000000", { 462,  40, 26, 8 } },
  { KC_KEY_GRAPHIC,    "GRAPHIC",    "#a0a0a0", {  00,  80, 26, 8 } },
  { 'a',               "A",          "#000000", {  42,  80, 26, 8 } },
  { 's',               "S",          "#000000", {  84,  80, 26, 8 } },
  { 'd',               "D",          "#000000", { 126,  80, 26, 8 } },
  { 'f',               "F",          "#000000", { 168,  80, 26, 8 } },
  { 'g',               "G",          "#000000", { 210,  80, 26, 8 } },
  { 'h',               "H",          "#000000", { 252,  80, 26, 8 } },
  { 'j',               "J",          "#000000", { 294,  80, 26, 8 } },
  { 'k',               "K",          "#000000", { 336,  80, 26, 8 } },
  { 'l',               "L",          "#000000", { 378,  80, 26, 8 } },
  { ':',               ":",          "#000000", { 420,  80, 26, 8 } },
  { '?',               "?",          "#000000", { 462,  80, 26, 8 } },
  { 'y',               "Y",          "#000000", {  42, 120, 26, 8 } },
  { 'x',               "X",          "#000000", {  84, 120, 26, 8 } },
  { 'c',               "C",          "#000000", { 126, 120, 26, 8 } },
  { 'v',               "V",          "#000000", { 168, 120, 26, 8 } },
  { 'b',               "B",          "#000000", { 210, 120, 26, 8 } },
  { 'n',               "N",          "#000000", { 252, 120, 26, 8 } },
  { 'm',               "M",          "#000000", { 294, 120, 26, 8 } },
  { ',',               ",",          "#000000", { 336, 120, 26, 8 } },
  { '.',               ".",          "#000000", { 378, 120, 26, 8 } },
  { ';',               ";",          "#000000", { 420, 120, 26, 8 } },
  { '=',               "=",          "#000000", { 462, 120, 26, 8 } },
  { KC_KEY_SHIFT,      "SHIFT",      "#a0a0a0", {  00, 160, 68, 8 } },
  { KC_KEY_LEFT,       "<--",        "#a0a0a0", {  84, 160, 26, 8 } },
  { KC_KEY_RIGHT,      "-->",        "#a0a0a0", { 126, 160, 26, 8 } },
  { ' ',               " ",          "#a0a0a0", { 168, 160, 68, 8 } },
  { ' ',               " ",          "#a0a0a0", { 252, 160, 68, 8 } },
  { KC_KEY_UP,         "^",          "#a0a0a0", { 336, 160, 26, 8 } },
  { KC_KEY_DOWN,       "v",          "#a0a0a0", { 378, 160, 26, 8 } },
  { KC_KEY_SHIFT,      "SHIFT",      "#a0a0a0", { 420, 160, 68, 8 } },
  { KC_KEY_PAUSE,      "PAUSE",      "#a0a0a0", { 546,   0, 26, 8 } },
  { KC_KEY_INSERT,     "INS",        "#a0a0a0", { 546,  40, 26, 8 } },
  { KC_KEY_ESC,        "ESC",        "#a0a0a0", { 546,  80, 26, 8 } },
  { KC_KEY_HOME,       "|<--",       "#a0a0a0", { 546, 120, 26, 8 } },
  { KC_KEY_SHIFT_LOCK, "SHIFT LOCK", "#a0a0a0", { 546, 160, 26, 8 } },
  { -1,                "RESET",      "#ff0000", { 588,   0, 68, 8 } },
  { KC_KEY_LIST,       "LIST",       "#a0a0a0", { 588,  40, 26, 8 } },
  { KC_KEY_RUN,        "RUN",        "#a0a0a0", { 588,  80, 68, 8 } },
  { KC_KEY_STOP,       "STOP",       "#a0a0a0", { 588, 120, 68, 8 } },
  { KC_KEY_ENTER,      "ENTER",      "#a0a0a0", { 588, 160, 68, 8 } },
  { ' ',               "",           "#000000", {   0,   0,  0, 0 } }
};
#endif

#if 0
static struct
{
  int ch;
  const char *text;
  const char *color;
  GdkRectangle rect;
} __data[] = {
  { -1,                "RES",        "#ff0000", {   0,   0, 30, 20 } },
  { KC_KEY_F1,         "ADR",        "#404040", {  40,   0, 30, 20 } },
  { KC_KEY_F2,         "DAT",        "#404040", {  80,   0, 30, 20 } },
  { '+',               "+",          "#404040", { 120,   0, 30, 20 } },
  { '-',               "-",          "#404040", { 160,   0, 30, 20 } },
  { -1,                "NMI",        "#ff0000", {   0,  35, 30, 20 } },
  { 'c',               "C",          "#ffffff", {  40,  35, 30, 20 } },
  { 'd',               "D",          "#ffffff", {  80,  35, 30, 20 } },
  { 'e',               "E",          "#ffffff", { 120,  35, 30, 20 } },
  { 'f',               "F",          "#ffffff", { 160,  35, 30, 20 } },
  { KC_KEY_F4,         "ST",         "#404040", {   0,  70, 30, 20 } },
  { '8',               "8",          "#ffffff", {  40,  70, 30, 20 } },
  { '9',               "9",          "#ffffff", {  80,  70, 30, 20 } },
  { 'a',               "A",          "#ffffff", { 120,  70, 30, 20 } },
  { 'b',               "B",          "#ffffff", { 160,  70, 30, 20 } },
  { KC_KEY_F3,         "LD",         "#404040", {   0, 105, 30, 20 } },
  { '4',               "4",          "#ffffff", {  40, 105, 30, 20 } },
  { '5',               "5",          "#ffffff", {  80, 105, 30, 20 } },
  { '6',               "6",          "#ffffff", { 120, 105, 30, 20 } },
  { '7',               "7",          "#ffffff", { 160, 105, 30, 20 } },
  { KC_KEY_ENTER,      "EX",         "#404040", {   0, 140, 30, 20 } },
  { '0',               "0",          "#ffffff", {  40, 140, 30, 20 } },
  { '1',               "1",          "#ffffff", {  80, 140, 30, 20 } },
  { '2',               "2",          "#ffffff", { 120, 140, 30, 20 } },
  { '3',               "3",          "#ffffff", { 160, 140, 30, 20 } },
  { ' ',               "",           "#000000", {   0,   0,  0,  0 } }
};
#endif

class CMD_ui_keyboard_window_toggle : public CMD
{
private:
  KeyboardWindow *_w;
  
public:
  CMD_ui_keyboard_window_toggle(KeyboardWindow *w) : CMD("ui-keyboard-window-toggle")
    {
      _w = w;
      register_cmd("ui-keyboard-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

static KeyboardWindow *kb;

KeyboardWindow::KeyboardWindow(void)
{
  _font = (GdkFont *)22;
  kb = this;
  _gc = NULL;
  _font = NULL;
  _colormap = NULL;
  init();
}

KeyboardWindow::~KeyboardWindow(void)
{
}

void
KeyboardWindow::sf_expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  int a;
  GdkColor color;
  KeyboardWindow *self = (KeyboardWindow *)data;
  GdkDrawable *drawable = GTK_WIDGET(widget)->window;

  if (self->_gc == NULL)
    self->_gc = gdk_gc_new(drawable);
  if (self->_font == NULL)
    self->_font = gdk_font_load("fixed");
  if (self->_colormap == NULL)
    self->_colormap = gdk_colormap_get_system();

  gdk_color_parse("#000000", &color);
  gdk_color_alloc(self->_colormap, &color);
  gdk_gc_set_foreground(self->_gc, &color);

  if ((get_kc_type() & KC_TYPE_85_1_CLASS) == 0)
    {
      gdk_draw_string(drawable, self->_font, self->_gc, 40, 40, _("comming soon..."));
      return;
    }

  for (a = 0;__data[a].rect.width > 0;a++)
    gdk_draw_string(drawable, self->_font, self->_gc,
		    __data[a].rect.x, __data[a].rect.y + 20,
		    __data[a].text);

  for (a = 0;__data[a].rect.width > 0;a++)
    {
      gdk_color_parse(__data[a].color, &color);
      gdk_color_alloc(self->_colormap, &color);
      gdk_gc_set_foreground(self->_gc, &color);
      gdk_draw_rectangle(drawable, self->_gc, 1,
                         __data[a].rect.x, __data[a].rect.y,
                         __data[a].rect.width, __data[a].rect.height);
    }

}

void
KeyboardWindow::sf_button_press(GtkWidget */*widget*/, GdkEventButton *event, gpointer data)
{
  KeyboardWindow *self = (KeyboardWindow *)data;
  self->button_press(event, true);
}

void
KeyboardWindow::sf_button_release(GtkWidget */*widget*/, GdkEventButton *event, gpointer data)
{
  KeyboardWindow *self = (KeyboardWindow *)data;
  self->button_press(event, false);
}

void
KeyboardWindow::button_press(GdkEventButton *event, bool press)
{
  int a, c;

  if ((get_kc_type() & KC_TYPE_85_1_CLASS) == 0)
    return;

  c = -1;
  for (a = 0;__data[a].rect.width > 0;a++)
    {
      if ((event->x < __data[a].rect.x) || (event->x > (__data[a].rect.x + __data[a].rect.width)))
        continue;
      if ((event->y < __data[a].rect.y) || (event->y > (__data[a].rect.y + __data[a].rect.height)))
        continue;
      c = __data[a].ch;

      if (c < 0)
	{
	  /*
	   *  special handling for reset key which is not
	   *  part of the keyboard matrix
	   */
	  CMD_EXEC("emu-reset");
	  return;
	}

      break;
    }
  if (c < 0)
    return;

  if (press)
    {
      switch (event->button)
        {
        case 2:
          keyboard->keyPressed(KC_KEY_CONTROL, KC_KEY_CONTROL);
          break;
        case 3:
          keyboard->keyPressed(KC_KEY_SHIFT, KC_KEY_SHIFT);
          break;
        }

      keyboard->keyPressed(c, c);
    }
  else
    {
      keyboard->keyReleased(c, c);
      switch (event->button)
        {
        case 2:
          keyboard->keyReleased(KC_KEY_CONTROL, KC_KEY_CONTROL);
          break;
        case 3:
          keyboard->keyReleased(KC_KEY_SHIFT, KC_KEY_SHIFT);
          break;
        }
    }
}

void
KeyboardWindow::init(void)
{
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_window, "KeyboardWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Keyboard"));
  gtk_window_set_resizable(GTK_WINDOW(_window), false);
  gtk_window_position(GTK_WINDOW(_window), GTK_WIN_POS_MOUSE);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-keyboard-window-toggle"); // FIXME:

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 2);
  gtk_container_border_width(GTK_CONTAINER(_w.vbox), 6);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  canvas
   */
  _w.canvas = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(_w.canvas), 656, 180);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.canvas, FALSE, TRUE, 0);
  gtk_widget_show(_w.canvas);
  gtk_signal_connect(GTK_OBJECT(_w.canvas), "button_press_event",
                     GTK_SIGNAL_FUNC(sf_button_press), this);
  gtk_signal_connect(GTK_OBJECT(_w.canvas), "button_release_event",
                     GTK_SIGNAL_FUNC(sf_button_release), this);
  gtk_widget_set_events(_w.canvas, (GDK_BUTTON_PRESS_MASK |
                                    GDK_BUTTON_RELEASE_MASK |
                                    GDK_PROPERTY_CHANGE_MASK |
                                    GDK_EXPOSURE_MASK));
  gtk_signal_connect(GTK_OBJECT(_w.canvas), "expose_event",
                     GTK_SIGNAL_FUNC(sf_expose), this);

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
                     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);
  
  /*
   *  close button
   */
  _w.close = gtk_button_new_with_label(_("Close"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.close,
                     FALSE, FALSE, 5);
  gtk_signal_connect(GTK_OBJECT(_w.close), "clicked",
                     GTK_SIGNAL_FUNC(cmd_exec_sf),
                     (gpointer)"ui-keyboard-window-toggle");
  GTK_WIDGET_SET_FLAGS(_w.close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.close);
  gtk_widget_show(_w.close);

  CMD *cmd;
  cmd = new CMD_ui_keyboard_window_toggle(this);
}
