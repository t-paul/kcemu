#include "mtk.h"

MTK_Colors * MTK_Colors::_self = NULL;

MTK_Colors::MTK_Colors(void)
{
  SDL_Color col[] = {
    { 200, 200, 200,   0 }, // normal
    {   0,   0,   0,   0 }, // background
    { 255, 255, 255,   0 }, // highlight
    {   0, 255,   0,   0 }, // pressed
  };
  
  for (int a = 0;a < 4;a++)
    _col[a] = col[a];
}

MTK_Colors::~MTK_Colors(void)
{
}

MTK_Colors *
MTK_Colors::get_instance(void)
{
  if (_self == NULL)
    _self = new MTK_Colors();

  return _self;
}

SDL_Color
MTK_Colors::get_foreground(void)
{
  return _col[0];
}

SDL_Color
MTK_Colors::get_background(void)
{
  return _col[1];
}

SDL_Color
MTK_Colors::get_highlight(void)
{
  return _col[2];
}

SDL_Color
MTK_Colors::get_pressed(void)
{
  return _col[3];
}
