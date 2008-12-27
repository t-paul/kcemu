/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2008 Torsten Paul
 *
 *  $Id$
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

#ifndef __kc_plotter_h
#define __kc_plotter_h

#include <cairo/cairo.h>

#include "kc/kc.h"

class Plotter
{
private:
  /** paper width in mm (A4) */
  static const double WIDTH_MM = 210.0;

  /** paper height in mm (A4) */
  static const double HEIGHT_MM = 297.0;

  /** plotter step width in cm */
  static const double STEP_WIDTH = 0.1;

  /** convert mm to points per inch (for pdf output) */
  static const double MM_TO_INCH = 72.0 / 25.4;

private:
  double _x;
  double _y;
  double _red;
  double _green;
  double _blue;
  double _line_width;
  double _pen_down_factor;
  double _origin_x;
  double _origin_y;
  bool   _pen_down;

  int _image_width;
  int _image_height;

  /**
   * Cairo surface and context for the double buffer image, the
   * size of the image will 2100 x 2970 pixel which matches the
   * resolution of the plotter XY4131 (A4 paper size with step
   * width of 0.1mm).
   */
  cairo_t *_buffer_cr;
  cairo_surface_t *_buffer_surface;

  /**
   * PDF surface and context that is only present if a pdf file
   * was opened by the user.
   */
  cairo_t *_pdf_cr;
  cairo_surface_t *_pdf_surface;

  /**
   * Image surface that is used for direct onscreen rendering. The
   * size is tracking the actual canvas size of the GUI window.
   * After changing the window size the content is restored from
   * the buffer surface.
   */
  cairo_t *_onscreen_cr;
  cairo_surface_t *_onscreen_surface;

private:
  virtual void set_point(cairo_t *cr, double x, double y);
  virtual void draw_to(cairo_t *cr, double x, double y);

  virtual void clear_image_surface(cairo_surface_t *surface);
  virtual void init_image_surface(cairo_surface_t **surface, cairo_t **cr, double width, double height);
  
public:
  Plotter(void);
  virtual ~Plotter(void);

  virtual void init(void);
  virtual void close(void);

  virtual void pen_up(void);
  virtual void pen_down(void);
  virtual void step(int delta_x, int delta_y);
  virtual void show_page(void);
  virtual cairo_status_t save_as_png(const char *filename, double height);

  virtual void open_pdf(const char *filename);

  virtual cairo_surface_t * get_pdf_surface(void);
  virtual cairo_surface_t * get_onscreen_surface(int width, int height);

  virtual double get_line_width(void);
  virtual void set_line_width(double line_width);

  virtual double get_origin_x(void);
  virtual void set_origin_x(double origin_x);

  virtual double get_origin_y(void);
  virtual void set_origin_y(double origin_y);

  virtual double get_pen_red(void);
  virtual void set_pen_red(double red);

  virtual double get_pen_green(void);
  virtual void set_pen_green(double green);

  virtual double get_pen_blue(void);
  virtual void set_pen_blue(double blue);

  virtual void set_pen_color(double red, double green, double blue);

  virtual double get_x(void);
  virtual double get_y(void);
  virtual bool is_pen_down(void);
};

#endif /* __kc_plotter_h */
