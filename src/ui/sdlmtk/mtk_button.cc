#include <string.h>
#include <malloc.h>

#include "mtk.h"

MTK_Button::MTK_Button(const char *text, const char *command)
{
  _text = strdup(text);

  if (command)
    _command = strdup(command);
  else
    _command = NULL;

  _surface_normal = TTF_RenderText_Shaded(MTK_Font::get_instance()->get_font(),
					  text,
					  MTK_Colors::get_instance()->get_foreground(),
					  MTK_Colors::get_instance()->get_background());
  _surface_highlight = TTF_RenderText_Shaded(MTK_Font::get_instance()->get_font(),
					     text,
					     MTK_Colors::get_instance()->get_highlight(),
					     MTK_Colors::get_instance()->get_background());
  _surface_pressed = TTF_RenderText_Shaded(MTK_Font::get_instance()->get_font(),
					   text,
					   MTK_Colors::get_instance()->get_pressed(),
					   MTK_Colors::get_instance()->get_background());
  
  set_state(STATE_NORMAL);
  set_width(_surface_normal->w);
  set_height(_surface_normal->h);
}

MTK_Button::~MTK_Button(void)
{
  free(_text);
  if (_command)
    free(_command);
  if (_surface_normal)
    SDL_FreeSurface(_surface_normal);
  if (_surface_highlight)
    SDL_FreeSurface(_surface_highlight);
  if (_surface_pressed)
    SDL_FreeSurface(_surface_pressed);
}

const char *
MTK_Button::get_text(void)
{
  return _text;
}

const char *
MTK_Button::get_command(void)
{
  return _command;
}

MTK_Widget::state_t
MTK_Button::get_state(void)
{
  return _state;
}

void
MTK_Button::set_state(state_t state)
{
  _state = state;
}

void
MTK_Button::activate(void)
{
  const char *cmd = get_command();

  if (cmd)
    MTK_Screen::get_instance()->get_callback()->button_clicked(cmd, this);
}

void
MTK_Button::handle_event(SDL_Event *event)
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
MTK_Button::render(SDL_Surface *screen, bool clear_cache)
{
  if (!clear_cache)
    if (!is_dirty())
      return false;

  set_dirty(false);

  SDL_Rect rect = *get_rect();

  SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
  MTK_Screen::get_instance()->add_rect(&rect);

  if (is_visible())
    {
      switch (get_state())
	{
	case STATE_NORMAL:
	  rect.w = _surface_normal->w;
	  rect.h = _surface_normal->h;
	  SDL_BlitSurface(_surface_normal, NULL, screen, &rect);
	  break;
	case STATE_HIGHLIGHT:
	  rect.w = _surface_highlight->w;
	  rect.h = _surface_highlight->h;
	  SDL_BlitSurface(_surface_highlight, NULL, screen, &rect);
	  break;
	case STATE_PRESSED:
	  rect.w = _surface_pressed->w;
	  rect.h = _surface_pressed->h;
	  SDL_BlitSurface(_surface_pressed, NULL, screen, &rect);
	  break;
	}
    }


  return true;
}
