#include "mtk.h"

MTK_VBox::MTK_VBox(int spacing)
{
  _spacing = spacing;
}

MTK_VBox::~MTK_VBox(void)
{
}

void
MTK_VBox::layout(void)
{
  _width = 0;
  _y = _spacing;
  process_children(this);
  set_width(_width);
  set_height(_y);
}

void
MTK_VBox::process_child(MTK_Widget *widget)
{
  widget->set_y(get_y() + _y);
  widget->set_x(get_x());
  _y += widget->get_height() + _spacing;

  int w = widget->get_width();
  if (w > _width)
    _width = w;
}
