#include "mtk.h"

MTK_HBox::MTK_HBox(int spacing)
{
  _spacing = spacing;
}

MTK_HBox::~MTK_HBox(void)
{
}

void
MTK_HBox::layout(void)
{
  _height = 0;
  _x = _spacing;
  process_children(this);
  set_height(_height);
  set_width(_x);
}

void
MTK_HBox::process_child(MTK_Widget *widget)
{
  widget->set_x(get_x() + _x);
  widget->set_y(get_y());
  _x += widget->get_width() + _spacing;

  int h = widget->get_height();
  if (h > _height)
    _height = h;
}
