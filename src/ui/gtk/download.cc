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

#include <string.h>
#include <stdlib.h>

#include <iostream>

#include <gcrypt.h>
#include <curl/curl.h>

#include "kc/system.h"

#include "kc/roms.h"
#include "kc/prefs/prefs.h"

#include "ui/gtk/cmd.h"
#include "ui/gtk/download.h"

class CMD_rom_download_window_toggle : public CMD
{
private:
  RomDownloadWindow *_w;

public:

  CMD_rom_download_window_toggle(RomDownloadWindow *w) : CMD("ui-rom-download-window-toggle")
  {
    _w = w;
    register_cmd("ui-rom-download-window-toggle");
  }

  void
  execute(CMD_Args *args, CMD_Context context)
  {
    _w->toggle();
  }
};

RomDownloadWindow::RomDownloadWindow(const char *ui_xml_file) : UI_Gtk_Window(ui_xml_file)
{
  _cmd = new CMD_rom_download_window_toggle(this);
}

RomDownloadWindow::~RomDownloadWindow(void)
{
  delete _cmd;
}

gboolean
RomDownloadWindow::download_ui_update(gpointer data)
{
  RomDownloadWindow *self = (RomDownloadWindow *)data;

  if (self->_w.bytes_total < 0)
    {
      char buf[1024];
      if (self->_w.download_errors == 0)
        {
          snprintf(buf, sizeof(buf), _("Success"));
        }
      else
        {
          snprintf(buf, sizeof(buf), _("Not downloaded: %d"), self->_w.download_errors);
        }

      self->set_text();

      gtk_progress_bar_update(GTK_PROGRESS_BAR(self->_w.progressbar), 1.0);
      gtk_progress_bar_set_text(self->_w.progressbar, buf);
      gtk_widget_set_sensitive(self->_window, TRUE);
      return FALSE;
    }

  if (self->_w.bytes_total > 0)
    {
      gtk_progress_bar_set_text(self->_w.progressbar, self->_w.download_filename);
      gtk_progress_bar_update(GTK_PROGRESS_BAR(self->_w.progressbar), (self->_w.bytes_base + self->_w.bytes_now) / self->_w.bytes_total);
    }
  
  return TRUE;
}

size_t
RomDownloadWindow::download_thread_write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  RomDownloadWindow *self = (RomDownloadWindow *)stream;

  size_t len = size * nmemb;
  if (self->_w.download_pos + len > self->_w.download_size)
    return 0;

  memcpy(self->_w.download_buffer + self->_w.download_pos, ptr, len);
  self->_w.download_pos += len;

  return nmemb;
}

size_t
RomDownloadWindow::download_thread_read_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  return 0;
}

int
RomDownloadWindow::download_thread_progress_func(void *ptr, double dltotal, double dlnow, double ultotal, double ulnow)
{
  RomDownloadWindow *self = (RomDownloadWindow *)ptr;

  if (dlnow <= dltotal)
    self->_w.bytes_now = dlnow;

  return 0;
}

gpointer
RomDownloadWindow::download_thread(gpointer data)
{
  RomDownloadWindow *self = (RomDownloadWindow *)data;

  const char *selector = "";
  if (!self->_w.download_all)
    {
      const SystemType *system = Preferences::instance()->get_system_type();
      selector = system->get_rom_directory().c_str();
    }

  long bytes = 0;
  const rom_registry_entry_list_t *missing_roms = RomRegistry::instance()->get_missing(selector);
  for (rom_registry_entry_list_t::const_iterator it = missing_roms->begin();it != missing_roms->end();it++)
    bytes += (*it)->get_size();

  CURL *curl = curl_easy_init();
  if (curl == NULL)
    return NULL;

  int errors = 0;
  self->_w.bytes_now = 0;
  self->_w.bytes_base = 0;
  self->_w.bytes_total = bytes;
  string base_url("http://localhost/KCemu/");
  for (rom_registry_entry_list_t::const_iterator it = missing_roms->begin();it != missing_roms->end();it++)
    {
      self->_w.download_pos = 0;
      self->_w.download_size = (*it)->get_size();
      self->_w.download_buffer = new byte_t[self->_w.download_size];
      memset(self->_w.download_buffer, 0, self->_w.download_size);
      self->_w.download_filename = (*it)->get_filename();
      
      string url = base_url + (*it)->get_id();
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_USERAGENT, "KCemu v" KCEMU_VERSION);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, self);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_thread_write_func);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, download_thread_read_func);
      curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 1024L);
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_thread_progress_func);
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, self);
      CURLcode res = curl_easy_perform(curl);
      cout << (*it)->get_id() << " result: " << res << endl;

      self->_w.bytes_now = 0;
      self->_w.bytes_base += (*it)->get_size();

      if (check_sha1(self->_w.download_buffer, self->_w.download_size, (*it)->get_id()))
        {
          if (!RomRegistry::instance()->save_rom((*it), self->_w.download_buffer, self->_w.download_size))
            self->_w.download_errors++;
        }
      else
        {
          self->_w.download_errors++;
        }
    }

  curl_easy_cleanup(curl);
  self->_w.bytes_total = -1;

  delete missing_roms;
}

bool
RomDownloadWindow::check_sha1(byte_t *buffer, int len, const char *sha1)
{
  static int init;
  gcry_md_hd_t h;
  gcry_error_t err;
  char hash[20];
  char text[41];

  if (!init)
    {
      if (!gcry_check_version(NULL))
        return false;
      gcry_control(GCRYCTL_DISABLE_SECMEM, 0);
      gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
      init = 1;
    }

  err = gcry_md_open(&h, GCRY_MD_SHA1, 0);
  if (err)
    return false;
  
  gcry_md_write(h, buffer, len);
  memcpy(hash, gcry_md_read(h, GCRY_MD_SHA1), 20);
  gcry_md_close(h);

  for (int a = 0;a < 20;a++)
    snprintf(&text[2 * a], 3, "%02x", hash[a] & 0xff);

  return strcmp(sha1, text) == 0;
}

void
RomDownloadWindow::on_dialog_download(GtkWidget *widget, gpointer user_data)
{
  RomDownloadWindow *self = (RomDownloadWindow *) user_data;

  self->_w.bytes_total = 0;
  self->_w.download_errors = 0;
  self->_w.download_all = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self->_w.checkbutton_download_all));
  g_timeout_add(100, download_ui_update, self);
  gtk_widget_set_sensitive(self->_window, FALSE);
  if (g_thread_create(download_thread, self, FALSE, NULL) == 0)
    {
      cout << "can't create thread!" << endl;
      return;
    }
}

void
RomDownloadWindow::on_dialog_continue(GtkWidget *widget, gpointer user_data)
{
  RomDownloadWindow *self = (RomDownloadWindow *) user_data;
  self->close();
}

void
RomDownloadWindow::on_dialog_quit(GtkWidget *widget, gpointer user_data)
{
  exit(0);
}

gboolean
RomDownloadWindow::on_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  exit(0);
}

void
RomDownloadWindow::show(void)
{
  UI_Gtk_Window::show();
  gtk_main();
}

void
RomDownloadWindow::close(void)
{
  hide();
  gtk_main_quit();
}

void
RomDownloadWindow::set_text(void)
{
  char buf[1024];

  const SystemType *system = Preferences::instance()->get_system_type();
  int all_missing = RomRegistry::instance()->count_missing_roms();
  int system_missing = RomRegistry::instance()->count_missing_roms(system->get_rom_directory());

  snprintf(buf, sizeof(buf), _("Some of the ROM images needed by KCemu are not installed or\n"
          "not accessible. KCemu can try to download the ROM images for\n"
          "the currently emulated system or all the systems from the KCemu\n"
          "homepage at http://kcemu.sourceforge.net/\n\n"
          "Number of missing ROMs for the current system: %d\n"
          "Number of missing ROMs total: %d\n\n"
          "If you need to use a proxy to access the internet, please enter\n"
          "the configuration data below."), system_missing, all_missing);

  gtk_label_set_text(_w.message, buf);

  gtk_widget_set_sensitive(GTK_WIDGET(_w.button_continue), system_missing == 0);
}

void
RomDownloadWindow::init(void)
{
  _window = get_widget("rom_download_window");
  gtk_signal_connect(GTK_OBJECT(_window), "delete_event", GTK_SIGNAL_FUNC(on_window_delete_event), this);

  _w.message = GTK_LABEL(get_widget("message"));
  _w.progressbar = GTK_PROGRESS_BAR(get_widget("progressbar_progress"));
  _w.checkbutton_download_all = GTK_CHECK_BUTTON(get_widget("checkbutton_download_all"));

  _w.button_download = GTK_BUTTON(get_widget("dialog_button_download"));
  g_signal_connect(_w.button_download, "clicked", G_CALLBACK(on_dialog_download), this);

  _w.button_continue = GTK_BUTTON(get_widget("dialog_button_continue"));
  g_signal_connect(_w.button_continue, "clicked", G_CALLBACK(on_dialog_continue), this);
  
  _w.button_quit = GTK_BUTTON(get_widget("dialog_button_quit"));
  g_signal_connect(_w.button_quit, "clicked", G_CALLBACK(on_dialog_quit), this);

  set_text();

  init_dialog(NULL, NULL);
}
