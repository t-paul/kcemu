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

#ifndef __ui_gtk_debug_h
#define __ui_gtk_debug_h

#include <gtk/gtk.h>

#include "ui/debug.h"
#include "ui/gtk/window.h"

typedef enum {
  SCROLL_NONE,
  SCROLL_FORWARD,
  SCROLL_BACKWARD,
  SCROLL_NEXT_PAGE,
  SCROLL_PREV_PAGE
} scroll_dir_t;

class debug_op_t;

#define DEBUG_NR_OF_ASM_LABELS (12)
#define DEBUG_NR_OF_MEM_LABELS (8)
#define DEBUG_NR_OF_REG_LABELS (4)
#define DEBUG_ASM_NAME         _("Disassembler")
#define DEBUG_MEM_NAME         _("Memory")
#define DEBUG_REG_NAME         _("Registers")
#define DEBUG_ASM_HEADLINE     _("Disassembler (incomplete, use CTRL-d to activate)")
#define DEBUG_MEM_HEADLINE     _("Memory (use CTRL-m to activate)")
#define DEBUG_REG_HEADLINE     _("Registers")

class DebugWindow : public UI_Gtk_Window, public DebugInterface
{
 private:
  //
  // argh, the following declarations trigger a compiler error :-(
  // 'Internal compiler error 19970302.'
  //
  //static const int    DEBUG_NR_OF_ASM_LABELS;
  //static const int    DEBUG_NR_OF_MEM_LABELS;
  //static const int    DEBUG_NR_OF_REG_LABELS;
  //static       char * DEBUG_ASM;
  //static       char * DEBUG_MEM;
  //static       char * DEBUG_REG;
  
 private:
  struct
  {
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *active;
    GtkWidget *op;
    GtkWidget *evb_asm;
    GtkWidget *frame_asm;
    GtkWidget *vbox_asm;
    GtkWidget *evb_mem;
    GtkWidget *frame_mem;
    GtkWidget *vbox_mem;
    GtkWidget *evb_reg;
    GtkWidget *frame_reg;
    GtkWidget *vbox_reg;
    GtkWidget *l_asm[DEBUG_NR_OF_ASM_LABELS];
    GtkWidget *l_mem[DEBUG_NR_OF_MEM_LABELS];
    GtkWidget *l_reg[DEBUG_NR_OF_REG_LABELS];
    GtkWidget *trace;
    GtkWidget *single_step;
    GtkWidget *execute_step;
  } _w;

  int _pc;
  int _pc_np;
  int _mem;
  bool _minibuffer_active;

  CMD *_cmd1;
  CMD *_cmd2;
  PangoFontDescription *_font_desc;

 protected:
  void init(void);
  void do_goto_string(const char *str);

  static gint key_press_func(GtkWidget *widget, GdkEventKey *event,
                             gpointer data);
  debug_op_t *_op;

 public:
  DebugWindow(void);
  virtual ~DebugWindow(void);

  void do_goto_int(int addr);

  virtual void debugTrace(void) {}
  virtual void debugSingleStep(void) {}
  
  friend class debug_op_t;
  friend class debug_op_asm;
  friend class debug_op_mem;
  friend class debug_op_reg;
};

/*
 *  this generates an operator new() that makes the class
 *  a singleton; that means every new call for this class returns
 *  the _same_ object!
 */
#define __NEW_SINGLETON(classname) \
  void * operator new(size_t size) { \
    static classname *ptr = 0; \
    if (!ptr) ptr = ::new classname(); \
    return ptr; \
  }

#define __CAPABILITIES(can_goto) \
  virtual bool can_go_to(void) { return can_goto; }

class debug_op_t {
 public:
  debug_op_t(void) {};
  virtual ~debug_op_t(void) {};
  void activate(DebugWindow *w);
  virtual void go_to(DebugWindow *w, int addr) {}
  virtual void update(DebugWindow *w, scroll_dir_t d = SCROLL_NONE) {}
  virtual const char * get_name(void) = 0;
  virtual const char * get_headline(void) = 0;
  virtual bool can_go_to(void) = 0;
};

class debug_op_asm : public debug_op_t {
 public:
  debug_op_asm(void) {};
  virtual ~debug_op_asm(void) {};
  virtual void go_to(DebugWindow *w, int addr);
  virtual void update(DebugWindow *w, scroll_dir_t d = SCROLL_NONE);
  virtual const char * get_name(void) {
    return DEBUG_ASM_NAME;
  }
  virtual const char * get_headline(void) {
    return DEBUG_ASM_HEADLINE;
  }
  __NEW_SINGLETON(debug_op_asm);
  __CAPABILITIES(true);
};

class debug_op_mem : public debug_op_t {
 public:
  debug_op_mem(void) {};
  virtual ~debug_op_mem(void) {};
  virtual void go_to(DebugWindow *w, int addr);
  virtual void update(DebugWindow *w, scroll_dir_t d = SCROLL_NONE);
  virtual const char * get_name(void) {
    return DEBUG_MEM_NAME;
  }
  virtual const char * get_headline(void) {
    return DEBUG_MEM_HEADLINE;
  }
  __NEW_SINGLETON(debug_op_mem);
  __CAPABILITIES(true);
};

class debug_op_reg : public debug_op_t {
 public:
  debug_op_reg(void) {};
  virtual ~debug_op_reg(void) {};
  //virtual void go_to(DebugWindow *w, int addr);
  virtual void update(DebugWindow *w, scroll_dir_t d = SCROLL_NONE);
  virtual const char * get_name(void) {
    return DEBUG_REG_NAME;
  }
  virtual const char * get_headline(void) {
    return DEBUG_REG_HEADLINE;
  }
  __NEW_SINGLETON(debug_op_reg);
  __CAPABILITIES(false);
};

#undef __NEW_SINGLETON
#undef __CAPABILITIES

#endif /* __ui_gtk_debug_h */
