#include <string.h>
#include <malloc.h>

#include "mtk.h"

MTK_ToggleButton::MTK_ToggleButton(const char *text, const char *command, bool init)
  : MTK_Button(text, command)
{
  _on = init;

  _surface_on = TTF_RenderText_Shaded(MTK_Font::get_instance()->get_font(),
					  "(on)",
					  MTK_Colors::get_instance()->get_foreground(),
					  MTK_Colors::get_instance()->get_background());
  _surface_off = TTF_RenderText_Shaded(MTK_Font::get_instance()->get_font(),
				       "(off)",
				       MTK_Colors::get_instance()->get_foreground(),
				       MTK_Colors::get_instance()->get_background());

  _button_width = get_width();
  set_width(_button_width + _surface_off->w + 4);
}

MTK_ToggleButton::~MTK_ToggleButton(void)
{
}

void
MTK_ToggleButton::activate(void)
{
  const char *cmd = get_command();
  
  _on = !_on;

  if (cmd)
    MTK_Screen::get_instance()->get_callback()->togglebutton_clicked(cmd, this);
}

void
MTK_ToggleButton::handle_event(SDL_Event *event)
{
  if (!is_visible())
    return;

  SDL_Rect *rect = get_rect();

  switch (event->type)
    {
    case SDL_MOUSEMOTION:
      if (!in_rect(rect, event->motion.x, event->motion.y))
	{
	  if (get_state() != STATE_NORMAL)
	    {
	      set_dirty(true);
	      set_state(STATE_NORMAL);
	    }
	}
      else
	{
	  if (get_state() == STATE_NORMAL)
	    {
	      set_dirty(true);
	      set_state(STATE_HIGHLIGHT);
	    }
	}
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (get_state() == STATE_HIGHLIGHT)
	{
	  set_dirty(true);
	  set_state(STATE_PRESSED);
	}
      break;
    case SDL_MOUSEBUTTONUP:
      if (get_state() == STATE_PRESSED)
	{
	  set_dirty(true);
	  set_state(STATE_HIGHLIGHT);
	  activate();
	}
      break;
    }
}

bool
MTK_ToggleButton::render(SDL_Surface *screen, bool clear_cache)
{
  if (!clear_cache)
    if (!is_dirty())
      return false;

  MTK_Button::render(screen, clear_cache);
  if (is_visible())
    {
      SDL_Rect rect = *get_rect();
      rect.x += _button_width;

      if (_on)
	{
	  rect.w = _surface_on->w;
	  rect.h = _surface_on->h;
	  SDL_BlitSurface(_surface_on, NULL, screen, &rect);
	}
      else
	{
	  rect.w = _surface_off->w;
	  rect.h = _surface_off->h;
	  SDL_BlitSurface(_surface_off, NULL, screen, &rect);
	}
    }

  return true;
}
