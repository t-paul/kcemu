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

#ifndef __ui_gtk_download_h
#define __ui_gtk_download_h

#include "kc/system.h"

#include "ui/gtk/window.h"

using namespace std;

class RomDownloadWindow : public UI_Gtk_Window {
private:
    struct
    {
      GtkLabel       *message;
      GtkProgressBar *progressbar;
      GtkButton      *button_download;
      GtkButton      *button_quit;
      GtkButton      *button_continue;
      GtkCheckButton *checkbutton_download_all;

      double          bytes_now;
      double          bytes_base;
      double          bytes_total;

      bool            download_all;
      int             download_pos;
      int             download_size;
      int             download_errors;
      byte_t         *download_buffer;
      const char     *download_filename;
    } _w;
    
    CMD *_cmd;
    
protected:
    void init(void);
    void set_text(void);
    
    void show(void);
    void close(void);

    static gboolean download_ui_update(gpointer data);
    static gpointer download_thread(gpointer data);
    static size_t download_thread_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
    static size_t download_thread_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream);
    static int download_thread_progress_func(void *ptr, double dltotal, double dlnow, double ultotal, double ulnow);
    static bool check_sha1(byte_t *buffer, int len, const char *sha1);

    static void on_dialog_download(GtkWidget *widget, gpointer user_data);
    static void on_dialog_quit(GtkWidget *widget, gpointer user_data);
    static void on_dialog_continue(GtkWidget *widget, gpointer user_data);
    static gboolean on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

public:
    RomDownloadWindow(const char *ui_xml_file);
    virtual ~RomDownloadWindow(void);
};

#endif /* __ui_gtk_download_h */

