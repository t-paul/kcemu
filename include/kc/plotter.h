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

#ifndef __kc_plotter_h
#define __kc_plotter_h

#include <cairo/cairo.h>

#include "kc/kc.h"

class Plotter
{
private:
  /** paper width in mm (A4) */
  static const double WIDTH_MM;

  /** paper height in mm (A4) */
  static const double HEIGHT_MM;

  /** plotting area width in mm */
  static const double PLOTTING_AREA_WIDTH_MM;

  /** plotting area height in mm */
  static const double PLOTTING_AREA_HEIGHT_MM;

  /** scaling factor for the buffer image surface */
  static const double BUFFER_SURFACE_SCALE;

  /** scaling factor for the onscreen image surface */
  static const double ONSCREEN_SURFACE_SCALE;

  /** plotter step width in cm */
  static const double STEP_WIDTH;

  /** convert mm to points per inch (for pdf output) */
  static const double MM_TO_INCH;

private:
  /** current coordianates of the plotting pen */
  double _x, _y;

  /** pen color, a.k.a. the foreground color */
  double _red, _green, _blue;

  /** paper color, a.k.a. the background color */
  double _bg_red, _bg_green, _bg_blue;

  double _line_width;
  double _pen_down_factor;
  bool   _pen_down;

  bool   _show_pen;
  bool   _show_plot_area;

  double _origin_x;
  double _origin_y;
  
  double _origin_x_new;
  double _origin_y_new;

  bool   _dirty;
  double _dirty_x1;
  double _dirty_y1;
  double _dirty_x2;
  double _dirty_y2;
  bool   _invalidated;

  /**
   * Image surface that is used for direct onscreen rendering.
   */
  cairo_t *_onscreen_cr;
  cairo_surface_t *_onscreen_surface;

  /**
   * Cairo surface and context for the double buffer image, the
   * size of the image will be 2100 x 2970 pixel which matches the
   * resolution of the plotter XY4131 (A4 paper size with step
   * width of 0.1mm). The actual plotting area is 1800 x 2550 pixel.
   */
  cairo_t *_buffer_cr;
  cairo_surface_t *_buffer_surface;

  /**
   * PDF surface and context that is only present if a pdf file
   * was opened by the user.
   */
  cairo_t *_pdf_cr;
  cairo_surface_t *_pdf_surface;

private:
  virtual void set_point(cairo_t *cr, double x, double y);
  virtual void draw_to(cairo_t *cr, double x, double y);

  virtual void invalidate(void);
  virtual void clear_dirty_rect(void);
  virtual void update_dirty_rect(double x, double y);

  virtual cairo_t * replace_cairo_context(cairo_t *old_cr, cairo_surface_t *surface, double scale_factor);
  virtual void clear_surface(cairo_surface_t *surface, double scale_factor);
  virtual void init_image_surface(cairo_surface_t **surface, cairo_t **cr, double width, double height);
  
public:
  Plotter(void);
  virtual ~Plotter(void);

  virtual void init(void);

  virtual void pen_up(void);
  virtual void pen_down(void);
  virtual void step(int delta_x, int delta_y);
  virtual void show_page(void);

  virtual void open_pdf(const char *filename);
  virtual void close_pdf(void);

  virtual cairo_status_t save_as_png(const char *filename);

  virtual bool is_dirty(void);
  virtual cairo_rectangle_t * get_dirty_rectangle(void);
  virtual cairo_surface_t * get_onscreen_surface(void);

  virtual double get_line_width(void);
  virtual void set_line_width(double line_width);

  virtual double get_origin_x(void);
  virtual void set_origin_x(double origin_x);

  virtual double get_origin_y(void);
  virtual void set_origin_y(double origin_y);

  virtual double get_bg_red(void);
  virtual double get_bg_green(void);
  virtual double get_bg_blue(void);
  virtual void set_bg_color(double red, double green, double blue);

  virtual double get_pen_red(void);
  virtual double get_pen_green(void);
  virtual double get_pen_blue(void);
  virtual void set_pen_color(double red, double green, double blue);

  virtual double get_x(void);
  virtual double get_y(void);

  virtual bool is_show_pen(void);
  virtual void set_show_pen(bool show_pen);

  virtual bool is_show_plot_area(void);
  virtual void set_show_plot_area(bool show_plot_area);

  virtual bool is_show_helpers(void);

  virtual bool is_pen_down(void);
};

#endif /* __kc_plotter_h */
