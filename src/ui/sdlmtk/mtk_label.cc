#include <string.h>
#include <malloc.h>

#include "mtk.h"

MTK_Label::MTK_Label(const char *text)
{
  _text = strdup(text);
  _surface = TTF_RenderText_Shaded(MTK_Font::get_instance()->get_font(),
				   _text,
				   MTK_Colors::get_instance()->get_foreground(),
				   MTK_Colors::get_instance()->get_background());
  
  set_width(_surface->w);
  set_height(_surface->h);
}

MTK_Label::~MTK_Label(void)
{
  free(_text);
  if (_surface)
    SDL_FreeSurface(_surface);
}

bool
MTK_Label::render(SDL_Surface *screen, bool clear_cache)
{
  if (!clear_cache)
    if (!is_dirty())
      return false;

  set_dirty(false);

  SDL_BlitSurface(_surface, NULL, screen, get_rect());
  return true;
}
