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
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/memory.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/debug.h"

using namespace std;

extern "C" int disass(int, char **); /* FIXME */

class CMD_debug_window_toggle : public CMD
{
private:
  DebugWindow *_w;

public:
  CMD_debug_window_toggle(DebugWindow *w) : CMD("ui-debug-window-toggle")
    {
      _w = w;
      register_cmd("ui-debug-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

class CMD_single_step_executed : public CMD
{
private:
  DebugWindow *_w;

public:
  CMD_single_step_executed(DebugWindow *w) : CMD("single-step-executed")
    {
      _w = w;
      register_cmd("single-step-executed");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->do_goto_int(z80->getPC());
    }
};

void
DebugWindow::do_goto_int(int addr)
{
  _op->go_to(this, addr);
  _op->update(this);
  (new debug_op_reg())->update(this);
}

void
DebugWindow::do_goto_string(const char *str)
{
  long val;
  char *endptr;

  if (str != NULL)
    {
      val = strtol(str, &endptr, 16);
      if ((*str != '\0') && (*endptr == '\0'))
	if ((val >= 0) && (val <= 0xffff))
	  {
	    do_goto_int(val);
	    return;
	  }
    }

  gdk_beep();
  gtk_entry_select_region(GTK_ENTRY(_w.op), 0, -1);
}

gint
DebugWindow::key_press_func(GtkWidget *widget, GdkEventKey *event,
			    gpointer data)
{
  DebugWindow *self;

  int clear_op = 0;
  self = (DebugWindow *)data;
  if (event->state & GDK_CONTROL_MASK)
    {
      switch (event->keyval)
	{
	case 'd':
	  delete self->_op;
	  self->_op = new debug_op_asm();
	  self->_op->activate(self);
	  break;
	case 'm':
	  delete self->_op;
	  self->_op = new debug_op_mem();
	  self->_op->activate(self);
	  break;
	case 'r':
	  delete self->_op;
	  self->_op = new debug_op_reg();
	  self->_op->activate(self);
	  break;
	case 'g':
	  /* gtk_label_set_text(GTK_LABEL(self->_w.op), "go to: "); */
	  if (self->_op->can_go_to())
	    {
	      // gtk_widget_show(self->_w.op);
	      self->_minibuffer_active = true;
	      gtk_widget_set_sensitive(self->_w.op, true);
	      gtk_entry_select_region(GTK_ENTRY(self->_w.op), 0, -1);
	      gtk_widget_grab_focus(self->_w.op);
	    }
	  break;
	case 'p':
	case 'b':
	case GDK_Up:
	case GDK_KP_Up:
	  self->_op->update(self, SCROLL_PREV_PAGE);
	  break;
	case 'f':
	case 'n':
	case GDK_Down:
	case GDK_KP_Down:
	  self->_op->update(self, SCROLL_NEXT_PAGE);
	  break;
	}
    }
  else
    {
      switch (event->keyval)
	{
	case GDK_Return:
	case GDK_KP_Enter:
	  if (!GTK_WIDGET_VISIBLE(self->_w.op)) break;
	  self->do_goto_string(gtk_entry_get_text(GTK_ENTRY(self->_w.op)));
	  break;
	case GDK_Escape:
	  clear_op = 1;
	  break;
	case GDK_BackSpace:
	  break;
	case GDK_Up:
	case GDK_KP_Up:
	  self->_op->update(self, SCROLL_BACKWARD);
	  break;
	case GDK_Down:
	case GDK_KP_Down:
	  self->_op->update(self, SCROLL_FORWARD);
	  break;
	case GDK_Page_Up:
	case GDK_KP_Page_Up:
	  self->_op->update(self, SCROLL_PREV_PAGE);
	  break;
	case GDK_Page_Down:
	case GDK_KP_Page_Down:
	  self->_op->update(self, SCROLL_NEXT_PAGE);
	  break;
	}
    }

  if (clear_op)
    {
      /* gtk_label_set_text(GTK_LABEL(self->_w.op), ""); */
      // gtk_widget_hide(self->_w.op);
      gtk_widget_set_sensitive(self->_w.op, false);
      self->_op->update(self);
    }

  if (!self->_minibuffer_active)
    gtk_signal_emit_stop_by_name(GTK_OBJECT(widget), "key_press_event");

  return FALSE;
}

DebugWindow::DebugWindow(void)
{
  _pc = 0xe000;
  _mem = 0xe000;

  _minibuffer_active = false;

  _font_desc = pango_font_description_new();
  pango_font_description_set_family(_font_desc, "Courier");
  pango_font_description_set_style(_font_desc, PANGO_STYLE_NORMAL);
  pango_font_description_set_variant(_font_desc, PANGO_VARIANT_NORMAL);
  pango_font_description_set_weight(_font_desc, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_stretch(_font_desc, PANGO_STRETCH_NORMAL);
  pango_font_description_set_size(_font_desc, 8 * PANGO_SCALE);

  _cmd1 = new CMD_debug_window_toggle(this);
  _cmd2 = new CMD_single_step_executed(this);
}

DebugWindow::~DebugWindow(void)
{
  delete _cmd1;
  delete _cmd2;
  pango_font_description_free(_font_desc);
}

void
DebugWindow::init(void)
{
  int a;
  GdkCursor *cursor;

  /*
   *  window
   */
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_window, "DebugWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Debugger"));
  gtk_widget_set_uposition(_window, 650, 50);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-debug-window-toggle"); // FIXME:
  gtk_signal_connect(GTK_OBJECT(_window), "key_press_event",
		     GTK_SIGNAL_FUNC(key_press_func), this);

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  hbox
   */
  _w.hbox = gtk_hbox_new(TRUE, 0);
  gtk_container_add(GTK_CONTAINER(_w.vbox), _w.hbox);
  gtk_widget_show(_w.hbox);

  /*
   *  active/ op label
   */
  _w.active = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_w.active), 0, 0.5);
  gtk_box_pack_start(GTK_BOX(_w.hbox), _w.active, FALSE, TRUE, 0);
  gtk_widget_show(_w.active);

  /*
   *  asm eventbox
   */
  _w.evb_asm = gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.evb_asm, FALSE, TRUE, 0);
  gtk_widget_show(_w.evb_asm);
  gtk_object_set_user_data(GTK_OBJECT(_w.evb_asm), this);
  gtk_widget_realize(_w.evb_asm);
  cursor = gdk_cursor_new(GDK_HAND1);
  gdk_window_set_cursor(_w.evb_asm->window, cursor);
  gdk_cursor_destroy(cursor);
  gtk_widget_show(_w.evb_asm);

  /*
   *  asm frame/ vbox
   */
  _w.frame_asm = gtk_frame_new(DEBUG_ASM_HEADLINE);
  gtk_container_add(GTK_CONTAINER(_w.evb_asm), _w.frame_asm);
  gtk_widget_show(_w.frame_asm);
  _w.vbox_asm = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_w.frame_asm), _w.vbox_asm);
  gtk_widget_show(_w.vbox_asm);

  /*
   *  mem eventbox
   */
  _w.evb_mem = gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.evb_mem, FALSE, TRUE, 0);
  gtk_widget_show(_w.evb_mem);
  gtk_object_set_user_data(GTK_OBJECT(_w.evb_mem), this);
  gtk_widget_realize(_w.evb_mem);
  cursor = gdk_cursor_new(GDK_CROSSHAIR);
  gdk_window_set_cursor(_w.evb_mem->window, cursor);
  gdk_cursor_destroy(cursor);
  gtk_widget_show(_w.evb_mem);

  /*
   *  mem frame/ vbox
   */
  _w.frame_mem = gtk_frame_new(DEBUG_MEM_HEADLINE);
  gtk_container_add(GTK_CONTAINER(_w.evb_mem), _w.frame_mem);
  gtk_widget_show(_w.frame_mem);
  _w.vbox_mem = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_w.frame_mem), _w.vbox_mem);
  gtk_widget_show(_w.vbox_mem);

  /*
   *  reg eventbox
   */
  _w.evb_reg = gtk_event_box_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.evb_reg, FALSE, TRUE, 0);
  gtk_widget_show(_w.evb_reg);
  gtk_object_set_user_data(GTK_OBJECT(_w.evb_reg), this);
  gtk_widget_realize(_w.evb_reg);
  cursor = gdk_cursor_new(GDK_XTERM);
  gdk_window_set_cursor(_w.evb_reg->window, cursor);
  gdk_cursor_destroy(cursor);
  gtk_widget_show(_w.evb_reg);

  /*
   *  reg frame/ vbox
   */
  _w.frame_reg = gtk_frame_new(DEBUG_REG_HEADLINE);
  gtk_container_add(GTK_CONTAINER(_w.evb_reg), _w.frame_reg);
  gtk_widget_show(_w.frame_reg);
  _w.vbox_reg = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_w.frame_reg), _w.vbox_reg);
  gtk_widget_show(_w.vbox_reg);

  _w.trace = gtk_toggle_button_new_with_label(_("Trace"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.trace, FALSE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(_w.trace), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (char *)"z80-trace-toggle"); // FIXME:
  gtk_widget_show(_w.trace);

  _w.single_step = gtk_toggle_button_new_with_label(_("Single Step"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.single_step, FALSE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(_w.single_step), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (char *)"z80-single-step-toggle"); // FIXME:
  gtk_widget_show(_w.single_step);

  _w.execute_step = gtk_button_new_with_label(_("Execute Step"));
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.execute_step, FALSE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(_w.execute_step), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (char *)"z80-execute-step"); // FIXME:
  gtk_widget_show(_w.execute_step);

  /*
    _w.op = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(_w.op), 0, 0.5);
  */
  _w.op = gtk_entry_new();
  gtk_widget_set_sensitive(_w.op, false);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.op, FALSE, TRUE, 0);
  gtk_widget_show(_w.op);

  /*
   *---------------------------------------------------------------------------
   */

  /*
   *  labels
   */
  for (a = 0;a < DEBUG_NR_OF_ASM_LABELS;a++)
    {
      _w.l_asm[a] = gtk_label_new("");
      gtk_widget_modify_font(_w.l_asm[a], _font_desc);
      gtk_misc_set_alignment(GTK_MISC(_w.l_asm[a]), 0, 0.5);
      gtk_box_pack_start(GTK_BOX(_w.vbox_asm), _w.l_asm[a], FALSE, TRUE, 0);
      gtk_widget_show(_w.l_asm[a]);
    }
  for (a = 0;a < DEBUG_NR_OF_MEM_LABELS;a++)
    {
      _w.l_mem[a] = gtk_label_new("");
      gtk_widget_modify_font(_w.l_mem[a], _font_desc);
      gtk_misc_set_alignment(GTK_MISC(_w.l_mem[a]), 0, 0.5);
      gtk_box_pack_start(GTK_BOX(_w.vbox_mem), _w.l_mem[a], FALSE, TRUE, 0);
      gtk_widget_show(_w.l_mem[a]);
    }
  for (a = 0;a < DEBUG_NR_OF_REG_LABELS;a++)
    {
      _w.l_reg[a] = gtk_label_new("");
      gtk_widget_modify_font(_w.l_reg[a], _font_desc);
      gtk_misc_set_alignment(GTK_MISC(_w.l_reg[a]), 0, 0.5);
      gtk_box_pack_start(GTK_BOX(_w.vbox_reg), _w.l_reg[a], FALSE, TRUE, 0);
      gtk_widget_show(_w.l_reg[a]);
    }

  /*
   *  initial display
   */
  _op = new debug_op_reg();
  _op->update(this);
  delete _op;

  _op = new debug_op_mem();
  _op->update(this);
  delete _op;

  _op = new debug_op_asm();
  _op->update(this);
  _op->activate(this);
}

void
debug_op_t::activate(DebugWindow *w)
{
  char buf[256];

  sprintf(buf, "Active Window: %s", get_name());
  gtk_label_set_text(GTK_LABEL(w->_w.active), buf);
}

void
debug_op_asm::go_to(DebugWindow *w, int addr)
{
  w->_pc = addr;
}

void
debug_op_asm::update(DebugWindow *w, scroll_dir_t direction)
{
  int a;
  int done;
  char *ptr = NULL;
  int pc, newpc;

  if (direction == SCROLL_NEXT_PAGE)
    {
      direction = SCROLL_NONE;
      w->_pc = w->_pc_np;
    }

  newpc = w->_pc - 20;

  a = 0;
  done = 0;
  do
    {
      pc = newpc;
      newpc = 0; // FIXME: call disassembler here
      free(ptr);
      switch (direction)
	{
	case SCROLL_NONE:
	  done = 1;
	  break;
	case SCROLL_FORWARD:
	  if ((pc <= w->_pc) && (newpc > w->_pc))
	    {
	      w->_pc = newpc;
	      done = 1;
	    }
	  break;
	case SCROLL_BACKWARD:
	  if ((pc < w->_pc) && (newpc >= w->_pc))
	    {
	      w->_pc = pc;
	      done = 1;
	    }
	  break;
	default:
	  break;
	}
      if (++a > 22)
	{
	  cerr << "ARGH! (" __FILE__ ":" << __LINE__ << ")" << endl;
	  return;
	}
    }
  while (!done);

  pc = w->_pc;
  for (a = 0;a < DEBUG_NR_OF_ASM_LABELS;a++)
    {
      if (a == (DEBUG_NR_OF_ASM_LABELS - 2))
	w->_pc_np = pc;
      pc = 0; // FIXME: call disassembler here
      gtk_label_set_text(GTK_LABEL(w->_w.l_asm[a]), ptr);
      free(ptr);
    }

  gtk_widget_queue_draw(w->_w.vbox_asm);
}

void
debug_op_mem::go_to(DebugWindow *w, int addr)
{
  w->_mem = addr;
}

void
debug_op_mem::update(DebugWindow *w, scroll_dir_t direction)
{
  int c;
  int a, b, mem;
  char mem_str[100];

  switch (direction)
    {
    case SCROLL_NONE:
      break;
    case SCROLL_FORWARD:
      w->_mem += 16;
      break;
    case SCROLL_BACKWARD:
      w->_mem -= 16;
      break;
    case SCROLL_NEXT_PAGE:
      w->_mem += 16 * DEBUG_NR_OF_MEM_LABELS;
      break;
    case SCROLL_PREV_PAGE:
      w->_mem -= 16 * DEBUG_NR_OF_MEM_LABELS;
      break;
    }

  mem = w->_mem;
  for (a = 0;a < DEBUG_NR_OF_MEM_LABELS;a++)
    {
      sprintf(mem_str, "%04xh:", mem);
      for (b = 0;b < 16;b++)
	{
	  sprintf(strchr(mem_str, 0), " %02x", memory->memRead8(mem + b));
	  if (b == 7) sprintf(strchr(mem_str, 0), " -");
	}
      sprintf(strchr(mem_str, 0), " | ");
      for (b = 0;b < 16;b++)
	{
	  c = memory->memRead8(mem + b);
	  sprintf(strchr(mem_str, 0), "%c", ((c >= 0x20) && (c < 0x80)) ? c : '.');
	  if (b == 7) sprintf(strchr(mem_str, 0), "-");
	}
      mem += 16;
      gtk_label_set_text(GTK_LABEL(w->_w.l_mem[a]), mem_str);
    }
}

void
debug_op_reg::update(DebugWindow *w, scroll_dir_t direction)
{
  int a;
  char buf[256];

  a = 0;
  sprintf(buf, "PC  = %04xh  SP  = %04xh",
	  z80->getPC(),
	  z80->getSP());
  gtk_label_set_text(GTK_LABEL(w->_w.l_reg[a++]), buf);
  sprintf(buf, "AF  = %04xh  BC  = %04xh  DE  = %04xh  HL  = %04xh",
	  z80->getAF(),
	  z80->getBC(),
	  z80->getDE(),
	  z80->getHL());
  gtk_label_set_text(GTK_LABEL(w->_w.l_reg[a++]), buf);
  sprintf(buf, "AF' = %04xh  BC' = %04xh  DE' = %04xh  HL' = %04xh",
	  z80->getAFs(),
	  z80->getBCs(),
	  z80->getDEs(),
	  z80->getHLs());
  gtk_label_set_text(GTK_LABEL(w->_w.l_reg[a++]), buf);
  sprintf(buf, "IX  = %04xh  IY  = %04xh  IFF = %02xh    I   = %02xh",
	  z80->getIX(),
	  z80->getIY(),
	  z80->getIFF(),
	  z80->getI());
  gtk_label_set_text(GTK_LABEL(w->_w.l_reg[a++]), buf);
  gtk_widget_queue_draw(w->_w.vbox_reg);
}
