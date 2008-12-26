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

#include <stdio.h>

#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>

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
  _pdf_cr = 0;
  _pdf_surface = 0;
  _origin_x = 20.0;
  _origin_y = 20.0;
  _image_surface = 0;
  _image_width = 0;
  _image_height = 0;
  set_line_width(0.2);
  pen_up();
}

void
Plotter::close(void)
{
  if (_pdf_cr)
    cairo_destroy(_pdf_cr);

  _pdf_cr = 0;
  _pdf_surface = 0;
}

void
Plotter::open_pdf(const char *filename)
{
  close();
  
  cairo_surface_t *surface = cairo_pdf_surface_create(filename, WIDTH_MM * MM_TO_INCH, HEIGHT_MM * MM_TO_INCH);
  cairo_status_t status = cairo_surface_status(surface);
  if (status == CAIRO_STATUS_SUCCESS)
    {
      _pdf_cr = cairo_create(surface);

      cairo_scale(_pdf_cr, MM_TO_INCH, MM_TO_INCH);
      cairo_translate(_pdf_cr, _origin_x, _origin_y);
      cairo_set_line_width(_pdf_cr, _line_width);
      cairo_set_line_cap(_pdf_cr, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_join(_pdf_cr, CAIRO_LINE_JOIN_ROUND);
      cairo_set_source_rgb(_pdf_cr, 0, 0, 0);
      _pdf_surface = surface;
    }
  cairo_surface_destroy(surface);
}

cairo_surface_t *
Plotter::get_pdf_surface(void)
{
  return _pdf_surface;
}

void
Plotter::show_page(void)
{
  cairo_surface_t *surface = get_pdf_surface();
  cairo_surface_show_page(surface);
  create_image_surface();
}

cairo_status_t
Plotter::save_as_png(const char *filename, double height)
{
  double h = height;
  double w = height * 0.7070;
  double scalex = w / (WIDTH_MM * MM_TO_INCH);
  double scaley = h / (HEIGHT_MM * MM_TO_INCH);

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
  cairo_t *cr = cairo_create(surface);
  cairo_surface_destroy(surface);
  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, 0, 0, w, h);
  cairo_fill(cr);
  cairo_scale(cr, scalex, scaley);
  cairo_set_source_surface(cr, plotter->get_pdf_surface(), 0, 0);
  cairo_paint(cr);
  cairo_status_t status = cairo_surface_write_to_png(surface, filename);
  cairo_destroy(cr);
  return status;
}

void
Plotter::create_image_surface(void)
{
  if (_image_surface)
    cairo_surface_destroy(_image_surface);

  double scalex = _image_width / (WIDTH_MM * MM_TO_INCH);
  double scaley = _image_height / (HEIGHT_MM * MM_TO_INCH);
  _image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, _image_width, _image_height);
  cairo_t *cr = cairo_create(_image_surface);

  cairo_set_source_rgb(cr, 1, 1, 1);
  cairo_rectangle(cr, 0, 0, _image_width, _image_height);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 0, 0, 0);

  cairo_scale(cr, scalex, scaley);
  cairo_set_source_surface(cr, _pdf_surface, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);
}

cairo_surface_t *
Plotter::get_image_surface(int width, int height)
{
  if ((_image_width != width) || (_image_height != height))
    {
      _image_width = width;
      _image_height = height;

      create_image_surface();
    }

  return _image_surface;
}

cairo_t *
Plotter::get_image_cr(double x, double y)
{
  double scalex = _image_width / WIDTH_MM;
  double scaley = _image_height / HEIGHT_MM;

  cairo_t *cr = cairo_create(_image_surface);
  cairo_set_line_width(cr, _line_width);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_scale(cr, scalex, scaley);
  cairo_translate(cr, _origin_x, _origin_y);
  cairo_move_to(cr, x, y);
  return cr;
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

  set_point(_pdf_cr, get_x(), get_y(), _line_width);

  if (_image_surface)
    {
      cairo_t *cr = get_image_cr(get_x(), get_y());
      set_point(cr, get_x(), get_y(), _line_width);
      cairo_destroy(cr);
    }
}

void
Plotter::step(int delta_x, int delta_y)
{
  double x = _x + delta_x * STEP_WIDTH;
  double y = _y + delta_y * STEP_WIDTH;

  if (x < 0)
    x = 0;
  if (x > WIDTH_MM)
    x = WIDTH_MM;

  if (y < 0)
    y = 0;
  if (y > HEIGHT_MM)
    y = HEIGHT_MM;

  draw_to(_pdf_cr, x, y);
  if (_image_surface)
    {
      cairo_t *cr = get_image_cr(get_x(), get_y());
      draw_to(cr, x, y);
      cairo_destroy(cr);
    }

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
}

double
Plotter::get_origin_x(void)
{
  return _origin_x;
}

void
Plotter::set_origin_x(double origin_x)
{
  _origin_x = origin_x;
}

double
Plotter::get_origin_y(void)
{
  return _origin_y;
}

void
Plotter::set_origin_y(double origin_y)
{
  _origin_y = origin_y;
}

void
Plotter::set_point(cairo_t *cr, double x, double y, double line_width)
{
  cairo_move_to(cr, x - (line_width / 2.0), y);
  cairo_rel_line_to(cr, line_width, 0);
  cairo_stroke(cr);
}

void
Plotter::draw_to(cairo_t *cr, double x, double y)
{
  if (is_pen_down())
    {
      cairo_line_to(cr, x, y);
      cairo_stroke(cr);
    }
  cairo_move_to(cr, x, y);
}
