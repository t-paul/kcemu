#include "mtk.h"

MTK_Screen * MTK_Screen::_self = NULL;

MTK_Screen::MTK_Screen(void)
{
  _blit = false;
  _callback = &_dummy_callback;
}

MTK_Screen::~MTK_Screen(void)
{
}

MTK_Screen *
MTK_Screen::get_instance(void)
{
  if (_self == NULL)
    _self = new MTK_Screen();

  return _self;
}

void
MTK_Screen::add_rect(SDL_Rect *rect)
{
  int max_x, max_y;

  if (!_blit)
    {
      _rect.x = rect->x;
      _rect.y = rect->y;
      _rect.w = rect->w;
      _rect.h = rect->h;
    }
  else
    {
      max_x = rect->x + rect->w;
      if ((_rect.x + _rect.w) > max_x)
	max_x = _rect.x + _rect.w;

      max_y = rect->y + rect->h;
      if ((_rect.y + _rect.h) > max_y)
	max_y = _rect.y + _rect.h;

      if (rect->x < _rect.x)
	_rect.x = rect->x;
      if (rect->y < _rect.y)
	_rect.y = rect->y;

      _rect.w = max_x - _rect.x;
      _rect.h = max_y - _rect.y;
    }

  _blit = true;
}

void
MTK_Screen::blit(SDL_Surface *screen, SDL_Surface *background, SDL_Surface *ui, bool clear_cache)
{
  if (!clear_cache)
    if (!_blit)
      return;

  _blit = false;

  if (clear_cache)
    {
      SDL_BlitSurface(background, NULL, screen, NULL);
      SDL_BlitSurface(ui, NULL, screen, NULL);
      SDL_UpdateRect(screen, 0, 0, 0, 0);
      return;
    }

  SDL_BlitSurface(background, &_rect, screen, &_rect);
  SDL_BlitSurface(ui, &_rect, screen, &_rect);
  SDL_UpdateRect(screen, _rect.x, _rect.y, _rect.w, _rect.h);
}

void
MTK_Screen::layout(void)
{
}

void
MTK_Screen::process_child(MTK_Widget *widget)
{
}

MTK_Callback *
MTK_Screen::get_callback(void)
{
  return _callback;
}

void
MTK_Screen::set_callback(MTK_Callback *callback)
{
  if (callback == NULL)
    _callback = &_dummy_callback;
  else
    _callback = callback;
}
