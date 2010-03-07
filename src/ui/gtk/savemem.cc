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

#include <stdio.h>
#include <stdlib.h>

#include "kc/system.h"

#include "cmd/cmd.h"

#include "kc/kc.h"
#include "kc/memory.h"

#include "ui/gtk/savemem.h"
#include "ui/gtk/cmd.h"

#include "libdbg/dbg.h"

class CMD_ui_save_memory_window_toggle : public CMD
{
private:
  SaveMemoryWindow *_w;
  
public:
  CMD_ui_save_memory_window_toggle(SaveMemoryWindow *w) : CMD("ui-save-memory-window-toggle")
    {
      _w = w;
      register_cmd("ui-save-memory-window-toggle");
    }
  
  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

SaveMemoryWindow::SaveMemoryWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  init();

  _cmd_window_toggle = new CMD_ui_save_memory_window_toggle(this);
}

SaveMemoryWindow::~SaveMemoryWindow(void)
{
  delete _cmd_window_toggle;
}

gboolean
SaveMemoryWindow::on_output(GtkSpinButton *spin_button, gpointer user_data)
{
   GtkAdjustment *adj = gtk_spin_button_get_adjustment(spin_button);
   int value = (int)gtk_adjustment_get_value(adj);
   gchar *text = g_strdup_printf("%04X", value);
   gtk_entry_set_text(GTK_ENTRY(spin_button), text);
   g_free(text);

  return TRUE;
}

gboolean
SaveMemoryWindow::on_input(GtkSpinButton *spin_button, gdouble *new_value, gpointer user_data)
{
  const gchar *text = gtk_entry_get_text(GTK_ENTRY(spin_button));

  gchar *err = NULL;
  unsigned long value = strtoul(text, &err, 16);
  if (*err)
    return GTK_INPUT_ERROR;
    
  *new_value = (gdouble)value;
  return TRUE;
}

void
SaveMemoryWindow::on_adjustment_start_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;

  gdouble value_start = gtk_adjustment_get_value(adjustment);
  gdouble value_end = gtk_adjustment_get_value(self->_w.adjustment_end);

  if (value_end < value_start)
    {
      value_end = value_start;
      self->set_adjustment_value_with_blocked_handler(self->_w.adjustment_end, value_end, self->_w.on_adjustment_end_value_changed_id);
    }

  self->set_length_adjustment();
  self->apply_selection((int)gtk_adjustment_get_value(self->_w.adjustment_start), (int)gtk_adjustment_get_value(self->_w.adjustment_end));
    
  on_jump_to_start(self->_w.toggle_button_jump_to_start, self);
}

void
SaveMemoryWindow::on_adjustment_end_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;

  gdouble value_start = gtk_adjustment_get_value(self->_w.adjustment_start);
  gdouble value_end = gtk_adjustment_get_value(adjustment);

  if (value_end < value_start)
    {
      value_start = value_end;
      self->set_adjustment_value_with_blocked_handler(self->_w.adjustment_start, value_start, self->_w.on_adjustment_start_value_changed_id);
    }

  self->set_length_adjustment();
  self->apply_selection((int)gtk_adjustment_get_value(self->_w.adjustment_start), (int)gtk_adjustment_get_value(self->_w.adjustment_end));

  on_jump_to_end(self->_w.toggle_button_jump_to_end, self);
}

void
SaveMemoryWindow::on_adjustment_length_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;

  gdouble value_start = gtk_adjustment_get_value(self->_w.adjustment_start);
  gdouble value_length = gtk_adjustment_get_value(adjustment);

  gdouble value_end = value_start + value_length - 1;
  self->set_adjustment_value_with_blocked_handler(self->_w.adjustment_end, value_end, self->_w.on_adjustment_end_value_changed_id);
  self->apply_selection((int)gtk_adjustment_get_value(self->_w.adjustment_start), (int)gtk_adjustment_get_value(self->_w.adjustment_end));

  on_jump_to_end(self->_w.toggle_button_jump_to_end, self);
}

void
SaveMemoryWindow::set_length_adjustment(void)
{
  gdouble value_start = gtk_adjustment_get_value(_w.adjustment_start);
  gdouble value_end = gtk_adjustment_get_value(_w.adjustment_end);

  // adjust upper bound before setting the new length value to prevent constraint problems
  _w.adjustment_length->upper = _w.adjustment_start->upper + 1 - value_start;
  g_signal_emit_by_name(G_OBJECT(_w.adjustment_length), "changed");

  gdouble value_length = value_end - value_start + 1;
  set_adjustment_value_with_blocked_handler(_w.adjustment_length, value_length, _w.on_adjustment_length_value_changed_id);
}

void
SaveMemoryWindow::set_adjustment_value_with_blocked_handler(GtkAdjustment *adjustment, gdouble value, gint handler_id)
{
  g_signal_handler_block(adjustment, handler_id);
  gtk_adjustment_set_value(adjustment, value);
  g_signal_handler_unblock(adjustment, handler_id);
}

void
SaveMemoryWindow::on_populate_popup(GtkTextView *textview, GtkMenu *menu, gpointer user_data)
{
#if 0
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), separator);
    gtk_widget_show(separator);

    GtkWidget *bottom = gtk_image_menu_item_new_from_stock(GTK_STOCK_GOTO_BOTTOM, NULL);
    gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), bottom);
    gtk_widget_show(bottom);

    GtkWidget *top = gtk_image_menu_item_new_from_stock(GTK_STOCK_GOTO_TOP, NULL);
    gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), top);
    gtk_widget_show(top);
#endif
}

void
SaveMemoryWindow::jump_to(int line, gdouble within_margin, gboolean use_align, gdouble yalign)
{
  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_line(_w.text_buffer, &iter, line);
  gtk_text_view_scroll_to_iter(_w.text_view, &iter, within_margin, use_align, 0.0, yalign);
}

void
SaveMemoryWindow::on_jump_to_start(GtkToggleButton *button, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;

  if (gtk_toggle_button_get_active(button))
    {
      int start_addr = (int)gtk_adjustment_get_value(self->_w.adjustment_start);
      self->jump_to(start_addr / 16, 0.0, TRUE, 0.1);
    }
}

void
SaveMemoryWindow::on_jump_to_end(GtkToggleButton *button, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;

  if (gtk_toggle_button_get_active(button))
    {
      int end_addr = (int)gtk_adjustment_get_value(self->_w.adjustment_end);
      self->jump_to(end_addr / 16, 0.05, FALSE, 0.0);
    }
}

void
SaveMemoryWindow::on_refresh(GtkButton *button, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;
  self->refresh();
}

void
SaveMemoryWindow::on_save(GtkButton *button, gpointer user_data)
{
  SaveMemoryWindow *self = (SaveMemoryWindow *)user_data;

  GtkWidget *filechooser = gtk_file_chooser_dialog_new(_("Save As..."),
                                                       GTK_WINDOW(self->_window),
                                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                       GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                                                       NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(filechooser), TRUE);

  char buf[1024];
  int start_addr = (int)gtk_adjustment_get_value(self->_w.adjustment_start);
  snprintf(buf, sizeof(buf), _("memorydump_0x%04x.img"), start_addr);
  gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(filechooser), buf);

  char *filename = NULL;
  if (gtk_dialog_run(GTK_DIALOG(filechooser)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(filechooser));
    }

  gtk_widget_destroy (filechooser);

  if (filename == NULL)
    return;

  int end_addr = (int)gtk_adjustment_get_value(self->_w.adjustment_end);
  self->save(filename, start_addr, end_addr);
  self->hide();
  g_free(filename);
}

void
SaveMemoryWindow::save(const char *filename, int start_addr, int end_addr)
{
  CMD_Args *args = new CMD_Args();
  args->set_string_arg("filename", filename);
  args->set_long_arg("start-address", start_addr);
  args->set_long_arg("end-address", end_addr);
  CMD_EXEC_ARGS("kc-image-save", args);
  delete args;
}

void
SaveMemoryWindow::remove_selection(int start_line, int end_line)
{
  if (start_line == end_line)
    return;
    
  for (int a = start_line;a < end_line;a++)
    line_selected[a] = false;

  GtkTextIter iter, iter2;
  gtk_text_buffer_get_iter_at_line(_w.text_buffer, &iter, start_line);
  gtk_text_buffer_get_iter_at_line(_w.text_buffer, &iter2, end_line);
  gtk_text_buffer_remove_tag(_w.text_buffer, _w.text_tag_bold, &iter, &iter2);
}

void
SaveMemoryWindow::apply_line_selection(int line, int bytes, int line_offset, int line_offset2, int separator_offset, int chars_per_byte, bool add)
{
  if (bytes == 0)
    return;

  GtkTextIter iter;
  gtk_text_buffer_get_iter_at_line(_w.text_buffer, &iter, line);
  if (!gtk_text_iter_forward_chars(&iter, line_offset))
    return;

  int chars = chars_per_byte * bytes + line_offset2;
  if (bytes > 7)
    chars += separator_offset;

  GtkTextIter iter2 = iter;
  if (!gtk_text_iter_forward_chars(&iter2, chars))
    return;

  if (add)
    {
      gtk_text_buffer_apply_tag(_w.text_buffer, _w.text_tag_bold, &iter, &iter2);
    }
  else
    {
      line_selected[line] = false;
      gtk_text_buffer_remove_tag(_w.text_buffer, _w.text_tag_bold, &iter, &iter2);
    }
}

void
SaveMemoryWindow::apply_selection(int start_addr, int end_addr)
{
  int start_line = start_addr / 16;
  int end_line = end_addr / 16;

  // Remove bold tag from all lines before and after the selection
  // _including_ the line that contains the last byte but _not_ the
  // line that contains the first byte.
  remove_selection(0, start_line);
  remove_selection(end_line, NR_OF_LINES);

  GtkTextIter iter, iter2;
  for (int line = start_line;line < end_line;line++)
    {
      if (line_selected[line])
        continue;

      line_selected[line] = true;
      gtk_text_buffer_get_iter_at_line(_w.text_buffer, &iter, line);
      if (gtk_text_iter_forward_chars(&iter, 7))
        {
          gtk_text_buffer_get_iter_at_line(_w.text_buffer, &iter2, line + 1);
          gtk_text_buffer_apply_tag(_w.text_buffer, _w.text_tag_bold, &iter, &iter2);
        }
    }

    // order is important in case of start_line == end_line
    apply_line_selection(end_line, end_addr % 16 + 1, 7, 0, 2, 3, true);
    apply_line_selection(end_line, end_addr % 16 + 1, 56, 3, 3, 1, true);
    apply_line_selection(start_line, start_addr % 16, 7, 0, 2, 3, false);
    apply_line_selection(start_line, start_addr % 16, 56, 3, 3, 1, false);
}

void
SaveMemoryWindow::load_memory(int start_addr, int end_addr)
{
  char buf[20];

  GtkTextIter iter, iter2;
  for (int line_addr = 0;line_addr < 0x10000;line_addr += 16)
    {
      snprintf(buf, sizeof(buf), "%04xh: ", line_addr);
      gtk_text_buffer_get_end_iter(_w.text_buffer, &iter);
      gtk_text_buffer_insert_with_tags(_w.text_buffer, &iter, buf, -1, _w.text_tag_italic, NULL);
      
      string line;
      for (int idx = 0;idx < 16;idx++)
        {
            int addr = line_addr + idx;
            snprintf(buf, sizeof(buf), "%02x%s", memory->memRead8(addr), (idx == 7) ? " | " : " ");
            line += buf;
        }
      line += "  ";
      for (int idx = 0;idx < 16;idx++)
        {
            int addr = line_addr + idx;
            byte_t b = memory->memRead8(addr);
            int c = ((b >= 0x20) && (b < 0x7f)) ? b : '.';
            snprintf(buf, sizeof(buf), "%c%s", c, (idx == 7) ? " | " : "");
            line += buf;
        }
      line += "\n";
      
      gtk_text_buffer_get_end_iter(_w.text_buffer, &iter);
      gtk_text_buffer_insert(_w.text_buffer, &iter, line.c_str(), -1);
    }

    gtk_text_buffer_get_start_iter(_w.text_buffer, &iter);
    gtk_text_buffer_get_end_iter(_w.text_buffer, &iter2);
    gtk_text_buffer_apply_tag(_w.text_buffer, _w.text_tag_monospace, &iter, &iter2);
    
    apply_selection(start_addr, end_addr);
}

void SaveMemoryWindow::refresh(void)
{
  gtk_text_buffer_set_text(_w.text_buffer, "", 0);

  for (int a = 0;a < NR_OF_LINES;a++)
    line_selected[a] = false;

  if (is_visible())
    {
      int start_addr = (int)gtk_adjustment_get_value(_w.adjustment_start);
      int end_addr = (int)gtk_adjustment_get_value(_w.adjustment_end);
      load_memory(start_addr, end_addr);
    }
}

void
SaveMemoryWindow::show(void)
{
  UI_Gtk_Window::show();

  refresh();
}

void
SaveMemoryWindow::init(void)
{
  _window = get_widget("save_memory_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
                     GTK_SIGNAL_FUNC(cmd_exec_sft),
                     (char *)"ui-save-memory-window-toggle"); // FIXME:

  _w.button_save = GTK_BUTTON(get_widget("dialog_button_save"));
  g_signal_connect(_w.button_save, "clicked", G_CALLBACK(on_save), this);

  _w.button_refresh = GTK_BUTTON(get_widget("dialog_button_refresh"));
  g_signal_connect(_w.button_refresh, "clicked", G_CALLBACK(on_refresh), this);

  _w.toggle_button_jump_to_start = GTK_TOGGLE_BUTTON(get_widget("toggle_button_jump_to_start"));
  g_signal_connect(_w.toggle_button_jump_to_start, "toggled", G_CALLBACK(on_jump_to_start), this);

  _w.toggle_button_jump_to_end = GTK_TOGGLE_BUTTON(get_widget("toggle_button_jump_to_end"));
  g_signal_connect(_w.toggle_button_jump_to_end, "toggled", G_CALLBACK(on_jump_to_end), this);
    
  _w.spin_button_start_hex = GTK_SPIN_BUTTON(get_widget("spin_button_start_hex"));
  _w.spin_button_start_dec = GTK_SPIN_BUTTON(get_widget("spin_button_start_dec"));
  _w.spin_button_end_hex = GTK_SPIN_BUTTON(get_widget("spin_button_end_hex"));
  _w.spin_button_end_dec = GTK_SPIN_BUTTON(get_widget("spin_button_end_dec"));
  _w.spin_button_length_hex = GTK_SPIN_BUTTON(get_widget("spin_button_length_hex"));
  _w.spin_button_length_dec = GTK_SPIN_BUTTON(get_widget("spin_button_length_dec"));
      
  _w.adjustment_start = gtk_spin_button_get_adjustment(_w.spin_button_start_hex);
  _w.adjustment_end = gtk_spin_button_get_adjustment(_w.spin_button_end_hex);
  _w.adjustment_length = gtk_spin_button_get_adjustment(_w.spin_button_length_hex);
    
  gtk_spin_button_set_adjustment(_w.spin_button_start_dec, _w.adjustment_start);
  gtk_spin_button_set_adjustment(_w.spin_button_end_dec, _w.adjustment_end);
  gtk_spin_button_set_adjustment(_w.spin_button_length_dec, _w.adjustment_length);

  g_signal_connect(_w.spin_button_start_hex, "output", G_CALLBACK(on_output), NULL);
  g_signal_connect(_w.spin_button_end_hex, "output", G_CALLBACK(on_output), NULL);
  g_signal_connect(_w.spin_button_length_hex, "output", G_CALLBACK(on_output), NULL);
  
  g_signal_connect(_w.spin_button_start_hex, "input", G_CALLBACK(on_input), NULL);
  g_signal_connect(_w.spin_button_end_hex, "input", G_CALLBACK(on_input), NULL);
  g_signal_connect(_w.spin_button_length_hex, "input", G_CALLBACK(on_input), NULL);

  _w.on_adjustment_start_value_changed_id = g_signal_connect(_w.adjustment_start, "value-changed", G_CALLBACK(on_adjustment_start_value_changed), this);
  _w.on_adjustment_end_value_changed_id = g_signal_connect(_w.adjustment_end, "value-changed", G_CALLBACK(on_adjustment_end_value_changed), this);
  _w.on_adjustment_length_value_changed_id = g_signal_connect(_w.adjustment_length, "value-changed", G_CALLBACK(on_adjustment_length_value_changed), this);

  _w.text_view = GTK_TEXT_VIEW(get_widget("textview"));
  _w.text_buffer = gtk_text_view_get_buffer(_w.text_view);
  _w.text_tag_bold = gtk_text_buffer_create_tag(_w.text_buffer, "bold",
                                                "weight", PANGO_WEIGHT_BOLD,
                                                "background", "#D0D0D0",
                                                "background-set", TRUE,
                                                NULL);
  _w.text_tag_italic = gtk_text_buffer_create_tag(_w.text_buffer, "italic", "style", PANGO_STYLE_ITALIC, NULL);
  _w.text_tag_monospace = gtk_text_buffer_create_tag(_w.text_buffer, "monospace", "family", "Monospace", NULL);
  g_signal_connect(_w.text_view, "populate-popup", G_CALLBACK(on_populate_popup), this);

  init_dialog("ui-save-memory-window-toggle", "window-save-memory");
}
