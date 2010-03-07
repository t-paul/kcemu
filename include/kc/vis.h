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

#ifndef __kc_vis_h
#define __kc_vis_h

#include "kc/system.h"

#include "kc/ic.h"
#include "kc/ports.h"

class VIS : public PortInterface, public InterfaceCircuit
{
 private:
  int _idx;
  int _mode;
  int _border;
  int _color[4];
  byte_t _char[0x0800];
  byte_t _changed[0x100];
  int _color_palette_changed;

 public:
  VIS(void);
  virtual ~VIS(void);

  virtual byte_t in(word_t addr);
  virtual void out(word_t addr, byte_t val);

  virtual int get_mode(void);
  virtual void set_mode(int mode);
  virtual int get_border(void);
  virtual void set_border(int border);
  virtual int get_color(int idx);
  virtual void set_color(int idx, int color);
  virtual int get_changed(byte_t idx);
  virtual void reset_changed(void);
  virtual int is_color_palette_changed(void);
  virtual void reset_color_palette_changed(void);

  virtual byte_t * get_memory(void);

  /*
   *  InterfaceCircuit
   */
  virtual void reti(void);
  virtual void irqreq(void);
  virtual word_t irqack(void);
  virtual void reset(bool power_on = false);
};

#endif /* __kc_vis_h */
