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

#include "kc/system.h"

#include "kc/z80.h"
#include "kc/pio.h"
#include "kc/ctc.h"
#include "kc/memory.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/info.h"

#include "libgtkex/libgtkex.h"

class CMD_info_window_toggle : public CMD
{
private:
  InfoWindow *_w;

public:
  CMD_info_window_toggle(InfoWindow *w) : CMD("ui-info-window-toggle")
    {
      _w = w;
      register_cmd("ui-info-window-toggle");
    }

  void execute(CMD_Args *args, CMD_Context context)
    {
      _w->toggle();
    }
};

void
InfoWindow::update(GtkWidget *w, InfoWindow *self)
{
  int p;

  p = gtk_notebook_current_page(GTK_NOTEBOOK(self->_w.notebook));
  self->do_update(p);
}

void
InfoWindow::switch_page(GtkWidget *w, GtkNotebookPage *page,
			gint page_num, InfoWindow *self)
{
  self->do_update(page_num);
}

void
InfoWindow::do_update_cpu(void)
{
  word_t b, m;
  char buf[20];
  unsigned int a;
  word_t af, bc, de, hl, ix, iy, sp, pc;
  struct {
    int reg; const char *fmt, *name; word_t *val; int shift, mask;
  } r[] = {
    { A,  "%02xh", "A",  &af, 8, 0xff },
    { F,  "%02xh", "F",  &af, 0, 0xff },
    { B,  "%02xh", "B",  &bc, 8, 0xff },
    { C,  "%02xh", "C",  &bc, 0, 0xff },
    { D,  "%02xh", "D",  &de, 8, 0xff },
    { E,  "%02xh", "E",  &de, 0, 0xff },
    { H,  "%02xh", "H",  &hl, 8, 0xff },
    { L,  "%02xh", "L",  &hl, 0, 0xff },
    { IX, "%04xh", "IX", &ix, 0, 0xffff },
    { IY, "%04xh", "IY", &iy, 0, 0xffff },
    { SP, "%04xh", "SP", &sp, 0, 0xffff },
    { BC, "%04xh", "BC", &bc, 0, 0xffff },
    { DE, "%04xh", "DE", &de, 0, 0xffff },
    { HL, "%04xh", "HL", &hl, 0, 0xffff },
    { PC, "%04xh", "PC", &pc, 0, 0xffff },
  };


  af = z80->getAF();
  bc = z80->getBC();
  de = z80->getDE();
  hl = z80->getHL();
  ix = z80->getIX();
  iy = z80->getIY();
  sp = z80->getSP();
  pc = z80->getPC();

  for (a = 0;a < (sizeof(r) / sizeof(r[0]));a++)
    {
      b = (*(r[a].val) >> r[a].shift) & r[a].mask;
      sprintf(buf, r[a].fmt, b);
      gtk_label_set(GTK_LABEL(_cpu.reg_lh[r[a].reg]), buf);
      sprintf(buf, "%d", b);
      gtk_label_set(GTK_LABEL(_cpu.reg_ld[r[a].reg]), buf);
      if (r[a].mask == 0xffff)
	{
	  m = ((memory->memRead8(b)) |
	       (memory->memRead8(b + 1) << 8));
	  sprintf(buf, "(%s) = %04xh", r[a].name, m);
	  gtk_label_set(GTK_LABEL(_cpu.reg_lp[r[a].reg]), buf);
	}
      else
	{
	  //gtk_led_line_set_value(GTK_LED_LINE(_cpu.reg_led[r[a].reg]), b);
	}
    }
}

void
InfoWindow::do_update_pio(void)
{
  byte_t a;
  char buf[10];
  const char *mode[] = {
    _("byte output"),
    _("byte input"),
    _("byte input/output"),
    _("bit input/output")
  };

  /*
   *  channel a
   */
  gtk_label_set(GTK_LABEL(_pio.mode_v[A]), mode[pio->getModeA()]);
  gtk_label_set(GTK_LABEL(_pio.irq_v[A]), (pio->getIRQA() ?
					   "enabled" : "disabled"));
  a = pio->getIRQVectorA();
  sprintf(buf, "%02xh (%d)", a, a);
  gtk_label_set(GTK_LABEL(_pio.irqvec_v[A]), buf);

  /*
   *  channel b
   */
  gtk_label_set(GTK_LABEL(_pio.mode_v[B]), mode[pio->getModeB()]);
  gtk_label_set(GTK_LABEL(_pio.irq_v[B]), (pio->getIRQB() ?
					   "enabled" : "disabled"));
  a = pio->getIRQVectorB();
  sprintf(buf, "%02xh (%d)", a, a);
  gtk_label_set(GTK_LABEL(_pio.irqvec_v[B]), buf);
}

void
InfoWindow::do_update_ctc(void)
{
  int a, c;
  char buf[20];

  for (c = 0;c < 4;c++)
    {
      gtk_label_set(GTK_LABEL(_ctc.irq_v[c]), (ctc->getIRQ(c) ?
					       "enabled" :
					       "disabled"));
      a = ctc->getIRQVector(c);
      sprintf(buf, "%02xh (%d)", a, a);
      gtk_label_set(GTK_LABEL(_ctc.irqvec_v[c]), buf);
    }
}

void
InfoWindow::do_update(int page)
{
  switch (page)
    {
    case 0:
      do_update_cpu();
      break;
    case 1:
      do_update_pio();
      break;
    case 2:
      do_update_ctc();
      break;
    }
}

InfoWindow::InfoWindow(void)
{
  _cmd = new CMD_info_window_toggle(this);
}

InfoWindow::~InfoWindow(void)
{
  delete _cmd;
}

GtkWidget *
InfoWindow::create_reg(int reg, const char *frame_title, int led)
{
  _cpu.reg_f[reg] = gtk_frame_new(frame_title);

  /*
   *  vbox
   */
  _cpu.reg_v[reg] = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_cpu.reg_f[reg]), _cpu.reg_v[reg]);
  gtk_widget_show(_cpu.reg_v[reg]);

  if (led)
    {
      /*
       *  led line
       */
      _cpu.reg_lp[reg] = NULL;
      _cpu.reg_led[reg] = gtk_led_line_new(8);
      gtk_box_pack_start(GTK_BOX(_cpu.reg_v[reg]), _cpu.reg_led[reg],
			 TRUE, TRUE, 0);
      gtk_widget_show(_cpu.reg_led[reg]);
    }
  else
    {
      /*
       *  label
       */
      _cpu.reg_led[reg] = NULL;
      _cpu.reg_lp[reg] = gtk_label_new("");
      gtk_misc_set_alignment(GTK_MISC(_cpu.reg_lp[reg]), 1, 0.5);
      gtk_misc_set_padding(GTK_MISC(_cpu.reg_lp[reg]), 4, 2);
      gtk_box_pack_start(GTK_BOX(_cpu.reg_v[reg]), _cpu.reg_lp[reg],
			 TRUE, TRUE, 0);
      gtk_widget_show(_cpu.reg_lp[reg]);
    }

  /*
   *  hbox
   */
  _cpu.reg_h[reg] = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_cpu.reg_v[reg]), _cpu.reg_h[reg]);
  gtk_widget_show(_cpu.reg_h[reg]);

  _cpu.reg_lh[reg] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_cpu.reg_lh[reg]), 0, 0.5);
  gtk_misc_set_padding(GTK_MISC(_cpu.reg_lh[reg]), 4, 2);
  gtk_box_pack_start(GTK_BOX(_cpu.reg_h[reg]), _cpu.reg_lh[reg],
		     TRUE, TRUE, 0);
  gtk_widget_show(_cpu.reg_lh[reg]);

  _cpu.reg_ld[reg] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_cpu.reg_ld[reg]), 1, 0.5);
  gtk_misc_set_padding(GTK_MISC(_cpu.reg_ld[reg]), 4, 2);
  gtk_box_pack_start(GTK_BOX(_cpu.reg_h[reg]), _cpu.reg_ld[reg],
		     TRUE, TRUE, 0);
  gtk_widget_show(_cpu.reg_ld[reg]);

  return _cpu.reg_f[reg];
}

GtkWidget *
InfoWindow::create_page_cpu(void)
{
  GtkWidget *w;
  unsigned int a;
  struct {
    int reg; const char *name; int r1, c1, r2, c2, led;
  } r[] = {
    {  A,  "A", 0, 1, 0, 1,  TRUE },
    {  F,  "F", 1, 2, 0, 1,  TRUE },
    { PC, "PC", 2, 3, 0, 1, FALSE },
    {  B,  "B", 0, 1, 1, 2,  TRUE },
    {  C,  "C", 1, 2, 1, 2,  TRUE },
    { BC, "BC", 2, 3, 1, 2, FALSE },
    {  D,  "D", 0, 1, 2, 3,  TRUE },
    {  E,  "E", 1, 2, 2, 3,  TRUE },
    { DE, "DE", 2, 3, 2, 3, FALSE },
    {  H,  "H", 0, 1, 3, 4,  TRUE },
    {  L,  "L", 1, 2, 3, 4,  TRUE },
    { HL, "HL", 2, 3, 3, 4, FALSE },
    { IX, "IX", 0, 1, 4, 5, FALSE },
    { IY, "IY", 1, 2, 4, 5, FALSE },
    { SP, "SP", 2, 3, 4, 5, FALSE },
  };

  _cpu.table = gtk_table_new(5, 3, FALSE);
  gtk_container_border_width(GTK_CONTAINER(_cpu.table), 4);
  gtk_table_set_row_spacings(GTK_TABLE(_cpu.table), 2);
  gtk_table_set_col_spacings(GTK_TABLE(_cpu.table), 4);
  gtk_widget_show(_cpu.table);

  for (a = 0;a < (sizeof(r) / sizeof(r[0]));a++)
    {
      w = create_reg(r[a].reg, r[a].name, r[a].led);
      gtk_table_attach(GTK_TABLE(_cpu.table), w,
		       r[a].r1, r[a].c1, r[a].r2, r[a].c2,
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		       (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		       0, 0);
      gtk_widget_show(w);
    }

  return _cpu.table;
}

GtkWidget *
InfoWindow::create_channel_pio(int c, const char *frame_title)
{
  /*
   *  channel frame
   */
  _pio.frame[c] = gtk_frame_new(frame_title);
  _pio.table[c] = gtk_table_new(1, 2, FALSE);
  gtk_container_border_width(GTK_CONTAINER(_pio.table[c]), 4);
  gtk_table_set_row_spacings(GTK_TABLE(_pio.table[c]), 2);
  gtk_table_set_col_spacings(GTK_TABLE(_pio.table[c]), 4);
  gtk_container_add(GTK_CONTAINER(_pio.frame[c]), _pio.table[c]);
  gtk_widget_show(_pio.table[c]);

  /*
   *  mode
   */
  _pio.mode_l[c] = gtk_label_new(_("Mode:"));
  gtk_misc_set_alignment(GTK_MISC(_pio.mode_l[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_pio.table[c]), _pio.mode_l[c],
		   0, 1, 0, 1,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_pio.mode_l[c]);

  _pio.mode_v[c] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_pio.mode_v[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_pio.table[c]), _pio.mode_v[c],
		   1, 2, 0, 1,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_pio.mode_v[c]);

  /*
   *  irq
   */
  _pio.irq_l[c] = gtk_label_new(_("IRQ:"));
  gtk_misc_set_alignment(GTK_MISC(_pio.irq_l[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_pio.table[c]), _pio.irq_l[c],
		   0, 1, 1, 2,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_pio.irq_l[c]);

  _pio.irq_v[c] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_pio.irq_v[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_pio.table[c]), _pio.irq_v[c],
		   1, 2, 1, 2,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_pio.irq_v[c]);

  /*
   *  irq vector
   */
  _pio.irqvec_l[c] = gtk_label_new(_("IRQ Vector:"));
  gtk_misc_set_alignment(GTK_MISC(_pio.irqvec_l[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_pio.table[c]), _pio.irqvec_l[c],
		   0, 1, 2, 3,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_pio.irqvec_l[c]);

  _pio.irqvec_v[c] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_pio.irqvec_v[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_pio.table[c]), _pio.irqvec_v[c],
		   1, 2, 2, 3,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_pio.irqvec_v[c]);

  return _pio.frame[c];
}

GtkWidget *
InfoWindow::create_page_pio()
{
  GtkWidget *w;

  _pio.hbox = gtk_hbox_new(TRUE, 0);
  gtk_widget_show(_pio.hbox);

  w = create_channel_pio(A, _("Channel A"));
  gtk_box_pack_start(GTK_BOX(_pio.hbox), w, TRUE, TRUE, 0);
  gtk_widget_show(w);

  w = create_channel_pio(B, _("Channel B"));
  gtk_box_pack_start(GTK_BOX(_pio.hbox), w, TRUE, TRUE, 0);
  gtk_widget_show(w);

  return _pio.hbox;
}

GtkWidget *
InfoWindow::create_channel_ctc(int c, const char *frame_title)
{
  /*
   *  channel frame
   */
  _ctc.frame[c] = gtk_frame_new(frame_title);
  _ctc.table[c] = gtk_table_new(1, 2, FALSE);
  gtk_container_border_width(GTK_CONTAINER(_ctc.table[c]), 4);
  gtk_table_set_row_spacings(GTK_TABLE(_ctc.table[c]), 2);
  gtk_table_set_col_spacings(GTK_TABLE(_ctc.table[c]), 4);
  gtk_container_add(GTK_CONTAINER(_ctc.frame[c]), _ctc.table[c]);
  gtk_widget_show(_ctc.table[c]);

  /*
   *  irq
   */
  _ctc.irq_l[c] = gtk_label_new(_("IRQ:"));
  gtk_misc_set_alignment(GTK_MISC(_ctc.irq_l[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_ctc.table[c]), _ctc.irq_l[c],
		   0, 1, 1, 2,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_ctc.irq_l[c]);

  _ctc.irq_v[c] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_ctc.irq_v[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_ctc.table[c]), _ctc.irq_v[c],
		   1, 2, 1, 2,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_ctc.irq_v[c]);

  /*
   *  irq vector
   */
  _ctc.irqvec_l[c] = gtk_label_new(_("IRQ Vector:"));
  gtk_misc_set_alignment(GTK_MISC(_ctc.irqvec_l[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_ctc.table[c]), _ctc.irqvec_l[c],
		   0, 1, 2, 3,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_ctc.irqvec_l[c]);

  _ctc.irqvec_v[c] = gtk_label_new("");
  gtk_misc_set_alignment(GTK_MISC(_ctc.irqvec_v[c]), 0, 0.5);
  gtk_table_attach(GTK_TABLE(_ctc.table[c]), _ctc.irqvec_v[c],
		   1, 2, 2, 3,
		   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
		   (GtkAttachOptions)0, 0, 0);
  gtk_widget_show(_ctc.irqvec_v[c]);
  return _ctc.frame[c];
}

GtkWidget *
InfoWindow::create_page_ctc()
{
  GtkWidget *w;

  _ctc.hbox = gtk_hbox_new(TRUE, 0);
  gtk_widget_show(_ctc.hbox);

  w = create_channel_ctc(0, _("Channel 0"));
  gtk_box_pack_start(GTK_BOX(_ctc.hbox), w, TRUE, TRUE, 0);
  gtk_widget_show(w);

  w = create_channel_ctc(1, _("Channel 1"));
  gtk_box_pack_start(GTK_BOX(_ctc.hbox), w, TRUE, TRUE, 0);
  gtk_widget_show(w);

  w = create_channel_ctc(2, _("Channel 2"));
  gtk_box_pack_start(GTK_BOX(_ctc.hbox), w, TRUE, TRUE, 0);
  gtk_widget_show(w);

  w = create_channel_ctc(3, _("Channel 3"));
  gtk_box_pack_start(GTK_BOX(_ctc.hbox), w, TRUE, TRUE, 0);
  gtk_widget_show(w);

  return _ctc.hbox;
}

void
InfoWindow::init(void)
{
  /*
   *  window
   */
  _window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(_window, "InfoWindow");
  gtk_window_set_title(GTK_WINDOW(_window), _("KCemu: Info"));
  gtk_widget_set_uposition(_window, 650, 50);
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event",
		     GTK_SIGNAL_FUNC(cmd_exec_sft),
		     (char *)"ui-info-window-toggle"); // FIXME:

  /*
   *  vbox
   */
  _w.vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(_window), _w.vbox);
  gtk_widget_show(_w.vbox);

  /*
   *  notebook
   */
  _w.notebook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.notebook,
		     TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(_w.notebook), "switch_page",
		     GTK_SIGNAL_FUNC(switch_page), this);
  gtk_widget_show(_w.notebook);

  gtk_notebook_append_page(GTK_NOTEBOOK(_w.notebook),
			   create_page_cpu(),
			   gtk_label_new(_("CPU")));
  gtk_notebook_append_page(GTK_NOTEBOOK(_w.notebook),
			   create_page_pio(),
			   gtk_label_new(_("PIO")));
  gtk_notebook_append_page(GTK_NOTEBOOK(_w.notebook),
			   create_page_ctc(),
			   gtk_label_new(_("CTC")));

  /*
   *  separator
   */
  _w.separator = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.separator,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.separator);

  /*
   *  button box
   */
  _w.bbox = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(_w.bbox),
			    GTK_BUTTONBOX_END);
  gtk_box_pack_start(GTK_BOX(_w.vbox), _w.bbox,
		     FALSE, FALSE, 5);
  gtk_widget_show(_w.bbox);

  /*
   *  update button
   */
  _w.b_update = gtk_button_new_with_label(_("Update"));
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_update);
  gtk_signal_connect(GTK_OBJECT(_w.b_update), "clicked",
		     GTK_SIGNAL_FUNC(update), this);
  GTK_WIDGET_SET_FLAGS(_w.b_update, GTK_CAN_DEFAULT);
  gtk_widget_show(_w.b_update);

  /*
   *  close button
   */
  _w.b_close = gtk_button_new_with_label(_("Close"));
  gtk_container_add(GTK_CONTAINER(_w.bbox), _w.b_close);
  gtk_signal_connect(GTK_OBJECT(_w.b_close), "clicked",
		     GTK_SIGNAL_FUNC(cmd_exec_sf),
		     (char *)"ui-info-window-toggle"); // FIXME:
  GTK_WIDGET_SET_FLAGS(_w.b_close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(_w.b_close);
  gtk_widget_show(_w.b_close);
}
