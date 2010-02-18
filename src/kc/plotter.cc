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

#include <math.h>
#include <stdio.h>

#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-svg.h>

#include "kc/system.h"

#include "kc/plotter.h"

Plotter::Plotter(void)
{
  init();
}

Plotter::~Plotter(void)
{
}

void
Plotter::init(void)
{
  _x = 0;
  _y = 0;
  _pen_down_factor = 1.2;
  _show_pen = false;
  _show_plot_area = false;

  _pdf_cr = 0;
  _pdf_surface = 0;

  _buffer_cr = 0;
  _buffer_surface = 0;

  _onscreen_cr = 0;
  _onscreen_surface = 0;
  
  _origin_x = _origin_x_new = (WIDTH_MM - PLOTTING_AREA_WIDTH_MM) / 2.0;
  _origin_y = _origin_y_new = (HEIGHT_MM - PLOTTING_AREA_HEIGHT_MM) / 2.0;

  clear_dirty_rect();
  
  pen_up();
  set_line_width(0.2);
  set_bg_color(1, 1, 1);
  set_pen_color(0, 0, 0);

  init_image_surface(&_buffer_surface, &_buffer_cr, WIDTH_MM * BUFFER_SURFACE_SCALE, HEIGHT_MM * BUFFER_SURFACE_SCALE);
  init_image_surface(&_onscreen_surface, &_onscreen_cr, WIDTH_MM * ONSCREEN_SURFACE_SCALE, HEIGHT_MM * ONSCREEN_SURFACE_SCALE);
}

void
Plotter::clear_dirty_rect(void)
{
  _dirty = false;
  _dirty_x1 = PLOTTING_AREA_WIDTH_MM;
  _dirty_y1 = PLOTTING_AREA_HEIGHT_MM;
  _dirty_x2 = 0;
  _dirty_y2 = 0;
}

void
Plotter::invalidate(void)
{
  _dirty = true;
  _invalidated = true;
}

void
Plotter::update_dirty_rect(double x, double y)
{
  _dirty = true;
  if (x < _dirty_x1)
    _dirty_x1 = x;
  if (y < _dirty_y1)
    _dirty_y1 = y;
  if (x > _dirty_x2)
    _dirty_x2 = x;
  if (y > _dirty_y2)
    _dirty_y2 = y;
}

bool
Plotter::is_dirty(void)
{
  return _dirty;
}

cairo_rectangle_t *
Plotter::get_dirty_rectangle(void)
{
  static cairo_rectangle_t rect;
  static cairo_rectangle_t rect_full = { 0, 0, WIDTH_MM * ONSCREEN_SURFACE_SCALE, HEIGHT_MM * ONSCREEN_SURFACE_SCALE};

  if (_invalidated)
    {
      _invalidated = false;
      return &rect_full;
    }
  
  if (!_dirty)
    return NULL;

  cairo_rectangle_t *r;
  if (is_show_helpers())
    {
      r = &rect_full;
    }
  else
    {
      if (_dirty_x1 > _dirty_x2)
        return NULL;

      r = &rect;
      rect.x = _dirty_x1 + _origin_x - 2 * _line_width;
      rect.y = _dirty_y1 + _origin_y - 2 * _line_width;
      rect.width = _dirty_x2 - _dirty_x1 + 4 * _line_width;
      rect.height = _dirty_y2 - _dirty_y1 + 4 * _line_width;

      rect.x *= ONSCREEN_SURFACE_SCALE;
      rect.y *= ONSCREEN_SURFACE_SCALE;
      rect.width *= ONSCREEN_SURFACE_SCALE;
      rect.height *= ONSCREEN_SURFACE_SCALE;

      rect.x -= 2;
      rect.y -= 2;
      rect.width += 4;
      rect.height += 4;

      if (rect.x < 0)
        rect.x = 0;
      if (rect.y < 0)
        rect.y = 0;
    }

  clear_dirty_rect();

  return r;
}

cairo_t *
Plotter::replace_cairo_context(cairo_t *old_cr, cairo_surface_t *surface, double scale_factor)
{
  cairo_t *cr = cairo_create(surface);

  cairo_scale(cr, scale_factor, scale_factor);
  cairo_translate(cr, _origin_x, _origin_y);

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

  cairo_set_line_width(cr, _line_width);
  cairo_set_source_rgb(cr, _red, _green, _blue);

  cairo_destroy(old_cr);
  
  return cr;
}

void
Plotter::init_image_surface(cairo_surface_t **surface, cairo_t **cr, double width, double height)
{
  *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  *cr = replace_cairo_context(*cr, *surface, width / WIDTH_MM);
  cairo_surface_destroy(*surface);

  clear_surface(*surface, width / WIDTH_MM);
}

void
Plotter::clear_surface(cairo_surface_t *surface, double scale_factor)
{
  cairo_t *cr = cairo_create(surface);
  cairo_scale(cr, scale_factor, scale_factor);
  cairo_set_source_rgb(cr, _bg_red, _bg_green, _bg_blue);
  cairo_rectangle(cr, 0, 0, WIDTH_MM, HEIGHT_MM);
  cairo_fill(cr);
  cairo_destroy(cr);
}

void
Plotter::open_pdf(const char *filename)
{
  close_pdf();
  
  cairo_surface_t *surface = cairo_pdf_surface_create(filename, WIDTH_MM * MM_TO_INCH, HEIGHT_MM * MM_TO_INCH);
  cairo_status_t status = cairo_surface_status(surface);
  if (status == CAIRO_STATUS_SUCCESS)
    {
      _pdf_cr = replace_cairo_context(_pdf_cr, surface, MM_TO_INCH);
      _pdf_surface = surface;
    }
  cairo_surface_destroy(surface);
}

void
Plotter::close_pdf(void)
{
  if (_pdf_cr)
    cairo_destroy(_pdf_cr);

  _pdf_cr = 0;
  _pdf_surface = 0;
}

void
Plotter::show_page(void)
{
  _origin_x = _origin_x_new;
  _origin_y = _origin_y_new;
  
  if (_pdf_surface)
    {
      cairo_surface_show_page(_pdf_surface);
      clear_surface(_pdf_surface, MM_TO_INCH);
      _pdf_cr = replace_cairo_context(_pdf_cr, _pdf_surface, MM_TO_INCH);
    }

  clear_surface(_buffer_surface, BUFFER_SURFACE_SCALE);
  _buffer_cr = replace_cairo_context(_buffer_cr, _buffer_surface, BUFFER_SURFACE_SCALE);

  clear_surface(_onscreen_surface, ONSCREEN_SURFACE_SCALE);
  _onscreen_cr = replace_cairo_context(_onscreen_cr, _onscreen_surface, ONSCREEN_SURFACE_SCALE);

  invalidate();
}

cairo_status_t
Plotter::save_as_png(const char *filename)
{
  cairo_status_t status = cairo_surface_write_to_png(_buffer_surface, filename);
  return status;
}

cairo_surface_t *
Plotter::get_onscreen_surface()
{
  static cairo_surface_t *surface = NULL;

  if (!is_show_helpers())
    return _onscreen_surface;

  if (surface)
    cairo_surface_destroy(surface);

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, WIDTH_MM * ONSCREEN_SURFACE_SCALE, HEIGHT_MM * ONSCREEN_SURFACE_SCALE);
  cairo_t *cr = cairo_create(surface);

  cairo_set_source_surface(cr, _onscreen_surface, 0, 0);
  cairo_paint(cr);

  cairo_scale(cr, ONSCREEN_SURFACE_SCALE, ONSCREEN_SURFACE_SCALE);
  cairo_translate(cr, _origin_x, _origin_y);

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

  cairo_set_line_width(cr, 0.1);

  if (is_show_pen())
    {
      cairo_set_source_rgba(cr, 1, 0, 0, 0.4);
      cairo_move_to(cr, get_x(), get_y());
      cairo_arc(cr, get_x(), get_y(), 1, 0, 2 * M_PI);
      cairo_fill(cr);

      cairo_set_source_rgba(cr, 0, 0, 1, 0.8);
      cairo_move_to(cr, get_x() - 5, get_y());
      cairo_rel_line_to(cr, 10, 0);
      cairo_stroke(cr);

      cairo_move_to(cr, get_x(), get_y() - 5);
      cairo_rel_line_to(cr, 0, 10);
      cairo_stroke(cr);
    }

  if (is_show_plot_area())
    {
      cairo_set_source_rgba(cr, 0, 1, 0, 0.8);
      cairo_rectangle(cr, 0, 0, PLOTTING_AREA_WIDTH_MM, PLOTTING_AREA_HEIGHT_MM);
      cairo_stroke(cr);
    }

  cairo_destroy(cr);

  return surface;
}

void
Plotter::pen_up(void)
{
  _pen_down = false;
}

void
Plotter::pen_down(void)
{
  _pen_down = true;

  set_point(_buffer_cr, get_x(), get_y());
  set_point(_onscreen_cr, get_x(), get_y());

  if (_pdf_cr)
    set_point(_pdf_cr, get_x(), get_y());
}

void
Plotter::step(int delta_x, int delta_y)
{
  double x = _x + delta_x * STEP_WIDTH;
  double y = _y + delta_y * STEP_WIDTH;

  if (x < 0)
    x = 0;
  if (x > PLOTTING_AREA_WIDTH_MM)
    x = PLOTTING_AREA_WIDTH_MM;

  if (y < 0)
    y = 0;
  if (y > PLOTTING_AREA_HEIGHT_MM)
    y = PLOTTING_AREA_HEIGHT_MM;

  draw_to(_buffer_cr, x, y);
  draw_to(_onscreen_cr, x, y);
  
  if (_pdf_cr)
    draw_to(_pdf_cr, x, y);

  _x = x;
  _y = y;
}

double
Plotter::get_x(void)
{
  return _x;
}

double
Plotter::get_y(void)
{
  return _y;
}

bool
Plotter::is_pen_down(void)
{
  return _pen_down;
}

double
Plotter::get_line_width(void)
{
  return _line_width;
}

void
Plotter::set_line_width(double line_width)
{
  _line_width = line_width;

  if (_buffer_cr)
    cairo_set_line_width(_buffer_cr, _line_width);
  
  if (_onscreen_cr)
    cairo_set_line_width(_onscreen_cr, _line_width);

  if (_pdf_cr)
    cairo_set_line_width(_pdf_cr, _line_width);
}

double
Plotter::get_origin_x(void)
{
  return _origin_x_new;
}

void
Plotter::set_origin_x(double origin_x)
{
  _origin_x_new = origin_x;
}

double
Plotter::get_origin_y(void)
{
  return _origin_y_new;
}

void
Plotter::set_origin_y(double origin_y)
{
  _origin_y_new = origin_y;
}

double
Plotter::get_bg_red(void)
{
  return _bg_red;
}

double
Plotter::get_bg_green(void)
{
  return _bg_green;
}

double
Plotter::get_bg_blue(void)
{
  return _bg_blue;
}

void
Plotter::set_bg_color(double red, double green, double blue)
{
  _bg_red = red;
  _bg_green = green;
  _bg_blue = blue;
}

double
Plotter::get_pen_red(void)
{
  return _red;
}

double
Plotter::get_pen_green(void)
{
  return _green;
}

double
Plotter::get_pen_blue(void)
{
  return _blue;
}

void
Plotter::set_pen_color(double red, double green, double blue)
{
  _red = red;
  _green = green;
  _blue = blue;

  if (_buffer_cr)
    cairo_set_source_rgb(_buffer_cr, _red, _green, _blue);

  if (_onscreen_cr)
    cairo_set_source_rgb(_onscreen_cr, _red, _green, _blue);

  if (_pdf_cr)
    cairo_set_source_rgb(_pdf_cr, _red, _green, _blue);
}

bool
Plotter::is_show_pen(void)
{
  return _show_pen;
}

void
Plotter::set_show_pen(bool show_pen)
{
  invalidate();
  _show_pen = show_pen;
}

bool
Plotter::is_show_plot_area(void)
{
  return _show_plot_area;
}

void
Plotter::set_show_plot_area(bool show_plot_area)
{
  invalidate();
  _show_plot_area = show_plot_area;
}

bool
Plotter::is_show_helpers(void)
{
  return is_show_pen() || is_show_plot_area();
}

void
Plotter::set_point(cairo_t *cr, double x, double y)
{
  cairo_move_to(cr, x, y);
  cairo_save(cr);
  cairo_set_line_width(cr, cairo_get_line_width(cr) * _pen_down_factor);
  cairo_rel_line_to(cr, 0, 0);
  cairo_stroke(cr);
  cairo_restore(cr);
  update_dirty_rect(x, y);
}

void
Plotter::draw_to(cairo_t *cr, double x, double y)
{
  // force dirty status also when just moving the cursor
  // this is required for the cursor indicator to also
  // generate update requests
  _dirty = true;

  if (is_pen_down())
    {
      cairo_line_to(cr, x, y);
      cairo_stroke(cr);
      update_dirty_rect(x, y);
    }
  cairo_move_to(cr, x, y);
}
