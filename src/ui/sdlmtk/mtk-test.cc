#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "mtk.h"

class MTK_UICallback : public MTK_Callback
{
  void button_clicked(const char *command, MTK_Button *button)
  {
    printf("button clicked: %s\n", command);
    if (command && (strcmp(command, "quit") == 0))
      exit(0);
  }
  void menubutton_clicked(const char *command, MTK_MenuButton *button)
  {
    printf("menu button clicked: %s\n", command);
  }
  void togglebutton_clicked(const char *command, MTK_ToggleButton *button)
  {
    printf("toggle button clicked: %s\n", command);
  }
};

static MTK_UICallback __ui_callback;

int
main(int argc, char **argv)
{
  int done;
  SDL_Event event;
  SDL_Surface *screen;
  SDL_Surface *overlay, *bmp;
  Uint32 rmask, gmask, bmask, amask;
  
  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif


  screen = NULL;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
      fprintf(stderr, "Couldn't initialize SDL: %s\n",SDL_GetError());
      exit(2);
    }
  atexit(SDL_Quit);
  
  /* Initialize the TTF library */
  if (TTF_Init() < 0)
    {
      fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
      exit(2);
    }
  atexit(TTF_Quit);

  screen = SDL_SetVideoMode(320, 200, 24, SDL_HWSURFACE);
  if (screen == NULL)
    {
      fprintf(stderr, "Couldn't set 320x200 video mode: %s\n", SDL_GetError());
      exit(2);
    }

  overlay = SDL_CreateRGBSurface(SDL_HWSURFACE, 320, 200, 24, rmask, gmask, bmask, 0);
  if (overlay == NULL)
    {
      fprintf(stderr, "Couldn't create overlay: %s\n", SDL_GetError());
      exit(2);
    }

  bmp = SDL_LoadBMP("debian.bmp");
  if (overlay == NULL)
    {
      fprintf(stderr, "Couldn't load BMP: %s\n", SDL_GetError());
      exit(2);
    }

  MTK_MenuButton *menu;
  MTK_HBox *box = new MTK_HBox(6);
  menu = new MTK_MenuButton("File", "file");
  menu->add(new MTK_Button("Load", "load"));
  menu->add(new MTK_Button("Tape", "tape"));
  menu->add(new MTK_Button("Disk", "disk"));
  menu->add(new MTK_Button("Module", "module"));
  menu->add(new MTK_Button("Quit", "quit"));
  box->add(menu);

  menu = new MTK_MenuButton("View", "view");
  menu->add(new MTK_ToggleButton("Info", "info"));
  menu->add(new MTK_Button("Statusbar", "status"));
  menu->add(new MTK_Button("Menu", "menu"));
  menu->add(new MTK_Button("Blah", "blah"));
  menu->add(new MTK_Button("Blubb", "blubb"));
  box->add(menu);

  box->add(new MTK_Button("Continue", "cont"));
  
  box->add(new MTK_Button("Help", "help"));
  box->set_x(6);
  box->set_y(2);

  MTK_Screen::get_instance()->set_callback(&__ui_callback);

  MTK_Screen::get_instance()->add(box);
  MTK_Screen::get_instance()->set_visible(true);

  SDL_SetAlpha(overlay, SDL_SRCALPHA, 180);
  MTK_Screen::get_instance()->blit(screen, bmp, overlay, true);

  done = 0;
  while (!done)
    {
      if (SDL_WaitEvent(&event) < 0)
	continue;

      switch (event.type)
	{
	case SDL_KEYDOWN:
	case SDL_QUIT:
	  done = 1;
	  break;
	}

      MTK_Screen *scr = MTK_Screen::get_instance();

      scr->handle_event(&event);
      if (scr->render(overlay))
	scr->blit(screen, bmp, overlay);
    }

  return 0;
}
