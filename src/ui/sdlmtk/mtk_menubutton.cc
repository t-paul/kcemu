#include <string.h>
#include <malloc.h>

#include "mtk.h"

MTK_MenuButton::MTK_MenuButton(const char *text, const char *command)
  : MTK_Button(text, command), _vbox(0)
{
}

MTK_MenuButton::~MTK_MenuButton(void)
{
}

void
MTK_MenuButton::activate(void)
{
  const char *cmd = get_command();

  _vbox.set_visible(true);
  set_dirty(true);

  if (cmd)
    MTK_Screen::get_instance()->get_callback()->menubutton_clicked(cmd, this);
}

void
MTK_MenuButton::handle_event(SDL_Event *event)
{
  bool hide = false;

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
      else
	{
	  if (!in_rect(_vbox.get_rect(), event->button.x, event->button.y))
	    hide = true;
	}
      break;
    case SDL_MOUSEBUTTONUP:
      if (get_state() == STATE_PRESSED)
	{
	  set_dirty(true);
	  set_state(STATE_HIGHLIGHT);
	  activate();
	}
      if (in_rect(_vbox.get_rect(), event->button.x, event->button.y))
	hide = true;
      break;
    }

  _vbox.handle_event(event);

  if (hide)
    _vbox.set_visible(false);
}

void
MTK_MenuButton::add(MTK_Widget *widget)
{
  _vbox.add(widget);
}

bool
MTK_MenuButton::render(SDL_Surface *screen, bool clear_cache)
{
  bool ret;

  ret = MTK_Button::render(screen, clear_cache);

  _vbox.set_x(get_x());
  _vbox.set_y(get_y() + get_height() + 4);

  ret = _vbox.render(screen, clear_cache) || ret;

  return ret;
}
