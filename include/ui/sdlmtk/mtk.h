#include <list>

#include <SDL.h>
#include <SDL_ttf.h>

class MTK_Button;
class MTK_MenuButton;
class MTK_ToggleButton;

class MTK_Callback
{
 public:
  virtual void button_clicked(const char *command, MTK_Button *button) = 0;
  virtual void menubutton_clicked(const char *command, MTK_MenuButton *button) = 0;
  virtual void togglebutton_clicked(const char *command, MTK_ToggleButton *button) = 0;
};

class MTK_DummyCallback : public MTK_Callback
{
  virtual void button_clicked(const char *command, MTK_Button *button) {}
  virtual void menubutton_clicked(const char *command, MTK_MenuButton *button) {}
  virtual void togglebutton_clicked(const char *command, MTK_ToggleButton *button) {}
};

class MTK_Colors
{
 private:
  static MTK_Colors *_self;

  SDL_Color _col[4];

 public:
  MTK_Colors(void);
  virtual ~MTK_Colors(void);
  
  static MTK_Colors * get_instance(void);

  SDL_Color get_foreground(void);
  SDL_Color get_background(void);
  SDL_Color get_highlight(void);
  SDL_Color get_pressed(void);
};


class MTK_Font
{
 private:
  static MTK_Font *_self;
  static TTF_Font *_font;
  static const char *_fontfile;

 public:
  MTK_Font(void);
  virtual ~MTK_Font(void);
  
  static MTK_Font * get_instance(void);

  TTF_Font * get_font(void);
};

class MTK_Widget
{
 private:
  bool _dirty;
  bool _visible;
  SDL_Rect _rect;
  MTK_Widget *_parent;

 protected:
  typedef enum {
    STATE_NORMAL = 0,
    STATE_HIGHLIGHT = 1,
    STATE_PRESSED   = 2,
  } state_t;

 protected:
  virtual bool in_rect(SDL_Rect *rect, int x, int y);

 public:
  MTK_Widget(void);
  virtual ~MTK_Widget(void);

  virtual SDL_Rect * get_rect(void);
  virtual int get_x(void);
  virtual void set_x(int x);
  virtual int get_y(void);
  virtual void set_y(int y);
  virtual int get_width(void);
  virtual void set_width(int width);
  virtual int get_height(void);
  virtual void set_height(int height);
  virtual void set_rect(int x, int y, int width, int height);

  virtual bool is_dirty(void);
  virtual void set_dirty(bool dirty);

  virtual bool is_visible(void);
  virtual void set_visible(bool visible);

  virtual MTK_Widget * get_parent(void);
  virtual void set_parent(MTK_Widget *widget);

  virtual void handle_event(SDL_Event *event);

  virtual bool render(SDL_Surface *screen, bool clear_cache = false) = 0;
};

typedef std::list<MTK_Widget *> widget_list_t;

class MTK_Container : public MTK_Widget
{
 private:
  widget_list_t _children;

 public:
  MTK_Container(void);
  virtual ~MTK_Container(void);

  virtual void set_visible(bool visible);

  virtual void add(MTK_Widget *widget);
  virtual bool render(SDL_Surface *screen, bool clear_cache = false);
  virtual void handle_event(SDL_Event *event);
  virtual void process_children(MTK_Container *container);

  virtual void process_child(MTK_Widget *widget) = 0;
  virtual void layout(void) = 0;
};

class MTK_Screen : public MTK_Container
{
 private:
  static MTK_Screen *_self;

  bool _blit;
  SDL_Rect _rect;
  MTK_Callback *_callback;
  MTK_DummyCallback _dummy_callback;

 public:
  MTK_Screen(void);
  virtual ~MTK_Screen(void);
  
  static MTK_Screen * get_instance(void);

  virtual void add_rect(SDL_Rect *rect);
  virtual void blit(SDL_Surface *screen, SDL_Surface *background, SDL_Surface *ui, bool clear_cache = false);

  virtual void layout(void);
  virtual void process_child(MTK_Widget *widget);

  virtual MTK_Callback * get_callback(void);
  virtual void set_callback(MTK_Callback *callback);
};

class MTK_HBox : public MTK_Container
{
 private:
  int _x, _height, _spacing;

 public:
  MTK_HBox(int spacing = 0);
  virtual ~MTK_HBox(void);

  virtual void layout(void);
  virtual void process_child(MTK_Widget *widget);
};

class MTK_VBox : public MTK_Container
{
 private:
  int _y, _width, _spacing;

 public:
  MTK_VBox(int spacing = 0);
  virtual ~MTK_VBox(void);

  virtual void layout(void);
  virtual void process_child(MTK_Widget *widget);
};

class MTK_Label : public MTK_Widget
{
 private:
  char *_text;
  SDL_Surface *_surface;

 public:
  MTK_Label(const char *text);
  virtual ~MTK_Label(void);

  virtual bool render(SDL_Surface *screen, bool clear_cache = false);
};

class MTK_Button : public MTK_Widget
{
 private:
  char *_text;
  char *_command;
  state_t _state;
  SDL_Surface *_surface_normal;
  SDL_Surface *_surface_highlight;
  SDL_Surface *_surface_pressed;

 public:
  MTK_Button(const char *text, const char *command = NULL);
  virtual ~MTK_Button(void);

  virtual const char * get_text(void);
  virtual const char * get_command(void);

  virtual state_t get_state(void);
  virtual void set_state(state_t state);

  virtual void activate(void);
  virtual void handle_event(SDL_Event *event);

  virtual bool render(SDL_Surface *screen, bool clear_cache = false);
};

class MTK_ToggleButton : public MTK_Button
{
 private:
  bool _on;
  int _button_width;
  SDL_Surface *_surface_on;
  SDL_Surface *_surface_off;

 public:
  MTK_ToggleButton(const char *text, const char *command = NULL, bool init = false);
  virtual ~MTK_ToggleButton(void);

  virtual void activate(void);
  virtual void handle_event(SDL_Event *event);
  
  virtual bool render(SDL_Surface *screen, bool clear_cache = false);
};

class MTK_MenuButton : public MTK_Button
{
 private:
  MTK_VBox _vbox;

 public:
  MTK_MenuButton(const char *text, const char *command = NULL);
  virtual ~MTK_MenuButton(void);

  virtual void activate(void);
  virtual void handle_event(SDL_Event *event);
  
  virtual void add(MTK_Widget *widget);
  virtual bool render(SDL_Surface *screen, bool clear_cache = false);
};
