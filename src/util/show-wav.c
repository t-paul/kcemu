#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#define PLUS (1)
#define MINUS (-1)

#define START_LONG_SYNC (2000)
#define START_SHORT_SYNC (60)

#define MAX_X (600)
#define SIZE_Y (260)

typedef unsigned char byte_t;
typedef unsigned long dword_t;

static long _pos = 0;
static FILE *_f;
static GdkGC *_gc;
static (*last_func)(GtkWidget *, char *) = 0;
static GdkColor _c_red;
static GdkColor _c_gray;
static GdkColor _c_blue;
static GdkColor _c_green;
static GdkColor _c_black;
static GdkFont *_font;
static dword_t _file_len;
static byte_t *_file_buf;
static byte_t *_info_buf;
static byte_t *_stat_buf;
static long _block_buf[255];

static int start_block = 0;
static int no_display = 1;

static long _bytes_read = 0;
static int sync_val = 0;
static int BIT_0, BIT_1, BIT_S;
typedef void (*state_func)(int bit);
static struct
{
  int sync;
  int done;
  int state;
  int bit_nr;
  int byte_nr;
  int block_nr;
  int last_bnr;
  unsigned char byte;
  unsigned char check_sum;
  char buf[128];
} state = {
  START_LONG_SYNC, 0,
};

static void
st_start(int bit)
{
  // printf("*** %s\n", __PRETTY_FUNCTION__);
  state.byte_nr = 0;
  state.check_sum = 0;
  state.state += 2;
  state.byte = 0;
  state.bit_nr = 8;
}

static void
st_wait_S(int bit)
{
  // printf("*** %s\n", __PRETTY_FUNCTION__);
  switch (bit)
    {
    case 1:
      break;
    case 2:
      state.state++;
      state.byte = 0;
      state.bit_nr = 8;
      break;
    default:
      printf("restart sync!\n");
      state.sync = START_SHORT_SYNC;
      state.state--;
    }
}

static void
handle_block(int block_nr)
{
  int a, b;
  char name[9];
  char filename[13];
  static FILE *f = NULL;

  if (block_nr == start_block)
    {
      for (a = b = 0;a < 8;a++)
        if (isprint(state.buf[a]) && !isspace(state.buf[a]))
          name[b++] = tolower(state.buf[a]);
      name[b] = '\0';
      strcpy(filename, name);
      strcat(filename, ".prg");
      f = fopen(filename, "wb");
      printf("\n%s (%s) ", name, filename);
    }
  printf("[%02x] ", state.block_nr);
  //if (state.block_nr == 0x27) no_display = 0;
  if (f)
    if (fwrite(state.buf, 128, 1, f) != 1)
      {
        printf("write error!\n");
        fclose(f);
        f = NULL;
      }
    else
      {
	fflush(f);
        printf("* <%d> ", _bytes_read);
      }
  if (block_nr == 255)
    {
      fclose(f);
      f = NULL;
      printf("\n");
    }
}

static void
st_read_byte(int bit)
{
  // printf("*** %s [%d]\n", __PRETTY_FUNCTION__, state.bit_nr);
  state.bit_nr--;
  state.byte /= 2;
  state.byte |= (bit << 7);
  if (state.bit_nr == 0)
    {
      _info_buf[_bytes_read] = state.byte;
      /* printf(" - %02x [%c]\n", state.byte & 0xff,
         isprint(state.byte) ? state.byte : '.'); */
      switch (state.byte_nr)
        {
        case 0:
          state.block_nr = state.byte;
          state.state--;
          _block_buf[state.block_nr] = _bytes_read;
          break;
        case 129:
          state.check_sum &= 0xff;
          if (state.check_sum != state.byte)
            {
              printf("checksum error [%d/ %d]!\n", state.check_sum, state.byte);
	      if (state.last_bnr == 0)
	        {
	          //state.done = 1;
		  //return;
		}
            }
          state.state++;
	  if ((state.last_bnr == 0) && (state.block_nr == 255))
	    {
	      state.done = 1;
	      return;
	    }
	  //printf("[%3d/%3d]", state.last_bnr, state.block_nr);
	  if (state.block_nr != 255)
	    if ((state.last_bnr + 1) != state.block_nr)
	      {
	        state.done = 1;
		printf("[%02x?] ", state.block_nr);
	        return;
	      }
          handle_block(state.block_nr);
	  state.last_bnr++;
          if (state.block_nr == 255)
            state.done = 1;
          break;
        default:
          state.buf[state.byte_nr - 1] = state.byte;
          state.check_sum += state.byte;
          state.state--;
          break;
        }
      state.byte_nr++;
    }
}

static void
st_end_of_block(int bit)
{
  // printf("*** %s\n", __PRETTY_FUNCTION__);
  switch (bit)
    {
    case 2:
      break;
    default:
      state.sync = START_SHORT_SYNC;
      state.state = 0;
      break;
    }
}

static state_func states[] =
{
  st_start,
  st_wait_S,
  st_read_byte,
  st_end_of_block,
};

void
handle_bit(int bit)
{
  static int old_state = -1;

  if (old_state != state.state)
    {
      // printf("\n+++ new state: %d +++\n", state.state);
    }
  //printf("[bit = %d]\n", bit);
  old_state = state.state;
  (states[state.state])(bit);
}

void
display(int c)
{
  int a;

  if (no_display) return;

  c /= 4;
  printf("\n");
  for (a = 0;a < 64;a++)
    {
      printf(" ");
      if (a == c) printf("*");
    }
}

void
handle_half_wave(int stat, int counter)
{
  static int locked = 0;
  static int old_counter;
  static int first = 1;
  
  if (state.sync > 0)
    {
      //printf("%d [%3d] - sync = %d\n", counter, stat, state.sync);
      _stat_buf[_bytes_read] = '*';
      if ((counter > (sync_val - 4)) && (counter < (sync_val + 4)))
        {
          state.sync--;
          if (state.sync == 0)
            {
              locked = 0;
	      if (first)
	        {
		  first = 0;
                  BIT_1 = 2 * sync_val;
                  BIT_0 = sync_val + sync_val / 2;
                  BIT_S = BIT_1 + sync_val / 2;
                  printf("locked with sync_val = %d [%d/%d/%d]\n",
              		sync_val, BIT_S, BIT_1, BIT_0);
	        }
              //getchar();
            }
        }
      else
        {
          sync_val = counter;
          state.sync = START_SHORT_SYNC;
        }
      return;
    }
  if (!locked)
    {
      _stat_buf[_bytes_read] = 'L';
      if (counter > (sync_val + sync_val / 3))
        {
          locked = 1;
          printf("locked with stat %d\n", stat);
          old_counter = counter;
        }
      return;
    }
  if (old_counter == 0)
    {
      old_counter = counter;
      return;
    }
  
  counter += old_counter;
  old_counter = 0;

  if (counter < BIT_0)
    {
      _stat_buf[_bytes_read] = '0';
      handle_bit(0);
    }
  else if (counter > BIT_S)
    {
      if (counter < 2 * BIT_S)
        {
          _stat_buf[_bytes_read] = 'S';
          handle_bit(2);
        }
    }
  else
    {
      _stat_buf[_bytes_read] = '1';
      handle_bit(1);
    }
}

static void
update(GtkWidget *w)
{
  byte_t c;
  char buf[100];
  int a, c1, c2;

  if (!_font)
    _font = gdk_font_load("fixed");

  gdk_gc_set_foreground(_gc, &_c_gray);
  gdk_draw_rectangle(w->window, _gc, TRUE, 0, 0, MAX_X, SIZE_Y);
  gdk_gc_set_foreground(_gc, &_c_red);
  gdk_draw_line(w->window, _gc, 0, 128, MAX_X, 128);

  fseek(_f, _pos, SEEK_SET);
  c1 = fgetc(_f);
  for (a = 1;a < MAX_X;a++)
    {
      c2 = fgetc(_f);
      gdk_gc_set_foreground(_gc, &_c_black);
      gdk_draw_line(w->window, _gc, a - 1, c1, a, c2);
      c1 = c2;
      c = _info_buf[_pos + a];
      if (c != 0)
        {
          sprintf(buf, "%02x [%c]", c, isprint(c) ? c : '.');
          gdk_gc_set_foreground(_gc, &_c_green);
          gdk_draw_string(w->window, _font, _gc, a, 40, buf);
        }
      c = _stat_buf[_pos + a];
      if (c != 0)
        {
          sprintf(buf, "%c", c);
          gdk_gc_set_foreground(_gc, &_c_blue);
          gdk_draw_string(w->window, _font, _gc, a, 25, buf);
          gdk_draw_line(w->window, _gc, a, 50, a, 138);
        }
    }

  gdk_gc_set_foreground(_gc, &_c_green);
  sprintf(buf, "pos = %ld/ %ld", _pos, _file_len);
  gdk_draw_string(w->window, _font, _gc, 10, 10, buf);
}

static void
expose(GtkWidget *w)
{
  printf("expose\n");
  update(w);
}

static void
do_goto_block(GtkWidget *w, char *s)
{
  unsigned long block;
  
  block = strtoul(s, NULL, 0);
  _pos = _block_buf[block];
}

static void
do_goto(GtkWidget *w, char *s)
{
  unsigned long pos;

  pos = strtoul(s, NULL, 0);
  printf("goto: %lu\n", pos);
  _pos = pos;
}

static void
do_right(GtkWidget *w, char *s)
{
  _pos -= 10;
  if (_pos < 0)
    _pos = 0;
}

static void
do_Right(GtkWidget *w, char *s)
{
  _pos -= (MAX_X / 2);
  if (_pos < 0)
    _pos = 0;
}

static void
do_left(GtkWidget *w, char *s)
{
  _pos += 10;
}

static void
do_Left(GtkWidget *w, char *s)
{
  _pos += (MAX_X / 2);
}

static void
activate(GtkWidget *w, gpointer data)
{
  char *ptr;
  GtkWidget *d = GTK_WIDGET(data);
  
  ptr = gtk_entry_get_text(GTK_ENTRY(w));

  switch (*ptr)
    {
    case '\0':
      break;
    case 'b': last_func = do_goto_block; break;
    case 'l': last_func = do_left; break;
    case 'L': last_func = do_Left; break;
    case 'r': last_func = do_right; break;
    case 'R': last_func = do_Right; break;
    case 'g': last_func = do_goto; break;
    case 'q': exit(0);
    default:  last_func = 0; break;
    }

  if (last_func)
    (*last_func)(d, &ptr[1]);
  update(d);
  gtk_entry_set_text(GTK_ENTRY(w), "");
}

void
create_window(void)
{
  GdkColormap *cmap;
  GtkWidget *w, *b, *d, *e;

  w = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  b = gtk_vbox_new(FALSE, 4);
  gtk_container_add(GTK_CONTAINER(w), b);
  gtk_widget_show(b);
  
  d = gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(d), MAX_X, SIZE_Y);
  gtk_box_pack_start(GTK_BOX(b), d, TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(d), "expose_event", GTK_SIGNAL_FUNC(expose), NULL);
  gtk_widget_show(d);

  e = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(b), e, FALSE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT(e), "activate", GTK_SIGNAL_FUNC(activate), d);
  gtk_widget_show(e);
  
  gtk_widget_show(w);

  gtk_widget_grab_focus(e);

  _gc = gdk_gc_new(d->window);

  cmap = gdk_colormap_get_system();
  gdk_color_parse("rgb:ff/00/00", &_c_red);
  gdk_color_parse("rgb:c0/c0/c0", &_c_gray);
  gdk_color_parse("rgb:00/00/60", &_c_blue);
  gdk_color_parse("rgb:00/60/00", &_c_green);
  gdk_color_parse("rgb:00/00/00", &_c_black);
  gdk_colormap_alloc_color(cmap, &_c_red, FALSE, TRUE);
  gdk_colormap_alloc_color(cmap, &_c_gray, FALSE, TRUE);
  gdk_colormap_alloc_color(cmap, &_c_blue, FALSE, TRUE);
  gdk_colormap_alloc_color(cmap, &_c_green, FALSE, TRUE);
  gdk_colormap_alloc_color(cmap, &_c_black, FALSE, TRUE);
}

void
do_read1(void)
{
  int c;
  int stat;
  int counter;

  stat = MINUS;
  counter = 0;

  _bytes_read = 0;
  while (242)
    {
      c = _file_buf[_bytes_read++];
      if (_bytes_read >= _file_len)
        return;
      if (state.done)
        return;
      display(c);
      c -= 128;
      
      //printf("%5d ", c);
      if (stat == PLUS)
        {
          if (c < MINUS)
            {
              stat = MINUS;
              //printf(" - %d\n", counter);
              handle_half_wave(stat, counter);
              counter = 0;
            }
        }
      else if (stat == MINUS)
        {
          if (c > PLUS)
            {
              stat = PLUS;
              //printf(" + %d\n", counter);
              handle_half_wave(stat, counter);
              counter = 0;
            }
        }
      //printf("\n");
      counter++;
    }
}

int
main(int argc, char **argv)
{
  int a, c;
  
  gtk_init(&argc, &argv);
  create_window();

  printf("argc = %d\n", argc);
  for (a = 0;a < argc;a++)
    printf("argv[%d] = %s\n", a, argv[a]);

  if (argc != 2)
    exit(1);

  _f = fopen(argv[1], "rb");
  if (_f == NULL)
    exit(2);
  
  _file_buf = malloc(3000000);
  if (!_file_buf)
    exit(3);

  _info_buf = malloc(3000000);
  if (!_info_buf)
    exit(4);

  _stat_buf = malloc(3000000);
  if (!_stat_buf)
    exit(5);

  _file_len = fread(_file_buf, 1, 3000000, _f);
  printf("%ld bytes read\n", _file_len);
  memset(_info_buf, 0, _file_len);
  memset(_stat_buf, 0, _file_len);
  memset(_block_buf, 0, sizeof(_block_buf));

  state.done = 0;
  state.last_bnr = start_block - 1;
  do_read1();
  
  gtk_main();

  fclose(_f);
}
