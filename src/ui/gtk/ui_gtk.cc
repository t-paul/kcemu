/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *  Copyright (C) 1997-2010 Torsten Paul
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <iostream>

#include <iomanip>

#include <signal.h> /* FIXME: only for testing */
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "kc/system.h"
#include "kc/prefs/types.h"
#include "kc/prefs/prefs.h"

#include "kc/z80.h"
#include "kc/keys.h"
#include "kc/memory.h" // text_update()
#include "kc/keyboard.h"

#include "sys/sysdep.h"

#include "cmd/cmd.h"

#include "ui/error.h"
#include "ui/gtk/cmd.h"

#include "ui/gtk/debug.h"
#include "ui/gtk/ui_gtk.h"

#include "ui/gtk/wav.h"
#include "ui/gtk/main.h"
#include "ui/gtk/help.h"
#include "ui/gtk/tape.h"
#include "ui/gtk/disk.h"
#include "ui/gtk/vdip.h"
#include "ui/gtk/info.h"
#include "ui/gtk/about.h"
#include "ui/gtk/debug.h"
#include "ui/gtk/color.h"
#include "ui/gtk/thanks.h"
#include "ui/gtk/module.h"
#include "ui/gtk/keyboard.h"
#include "ui/gtk/copying.h"
#include "ui/gtk/tapeadd.h"
#include "ui/gtk/options.h"
#include "ui/gtk/selector.h"
#include "ui/gtk/savemem.h"
#include "ui/gtk/screenshot.h"
#include "ui/gtk/plotter.h"
#include "ui/gtk/video.h"

#include "ui/gtk/ui_gtk0.h"
#include "ui/gtk/ui_gtk1.h"
#include "ui/gtk/ui_gtk3.h"
#include "ui/gtk/ui_gtk4.h"
#include "ui/gtk/ui_gtk6.h"
#include "ui/gtk/ui_gtk8.h"
#include "ui/gtk/ui_gtk9.h"
#include "ui/gtk/ui_gtk_kramermc.h"
#include "ui/gtk/ui_gtk_muglerpc.h"
#include "ui/gtk/ui_gtk_vcs80.h"
#include "ui/gtk/ui_gtk_c80.h"

#include "ui/generic/ui_0.h"


#include "libdbg/dbg.h"

using namespace std;

static UI_Gtk *self;

class CMD_ui_toggle : public CMD {
private:
    UI_Gtk *_w;
    
public:
    CMD_ui_toggle(UI_Gtk *w) : CMD("ui-toggle") {
        _w = w;
        register_cmd("ui-speed-limit-toggle", 0);
        register_cmd("ui-zoom-1", 1);
        register_cmd("ui-zoom-2", 2);
        register_cmd("ui-zoom-3", 3);
        register_cmd("ui-display-effects-toggle", 4);
    }
    
    void execute(CMD_Args *args, CMD_Context context) {
        switch (context) {
            case 0:
                _w->speed_limit_toggle();
                break;
            case 1:
            case 2:
            case 3:
                _w->gtk_zoom(context);
                break;
            case 4:
                _w->display_effects_toggle();
                break;
        }
    }
};

class CMD_update_colortable : public CMD {
private:
    UI_Gtk *_ui;
    ColorWindow *_colwin;
    
public:
    CMD_update_colortable(UI_Gtk *ui, ColorWindow *colwin) : CMD("ui-update-colortable") {
        _ui = ui;
        _colwin = colwin;
        register_cmd("ui-update-colortable");
    }
    
    void execute(CMD_Args *args, CMD_Context context) {
        _ui->allocate_colors(_colwin->get_brightness_fg(), _colwin->get_contrast_fg(), _colwin->get_brightness_bg(), _colwin->get_contrast_bg());
        _ui->update(true, true);
    }
};

void
UI_Gtk::idle(void) {
    gtk_main_quit();
}

void
UI_Gtk::sf_selection_received(GtkWidget *widget,
        GtkSelectionData *sel_data,
        gpointer *data) {
    GdkAtom atom;
    UI_Gtk *self;
    CMD_Args *args;
    
    self = (UI_Gtk *)data;
    if (sel_data->length < 0)
        return;
    
    atom = gdk_atom_intern("TEXT", FALSE);
    if (atom == GDK_NONE)
        return;
    
    /*
     *  may check sel_data->type here...
     */
    
    // printf("selection (%d bytes) = %s\n", sel_data->length, sel_data->data);
    args = new CMD_Args();
    args->set_string_arg("text", (const char *)sel_data->data);
    CMD_EXEC_ARGS("keyboard-replay", args);
}

void
UI_Gtk::key_press_release(GdkEventKey *event, bool press) {
    int c = 0;
    int key_code;
    
    key_code = event->hardware_keycode;
    
    switch (event->keyval) {
        case GDK_Alt_L:
        case GDK_Alt_R:
        case GDK_Meta_L:
        case GDK_Meta_R:
        case GDK_Super_L:
        case GDK_Super_R:
        case GDK_Hyper_L:
        case GDK_Hyper_R:          c = KC_KEY_ALT;     break;
        case GDK_ISO_Level3_Shift:
        case GDK_Mode_switch:      c = KC_KEY_ALT_GR;  break;
        case GDK_Shift_L:
        case GDK_Shift_R:          c = KC_KEY_SHIFT;   break;
        case GDK_Control_L:
        case GDK_Control_R:        c = KC_KEY_CONTROL; break;
        case GDK_Left:
        case GDK_KP_Left:          c = KC_KEY_LEFT;    break;
        case GDK_Right:
        case GDK_KP_Right:         c = KC_KEY_RIGHT;   break;
        case GDK_Up:
        case GDK_KP_Up:            c = KC_KEY_UP;      break;
        case GDK_Down:
        case GDK_KP_Down:          c = KC_KEY_DOWN;    break;
        case GDK_Escape:           c = KC_KEY_ESC;     break;
        case GDK_Home:             c = KC_KEY_HOME;    break;
        case GDK_End:              c = KC_KEY_END;     break;
        case GDK_Pause:            c = KC_KEY_PAUSE;   break;
        case GDK_Print:            c = KC_KEY_PRINT;   break;
        case GDK_Delete:
        case GDK_KP_Delete:        c = KC_KEY_DEL;     break;
        case GDK_Insert:
        case GDK_KP_Insert:        c = KC_KEY_INSERT;  break;
        case GDK_Page_Up:
        case GDK_KP_Page_Up:       c = KC_KEY_PAGE_UP; break;
        case GDK_Page_Down:
        case GDK_KP_Page_Down:     c = KC_KEY_PAGE_DOWN; break;
        case GDK_F1:
        case GDK_KP_F1:            c = KC_KEY_F1;      break;
        case GDK_F2:
        case GDK_KP_F2:            c = KC_KEY_F2;      break;
        case GDK_F3:
        case GDK_KP_F3:            c = KC_KEY_F3;      break;
        case GDK_F4:
        case GDK_KP_F4:            c = KC_KEY_F4;      break;
        case GDK_F5:               c = KC_KEY_F5;      break;
        case GDK_F6:               c = KC_KEY_F6;      break;
        case GDK_F7:               c = KC_KEY_F7;      break;
        case GDK_F8:               c = KC_KEY_F8;      break;
        case GDK_F9:               c = KC_KEY_F9;      break;
        case GDK_F10:              c = KC_KEY_F10;     break;
        case GDK_F11:              c = KC_KEY_F11;     break;
        case GDK_F12:              c = KC_KEY_F12;     break;
        case GDK_F13:              c = KC_KEY_F13;     break;
        case GDK_F14:              c = KC_KEY_F14;     break;
        case GDK_F15:              c = KC_KEY_F15;     break;
        case GDK_KP_0:             c = '0';            break;
        case GDK_KP_1:             c = '1';            break;
        case GDK_KP_2:             c = '2';            break;
        case GDK_KP_3:             c = '3';            break;
        case GDK_KP_4:             c = '4';            break;
        case GDK_KP_5:             c = '5';            break;
        case GDK_KP_6:             c = '6';            break;
        case GDK_KP_7:             c = '7';            break;
        case GDK_KP_8:             c = '8';            break;
        case GDK_KP_9:             c = '9';            break;
        case GDK_KP_Equal:         c = '=';            break;
        case GDK_KP_Multiply:      c = '*';            break;
        case GDK_KP_Add:           c = '+';            break;
        case GDK_KP_Subtract:      c = '-';            break;
        case GDK_KP_Divide:        c = '/';            break;
        case GDK_KP_Enter:         c = 0x0d;           break;
        case GDK_dead_circumflex:  c = '^';            break;
        case GDK_dead_acute:       c = '\'';           break;
        case GDK_dead_grave:       c = '\'';           break;
        default:
            c = event->keyval & 0xff;
            break;
    }
    
    DBG(2, form("KCemu/UI/key_kc",
            "%s - keyval = %5d/0x%04x / keycode = %5d -> kccode = %5d/0x%04x\n",
            press ? "press  " : "release",
            event->keyval, event->keyval, key_code, c, c));
    
    if (press)
        keyboard->keyPressed(c, key_code);
    else
        keyboard->keyReleased(c, key_code);
}

gboolean
UI_Gtk::sf_key_press(GtkWidget */*widget*/, GdkEventKey *event) {
    DBG(2, form("KCemu/UI/key_press",
            "key_press:   keyval = %04x, keycode = %04x\n",
            event->keyval, event->hardware_keycode));
    
    /*
     *  Don't handle the key event if the ALT modifier
     *  is set. This allows for better handling of the
     *  accelerator key in the menu bar.
     */
    if ((event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK)
        return FALSE;
    
    key_press_release(event, true);
    return TRUE;
}

gboolean
UI_Gtk::sf_key_release(GtkWidget */*widget*/, GdkEventKey *event) {
    DBG(2, form("KCemu/UI/key_release",
            "key_release: keyval = %04x, keycode = %04x\n",
            event->keyval, event->hardware_keycode));
    
    /*
     *  Don't handle the key event if the ALT modifier
     *  is set. This allows for better handling of the
     *  accelerator key in the menu bar.
     */
    if ((event->state & GDK_MOD1_MASK) == GDK_MOD1_MASK)
        return FALSE;
    
    key_press_release(event, false);
    return TRUE;
}

void
UI_Gtk::sf_focus_in(GtkWidget * /* widget */, GdkEventFocus *event) {
    DBG(2, form("KCemu/UI/focus_in",
            "got focus\n"));
    
    keyboard->keyReleased(-1, -1);
}

void
UI_Gtk::sf_focus_out(GtkWidget *widget, GdkEventFocus *event) {
    DBG(2, form("KCemu/UI/focus_out",
            "lost focus\n"));
    
    keyboard->keyReleased(-1, -1);
}

void
UI_Gtk::sf_leave_notify(GtkWidget *widget, GdkEventCrossing *event) {
    keyboard->keyReleased(-1, -1);
}

void
UI_Gtk::speed_limit_toggle(void) {
    _speed_limit = !_speed_limit;
}

void
UI_Gtk::text_update(void) {
#if 0
    static GdkFont *font = 0;
    unsigned long val = 0;
    
    if (font == 0)
        font = gdk_font_load("fixed");
    
    byte_t *irm = memory->getIRM();
    for (int a = 0;a < 8;a++) {
        val |= (irm[40 * a + 320] & 0xf);
        val <<= 4;
    }
    
    char buf[100];
    snprintf(buf, 100, "%08x", val);
    gdk_draw_string(_text.canvas->window, font, _gc, 10, 10, buf);
#endif
}

void
UI_Gtk::gtk_sync(void) {
    static int count = 0;
    static bool first = true;
    static long tv_sec, tv_usec;
    static long tv1_sec = 0, tv1_usec = 0;
    static long tv2_sec, tv2_usec;
    static unsigned long frame = 25;
    static unsigned long long base, d2;
    static long basetime_sec = 0, basetime_usec = 0;
    
    unsigned long timeframe, diff, fps;
    
    if (++count >= 60) {
        count = 0;
        sys_gettimeofday(&tv2_sec, &tv2_usec);
        diff = ((1000000 * (tv2_sec - tv1_sec)) + (tv2_usec - tv1_usec));
        fps = 60500000 / diff;
        _main_window->set_fps(fps);
        tv1_sec = tv2_sec;
        tv1_usec = tv2_usec;
    }
    
    if (first) {
        first = false;
        sys_gettimeofday(&tv1_sec, &tv1_usec);
        sys_gettimeofday(&basetime_sec, &basetime_usec);
        base = (basetime_sec * 50) + basetime_usec / 20000;
        base -= 26; // see comment below
    }
    
    sys_gettimeofday(&tv_sec, &tv_usec);
    d2 = (tv_sec * 50) + tv_usec / 20000;
    timeframe = (unsigned long)(d2 - base);
    frame++;
    
    /*
     *  because of this test we start with frame = 25 otherwise it
     *  would fail due to the fact that timeframe is unsigned!
     */
    if (frame < (timeframe - 20)) {
        DBG(2, form("KCemu/UI/update",
                "counter = %lu, frame = %lu, timeframe = %lu\n",
                (unsigned long)z80->getCounter(), frame, timeframe));
        frame = timeframe;
    }
    
    if (_speed_limit) {
        if (frame > (timeframe + 1)) {
            sys_usleep(20000 * (frame - timeframe - 1));
        }
    }
    else {
        frame = timeframe;
    }
    
    /*
     * if (!_auto_skip)
     * {
     * processEvents();
     * update();
     * }
     */
    
    sys_gettimeofday(&tv_sec, &tv_usec);
    d2 = (tv_sec * 50) + tv_usec / 20000;
    timeframe = (unsigned long)(d2 - base);
    _auto_skip = false;
    
    if (frame < timeframe) {
        if (++_cur_auto_skip > _max_auto_skip)
            _cur_auto_skip = 0;
        else
            _auto_skip = true;
    }
}

UI_Gtk::UI_Gtk(void) {
    _ui = 0;
    _init = false;
    _video_encoder_state = VideoEncoder::VIDEO_ENCODER_STATE_STOP;
    _video_encoder = _video_encoder_dummy = new DummyVideoEncoder();
}

UI_Gtk::~UI_Gtk(void) {
    delete _about_window;
    delete _help_window;
    delete _thanks_window;
    delete _color_window;
    delete _tape_window;
    delete _tape_add_window;
    delete _disk_window;
    delete _vdip_window;
    delete _save_memory_window;
    delete _module_window;
    delete _keyboard_window;
    delete _copying_window;
    delete _options_window;
    delete _selector_window;
    delete _debug_window;
    delete _info_window;
    delete _wav_window;
    delete _screenshot_window;
    delete _plotter_window;
    delete _video_window;
    delete _edit_header_window;
    delete _dialog_window;
    delete _file_browser;
    delete _main_window;
}

void
UI_Gtk::init(int *argc, char ***argv) {
    self = this;
    _shift_lock = false;
    _speed_limit = true;
    
    _auto_skip = false;
    _cur_auto_skip = 0;
    _max_auto_skip = 6;
    
#ifdef ENABLE_NLS
    /*
     *  We need to get all text in UTF-8 because this is required
     *  for GTK versions above 2.0 (more precisely it's required by
     *  the Pango library).
     *
     *  And we do it here because this way it's possible to get the
     *  help/usage messages still in the default locale of the user.
     *  From this point all messages that are sent to the console are
     *  encoded in UTF-8 too.
     */
    bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif /* ENABLE_NLS */
    
    gtk_init(argc, argv);
    
    string datadir(kcemu_datadir);
    
    /*
     *  load gtk ressource files
     */
    string rc_filename = datadir + "/.kcemurc.gtk";
    gtk_rc_parse(rc_filename.c_str());
    
    const char *tmp = kcemu_homedir;
    if (tmp) {
        string homedir(tmp);
        string home_rc_filename = homedir + "/.kcemurc.gtk";
        gtk_rc_parse(home_rc_filename.c_str());
    }
    else
        cerr << "Warning: HOME not set! can't locate file `.kcemurc.gtk'" << endl;
    
    /*
     *  don't let gtk catch SIGSEGV, make core dumps ;-)
     */
    signal(SIGSEGV, SIG_DFL);
    
    Error::instance()->addErrorListener(this);
    
    _dialog_window      = new DialogWindow("dialog.glade");
    _selector_window    = new ProfileSelectorWindow("selector.glade");
    _help_window        = new HelpWindow("help.glade");
}

void
UI_Gtk::init2(void) {
    create_ui();
    _main_window        = new MainWindow("main.glade");
    _about_window       = new AboutWindow("about.glade");
    _thanks_window      = new ThanksWindow("thanks.glade");
    _color_window       = new ColorWindow("color.glade");
    _tape_window        = new TapeWindow("tape.glade");
    _tape_add_window    = new TapeAddWindow("tapeedit.glade");
    _disk_window        = new DiskWindow("disk.glade");
    _vdip_window        = new VDIPWindow("vdip.glade");
    _module_window      = new ModuleWindow("module.glade");
    _keyboard_window    = new KeyboardWindow("keyboard.glade");
    _copying_window     = new CopyingWindow("legal.glade");
    _options_window     = new OptionsWindow("options.glade");
    _wav_window         = new WavWindow("audio.glade");
    _screenshot_window  = new ScreenshotWindow("screenshot.glade", this);
    _plotter_window     = new PlotterWindow("plotter.glade");
    _save_memory_window = new SaveMemoryWindow("savemem.glade");
    _video_window       = new VideoWindow("video.glade", this);
    
    _edit_header_window = new EditHeaderWindow();
    _file_browser       = new FileBrowser();
    
    _debug_window       = new DebugWindow();
    _info_window        = new InfoWindow();
    
    ColorWindow *color_window = (ColorWindow *)_color_window;
    
    allocate_colors(color_window->get_brightness_fg(), color_window->get_contrast_fg(), color_window->get_brightness_bg(), color_window->get_contrast_bg());
    
    /* this _must_ come last due to some initialization for menus */
    _main_window->show(get_width(), get_height());
    
    CMD *cmd;
    cmd = new CMD_ui_toggle(this);
    cmd = new CMD_update_colortable(this, color_window);
    
    GtkWidget *main_window = _main_window->get_main_window();
    g_signal_connect(main_window, "selection_received", G_CALLBACK(sf_selection_received), this);
    g_signal_connect(main_window, "key_press_event", G_CALLBACK(sf_key_press), this);
    g_signal_connect(main_window, "key_release_event", G_CALLBACK(sf_key_release), this);
    g_signal_connect(main_window, "focus_in_event", G_CALLBACK(sf_focus_in), this);
    g_signal_connect(main_window, "focus_out_event", G_CALLBACK(sf_focus_out), this);
    
//    static GtkTargetEntry targetlist[] = {
//        { "STRING",        0, 1 },
//        { "TEXT",          0, 2 },
//        { "COMPOUND_TEXT", 0, 3 }
//    };
//    gint ntargets  = sizeof(targetlist) / sizeof(targetlist[0]);
//    gint nentries  = sizeof(entries)    / sizeof(entries[0]);
//    gint nentriesP = sizeof(entriesP)   / sizeof(entriesP[0]);
//
//    g_selection_add_targets(_main.window, GDK_SELECTION_PRIMARY, targetlist, ntargets);
}

void
UI_Gtk::gtk_resize(void) {
    if (_main_window->resize(get_width(), get_height()))
      set_video_encoder_state(VideoEncoder::VIDEO_ENCODER_STATE_STOP);
}

int
UI_Gtk::ensure_range(double val)
{
  if (val < 0)
    return 0;
  if (val > 255)
    return 255;
  return (int)val;
}

void
UI_Gtk::allocate_colors(double brightness_fg, double contrast_fg, double brightness_bg, double contrast_bg)
{
  int idx = 0;
  list<UI_Color> colors(_ui->get_colors());
  for (list<UI_Color>::const_iterator it = colors.begin();it != colors.end();it++, idx++)
    {
      double r, g, b;
      if ((*it).is_rgb())
        {
          r = (*it).get_red();
          g = (*it).get_green();
          b = (*it).get_blue();
        }
      else
        {
          int red, green, blue;
          //double s = (*it).is_bg() ? 0.85 : 1.0;
          //double v = (*it).is_bg() ? 0.70 : 1.0;
          hsv2rgb((*it).get_hue(), 1.0, 1.0, &red, &green, &blue);
          r = red;
          g = green;
          b = blue;
        }

      double contrast;
      double brightness;
      if ((*it).is_bg())
        {
          contrast = contrast_bg;
          brightness = brightness_bg;
        }
      else
        {
          contrast = contrast_fg;
          brightness  = brightness_fg;
        }

      r *= 1.5 * contrast;
      g *= 1.5 * contrast;
      b *= 1.5 * contrast;
      r += (256 * brightness - 127);
      g += (256 * brightness - 127);
      b += (256 * brightness - 127);

      int red = ensure_range(r);
      int green = ensure_range(g);
      int blue = ensure_range(b);

      DBG(2, form("KCemu/UI/color",
                  "%c(%3d): %5.2f / %5.2f - %7.2f, %7.2f, %7.2f - %3d, %3d, %3d\n",
                  (*it).is_bg() ? 'B' : 'F',
                  idx, brightness, contrast,
                  r, g, b,
                  red, green, blue));

      _main_window->allocate_color_rgb(idx, red, green, blue);
      _video_encoder->allocate_color_rgb(idx, red, green, blue);
    }
}

void
UI_Gtk::set_video_encoder(VideoEncoder *encoder)
{
  if (_video_encoder == encoder)
    return;

  _video_encoder->close();
  _video_encoder = (encoder == NULL) ? _video_encoder_dummy : encoder;
}

void
UI_Gtk::set_video_encoder_state(int state)
{
  if (state == _video_encoder_state)
    return;

  if (state == VideoEncoder::VIDEO_ENCODER_STATE_PAUSE)
    {
      _video_encoder_state = VideoEncoder::VIDEO_ENCODER_STATE_PAUSE;
      CMD_EXEC("ui-video-pause");
    }

  if (state == VideoEncoder::VIDEO_ENCODER_STATE_STOP)
    {
      if (_video_encoder_state == VideoEncoder::VIDEO_ENCODER_STATE_RECORD)
        _video_encoder->close();
      _video_encoder_state = VideoEncoder::VIDEO_ENCODER_STATE_STOP;
      CMD_EXEC("ui-video-stop");
    }

  if (state == VideoEncoder::VIDEO_ENCODER_STATE_RECORD)
    {
      UI_Base *ui = _ui->get_generic_ui();
      int width = _ui->get_generic_ui()->get_real_width();
      int height = _ui->get_generic_ui()->get_real_height();
      if (_video_encoder->init(_video_encoder_filename, width, height, _video_encoder_frame_skip, _video_encoder_quality))
        {
          // cheat a bit by forcing color table update and repaint
          CMD_EXEC("ui-update-colortable");
          _video_encoder->encode(ui->get_buffer(), NULL);
          _video_encoder_state = VideoEncoder::VIDEO_ENCODER_STATE_RECORD;
          _video_frame = _video_skip = _video_encoder_frame_skip;
          CMD_EXEC("ui-video-record");
        }
      else
        {
          _video_encoder_state = VideoEncoder::VIDEO_ENCODER_STATE_STOP;
          CMD_EXEC("ui-video-stop");
        }
    }
}

void
UI_Gtk::set_video_encoder_config(const char *filename, double quality, int frame_skip, bool start_on_reset)
{
  _video_encoder_filename = filename;
  _video_encoder_quality = quality;
  _video_encoder_frame_skip = frame_skip;
  _video_encoder_start_on_reset = start_on_reset;
}

void
UI_Gtk::show(void) {
    _init = true;
    gtk_resize();
}

void
UI_Gtk::gtk_zoom(int zoom) {
    if (zoom < 1)
        zoom = 1;
    if (zoom > 3)
        zoom = 3;
    
    kcemu_ui_scale = zoom;
    
    gtk_resize();
    update(true, true);
}

void
UI_Gtk::processEvents(void) {
    _main_window->process_events();
    
    while (gtk_events_pending())
        gtk_main_iteration();
}

void
UI_Gtk::display_effects_toggle(void) {
    _main_window->set_display_effect(!_main_window->get_display_effect());
    update(true, true);
}

void
UI_Gtk::update(bool full_update, bool clear_cache) {
    Scanline *scanline = _ui->get_scanline();
    if (scanline != NULL) {
        scanline->update();
    }
    
    MemAccess *memaccess = _ui->get_mem_access();
    if (memaccess != NULL) {
        memaccess->update();
    }

    UI_Base *ui = _ui->get_generic_ui();
    ui->generic_update(scanline, memaccess, clear_cache);
    _main_window->update(ui, get_width(), get_height(), full_update);
    if (_video_encoder_state == VideoEncoder::VIDEO_ENCODER_STATE_RECORD)
      {
        if (_video_frame <= 0)
          {
            _video_frame = _video_skip;
            // we can use the dirty buffer only when not skipping frames
            _video_encoder->encode(ui->get_buffer(), _video_skip == 1 ? ui->get_dirty_buffer() : NULL);
          }
        _video_frame--;
      }
    memset(ui->get_dirty_buffer(), 0, ui->get_dirty_buffer_size());
    processEvents();
    gtk_sync();
}

UI_ModuleInterface *
UI_Gtk::getModuleInterface(void) {
    return (ModuleWindow *)_module_window;
}

TapeInterface *
UI_Gtk::getTapeInterface(void) {
    return (TapeWindow *)_tape_window;
}

DebugInterface *
UI_Gtk::getDebugInterface(void) {
    return (DebugWindow *)_debug_window;
}

void
UI_Gtk::errorInfo(const char *msg) {
    _dialog_window->show_dialog_ok(_("Info"), msg);
}

GdkPixbuf *
UI_Gtk::get_screenshot(void)
{
  UI_Base *ui = _ui->get_generic_ui();
  GdkColor *colormap = _main_window->get_colormap();

  int width = ui->get_real_width();
  int height = ui->get_real_height();
  int count = width * height;

  byte_t *image = ui->get_buffer();
  byte_t *buf = new byte_t[3 * count];

  for (int src = 0, dst = 0;src < count;src++)
    {
      GdkColor col = colormap[image[src]];
      buf[dst++] = col.red >> 8;
      buf[dst++] = col.green >> 8;
      buf[dst++] = col.blue >> 8;
    }

  return gdk_pixbuf_new_from_data(buf, GDK_COLORSPACE_RGB, FALSE, 8, width, height, 3 * width, on_pixbuf_destroy, NULL);
}

void
UI_Gtk::on_pixbuf_destroy(guchar *pixels, gpointer user_data)
{
  delete[] pixels;
}

char *
UI_Gtk::select_profile(void) {
    _selector_window->show();
    return ((ProfileSelectorWindow *)_selector_window)->get_selected_profile();
}

void
UI_Gtk::create_ui(void) {
    const SystemType *system_type = Preferences::instance()->get_system_type();

    _callback_value = system_type->get_ui_callback_value();
    _callback_value_retrace = system_type->get_ui_callback_retrace_value();

    switch (Preferences::instance()->get_kc_type()) {
        case KC_TYPE_85_1:
        case KC_TYPE_87:
            _ui = new UI_Gtk1();
            break;
        case KC_TYPE_85_2:
        case KC_TYPE_85_3:
            _ui = new UI_Gtk3();
            break;
        case KC_TYPE_85_4:
        case KC_TYPE_85_5:
            _ui = new UI_Gtk4();
            break;
        case KC_TYPE_LC80:
            _ui = new UI_Gtk8();
            break;
        case KC_TYPE_Z1013:
            _ui = new UI_Gtk0();
            break;
        case KC_TYPE_A5105:
            _ui = new UI_Gtk9();
            break;
        case KC_TYPE_POLY880:
            _ui = new UI_Gtk6();
            break;
        case KC_TYPE_KRAMERMC:
            _ui = new UI_Gtk_KramerMC();
            break;
        case KC_TYPE_MUGLERPC:
            _ui = new UI_Gtk_MuglerPC();
            break;
        case KC_TYPE_VCS80:
            _ui = new UI_Gtk_VCS80();
            break;
        case KC_TYPE_C80:
            _ui = new UI_Gtk_C80();
            break;
        case KC_TYPE_ALL:
        case KC_TYPE_NONE:
        case KC_TYPE_85_1_CLASS:
        case KC_TYPE_85_2_CLASS:
            DBG(0, form("KCemu/internal_error",
                    "KCemu: got unhandled value from get_kc_type(): %d\n",
                    Preferences::instance()->get_kc_type()));
            break;
    }
    
    z80->register_ic(this);
    add_callback();
}

void
UI_Gtk::destroy_ui(void) {
    z80->unregister_ic(this);
    delete _ui;
}

void
UI_Gtk::add_callback(void) {
    z80->addCallback(_callback_value, this, (void *)0);
    if (_callback_value_retrace > 0)
        z80->addCallback(_callback_value_retrace, this, (void *)1);
}

void
UI_Gtk::callback(void *data) {
    if (data == (void *)1) {
        if (_ui != NULL) {
            _ui->get_generic_ui()->generic_signal_v_retrace(false);
        }
    } else {
        if (_ui != NULL) {
            update();
            _ui->get_generic_ui()->generic_signal_v_retrace(true);
        }
        add_callback();
    }
}

void
UI_Gtk::flash(bool enable) {
    Scanline *scanline = _ui->get_scanline();
    if (scanline != NULL) {
        scanline->trigger(enable);
    }
}

int
UI_Gtk::get_mode(void) {
    if (_ui == NULL) {
        DBG(1, form("KCemu/warning",
                "UI_Gtk::get_mode(): generic ui not yet initialized!\n"));
        return 0;
    }
    return _ui->get_generic_ui()->generic_get_mode();
}

void
UI_Gtk::set_mode(int mode) {
    if (_ui == NULL) {
        DBG(1, form("KCemu/warning",
                "UI_Gtk::set_mode(): generic ui not yet initialized!\n"));
        return ;
    }
    _ui->get_generic_ui()->generic_set_mode(mode);
    gtk_resize();
    update(true, true);
}

void
UI_Gtk::memory_read(word_t addr) {
    MemAccess *memaccess = _ui->get_mem_access();
    if (memaccess != NULL) {
        memaccess->memory_read(addr);
    }
}

void
UI_Gtk::memory_write(word_t addr) {
    MemAccess *memaccess = _ui->get_mem_access();
    if (memaccess != NULL) {
        memaccess->memory_write(addr);
    }
}

int
UI_Gtk::get_width(void) {
    return kcemu_ui_scale * _ui->get_generic_ui()->get_real_width();
}

int
UI_Gtk::get_height(void) {
    return kcemu_ui_scale * _ui->get_generic_ui()->get_real_height();
}

void
UI_Gtk::reset(bool power_on)
{
  if (_video_encoder_start_on_reset)
    {
      _video_encoder_start_on_reset = false;
      CMD_EXEC("ui-video-reset");
      set_video_encoder_state(VideoEncoder::VIDEO_ENCODER_STATE_RECORD);
    }
}