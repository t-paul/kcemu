#include "mtk.h"

MTK_Container::MTK_Container(void)
{
}

MTK_Container::~MTK_Container(void)
{
}

void
MTK_Container::set_visible(bool visible)
{
  MTK_Widget::set_visible(visible);
  for (widget_list_t::iterator it = _children.begin();it != _children.end();it++)
    (*it)->set_visible(visible);
}

void
MTK_Container::add(MTK_Widget *widget)
{
  _children.push_back(widget);
  widget->set_parent(this);
  set_dirty(true);
}

bool
MTK_Container::render(SDL_Surface *screen, bool clear_cache)
{
  if (is_dirty())
    layout();

  bool ret = false;
  for (widget_list_t::iterator it = _children.begin();it != _children.end();it++)
    if ((*it)->render(screen, clear_cache))
      ret = true;

  set_dirty(false);

  return ret;
}

void
MTK_Container::process_children(MTK_Container *container)
{
  for (widget_list_t::iterator it = _children.begin();it != _children.end();it++)
    container->process_child(*it);
}

void
MTK_Container::handle_event(SDL_Event *event)
{
  for (widget_list_t::iterator it = _children.begin();it != _children.end();it++)
    (*it)->handle_event(event);
}
