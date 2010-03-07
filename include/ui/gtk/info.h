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

#ifndef __ui_gtk_info_h
#define __ui_gtk_info_h

#include <gtk/gtk.h>

#include "ui/gtk/window.h"

class InfoWindow : public UI_Gtk_Window
{
 private:
  typedef enum {
    A, B, C, D, E, F, H, L,
    IX, IY, BC, DE, HL, SP, PC,
    NR_OF_REGS
  } _regs;
  
  struct
  {
    GtkWidget *vbox;
    GtkWidget *notebook;
    GtkWidget *b;
    GtkWidget *separator;
    GtkWidget *bbox;
    GtkWidget *b_update;
    GtkWidget *b_close;
  } _w;
  struct
  {
    GtkWidget *table;
    GtkWidget *reg_f[NR_OF_REGS];
    GtkWidget *reg_v[NR_OF_REGS];   /* vbox */
    GtkWidget *reg_led[NR_OF_REGS]; /* label pointer */
    GtkWidget *reg_lp[NR_OF_REGS];  /* label pointer */
    GtkWidget *reg_h[NR_OF_REGS];   /* hbox */
    GtkWidget *reg_lh[NR_OF_REGS];  /* label hex */
    GtkWidget *reg_ld[NR_OF_REGS];  /* label dec */
  } _cpu;
  struct
  {
    GtkWidget *hbox;
    GtkWidget *frame[2];
    GtkWidget *table[2];
    GtkWidget *mode_l[2];
    GtkWidget *mode_v[2];
    GtkWidget *irq_l[2];
    GtkWidget *irq_v[2];
    GtkWidget *irqvec_l[2];
    GtkWidget *irqvec_v[2];
  } _pio;
  struct
  {
    GtkWidget *hbox;
    GtkWidget *frame[4];
    GtkWidget *table[4];    
    GtkWidget *irq_l[4];
    GtkWidget *irq_v[4];
    GtkWidget *irqvec_l[4];
    GtkWidget *irqvec_v[4];
  } _ctc;

  CMD *_cmd;
  
 protected:
  void init(void);
  GtkWidget * create_page_cpu();
  GtkWidget * create_page_pio();
  GtkWidget * create_page_ctc();
  GtkWidget * create_channel_pio(int channel, const char *frame_title);
  GtkWidget * create_channel_ctc(int channel, const char *frame_title);
  GtkWidget * create_reg(int reg, const char *frame_title, int led);
  static void update(GtkWidget *w, InfoWindow *self);
  static void switch_page(GtkWidget *w, GtkNotebookPage *page,
                          gint page_num, InfoWindow *self);
  void do_update(int page);
  void do_update_cpu(void);
  void do_update_pio(void);
  void do_update_ctc(void);
  
 public:
  InfoWindow(void);
  virtual ~InfoWindow(void);
};

#endif /* __ui_gtk_info_h */
