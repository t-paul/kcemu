#include "mtk.h"

MTK_Widget::MTK_Widget(void)
{
  set_parent(NULL);
  set_rect(0, 0, 0, 0);
  _dirty = false;
  _visible = false;
}

MTK_Widget::~MTK_Widget(void)
{
}

SDL_Rect *
MTK_Widget::get_rect(void)
{
  return &_rect;
}

int
MTK_Widget::get_x(void)
{
  return _rect.x;
}

void
MTK_Widget::set_x(int x)
{
  if (_rect.x == x)
    return;

  _rect.x = x;
  set_dirty(true);
}

int
MTK_Widget::get_y(void)
{
  return _rect.y;
}

void
MTK_Widget::set_y(int y)
{
  if (_rect.y == y)
    return;

  _rect.y = y;
  set_dirty(true);
}

int
MTK_Widget::get_width(void)
{
  return _rect.w;
}

void
MTK_Widget::set_width(int width)
{
  if (_rect.w == width)
    return;

  _rect.w = width;
  set_dirty(true);
}

int
MTK_Widget::get_height(void)
{
  return _rect.h;
}

void
MTK_Widget::set_height(int height)
{
  if (_rect.h == height)
    return;

  _rect.h = height;
  set_dirty(true);
}

void
MTK_Widget::set_rect(int x, int y, int width, int height)
{
  _rect.x = x;
  _rect.y = y;
  _rect.w = width;
  _rect.h = height;

  set_dirty(true);
}

bool
MTK_Widget::in_rect(SDL_Rect *rect, int x, int y)
{
  if (x < rect->x)
    return false;

  if (x >= (rect->x + rect->w))
    return false;

  if (y < rect->y)
    return false;
      
  if (y >= (rect->y + rect->h))
    return false;

  return true;
}

bool
MTK_Widget::is_dirty(void)
{
  return _dirty;
}

void
MTK_Widget::set_dirty(bool dirty)
{
  _dirty = dirty;

  if (dirty)
    {
      MTK_Widget *parent = get_parent();
      if (parent)
	parent->set_dirty(dirty);
    }
}

bool
MTK_Widget::is_visible(void)
{
  return _visible;
}

void
MTK_Widget::set_visible(bool visible)
{
  if (_visible != visible)
    {
      _visible = visible;
      set_dirty(true);
    }
}

MTK_Widget *
MTK_Widget::get_parent(void)
{
  return _parent;
}

void
MTK_Widget::set_parent(MTK_Widget *widget)
{
  _parent = widget;
}

void
MTK_Widget::handle_event(SDL_Event *event)
{
}
