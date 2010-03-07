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

#ifndef __ui_gtk_options_h
#define __ui_gtk_options_h

#include <list>
#include <string>

#include "kc/system.h"
#include "kc/prefs/profile.h"

#include "ui/gtk/window.h"

using namespace std;

class OptionsWindow : public UI_Gtk_Window {
private:
    enum {
        TREE_INT_COLUMN,
        TREE_TEXT_COLUMN,
        TREE_ICON_COLUMN,
        TREE_CONFIG_NAME_COLUMN,
        
        TREE_N_COLUMNS
    };
    
    enum {
        MODULES_NAME_COLUMN,
        MODULES_MODIF_COLUMN,
        MODULES_POINTER_COLUMN,
        
        MODULES_N_COLUMNS
    };
            
    enum {
        ROMS_DESCRIPTION_COLUMN,
        ROMS_FILENAME_COLUMN,
        ROMS_TYPE_COLUMN,
        
        ROMS_N_COLUMNS
    };
    
    enum {
        NR_OF_MODULES = 8,
        NR_OF_ROMS = 8,
        NR_OF_NETWORK_SETTINGS = 4,
    };
    
    static const char * FILE_CHOOSER_BUTTON_KEY;
    static const char * FILE_CHOOSER_CLEAR_BUTTON_KEY;
    static const char * DATA_KEY_CHECK_BUTTON;
    static const char * PREFERENCES_KEY;
    static const char * DEFAULT_DIR_KEY;
    static const char * TARGET_WIDGET1_KEY;
    static const char * TARGET_WIDGET2_KEY;
    static const char * TARGET_WIDGET3_KEY;
    
    // unfortunately the signature of gtk_tree_path_prev and
    // gtk_tree_path_next are different. but as we just need to
    // treat them as void, we cast them to the following type.
    typedef void (*GtkPathMoveFunc)(GtkTreePath *);
    
    typedef void (*GtkIterMoveFunc)(GtkTreeStore *, GtkTreeIter *, GtkTreeIter *);
    
    struct {
        GtkWidget    *treeview;
        GtkWidget    *b_new;
        GtkWidget    *b_copy;
        GtkWidget    *b_delete;
        GtkWidget    *b_up;
        GtkWidget    *b_down;
        GtkWidget    *b_ok;
        GtkWidget    *b_close;
        GtkWidget    *b_expand;
        GtkWidget    *b_collapse;
        
        GtkNotebook    *notebook;
        
        GtkEntry       *entry_profile_name;
        GtkComboBox    *combobox_system_type;
        GtkComboBox    *combobox_system_variant;
        GtkCheckButton *check_button_system_variant;
        GtkTextView    *textview_comment;
        GtkTextBuffer  *textbuffer_comment;
        
        GtkCheckButton *check_button_display_scale;
        GtkCheckButton *check_button_display_effects;
        GtkCheckButton *check_button_display_mem_access;
        GtkCheckButton *check_button_display_debug;

        GtkCheckButton *check_button_d004;
        GtkCheckButton *check_button_f8_rom;
        GtkCheckButton *check_button_busdrivers;
        GtkComboBox    *combobox_d004;
        GtkComboBox    *combobox_f8_rom;
        GtkSpinButton  *spin_button_busdrivers;

        GtkSpinButton  *spin_button_display_scale;
        GtkComboBox    *combobox_display_effects;
        GtkComboBox    *combobox_display_mem_access;
        GtkComboBox    *combobox_display_debug;
        GtkComboBox    *combobox_module[NR_OF_MODULES];
        GtkCheckButton *check_button_modules;
        
        GtkEntry       *entry_network_ip_address;
        GtkEntry       *entry_network_netmask;
        GtkEntry       *entry_network_gateway;
        GtkEntry       *entry_network_dns_server;

        GtkLabel         *roms_label[NR_OF_ROMS];
        GtkComboBoxEntry *roms_comboboxentry[NR_OF_ROMS];
        GtkButton        *roms_open_button[NR_OF_ROMS];
        GtkCheckButton   *roms_check_button[NR_OF_ROMS];
        
        GtkTreeStore *treestore;
        GtkListStore *liststore_system;
        GtkListStore *liststore_modules;
        
        GdkPixbuf    *pixbuf_root;
        GdkPixbuf    *pixbuf_default;
        GdkPixbuf    *pixbuf_deleted;
        
        gint          on_profile_name_changed_id;
        gint          on_comment_changed_id;
        gint          on_system_type_changed_id;
        gint          on_system_variant_changed_id;
        gint          on_display_scale_value_changed_id;
        gint          on_display_effects_changed_id;
        gint          on_display_mem_access_changed_id;
        gint          on_display_debug_changed_id;
        gint          on_kc85_d004_changed_id;
        gint          on_kc85_f8_rom_changed_id;
        gint          on_kc85_busdrivers_changed_id;
        gint          on_module_changed_id[NR_OF_MODULES];
        gint          on_rom_changed_id[NR_OF_ROMS];
        gint          on_network_changed_id[NR_OF_NETWORK_SETTINGS];
    } _w;
    
    CMD *_cmd;
    CMD *_cmd_dialog;
    
    kc_type_t     _current_kc_type;
    Profile      *_current_profile;
    list<string>  _delete_path;
    char         *_open_rom_last_path;

protected:
    void init(void);

    bool is_ip_address(const char *addr);

    GtkTreeStore * get_tree_model(void);
    GtkListStore * get_system_list_model(void);
    GtkListStore * get_variant_list_model(kc_variant_t kc_variant, const char *name, ...);
    GtkListStore * get_modules_list_model(kc_type_t kc_type);

    GtkTreeModel * get_selected_tree_iter(GtkTreeIter *iter);
    void expand_and_select(GtkTreeView *treeview, GtkTreeModel *model, GtkTreeIter *iter);
    ProfileValue * get_current_profile_value(const char *key);
    const char * get_preferences_key(GObject *object);
    void set_preferences_key(GObject *object, const char *key);

    void apply_profile(void);
    void apply_comment(void);
    void apply_system_type(void);
    void apply_kc85_settings(void);
    void apply_display_settings(void);
    void apply_network_settings(void);
    void apply_modules_settings(kc_type_t kc_type);
    void apply_roms_settings(kc_type_t kc_type, kc_variant_t kc_variant);
    void apply_system_variant(kc_type_t kc_type, kc_variant_t kc_variant);
    void apply_filechooserbutton(GtkFileChooser *filechooser);
    void apply_entry_value(GtkEntry *entry, gint signal_id);
    void apply_combobox_value(GtkCheckButton *check_button, GtkComboBox *combobox, gint handler_id);
    void apply_spin_button_value(GtkCheckButton *check_button, GtkSpinButton *spin_button, gint signal_id, int default_value);

    void expand_tree(void);
    void collapse_tree(void);
    void add_system(GtkTreeStore *store, GtkTreeIter *iter, const char *name, const char *config_name, const char *icon_name);
    void move_row(GtkPathMoveFunc path_func, GtkIterMoveFunc move_func);
    void set_roms_liststore(int idx, const char *rom_key, SystemROM *rom);
    bool check_button_toggled(GtkToggleButton *togglebutton);
    void set_widget_sensitive_by_key(GObject *object, const gchar *key, bool sensitive);
    void wire_check_button(const char *preferences_key, GtkCheckButton *check_button, GCallback callback, GtkWidget *target1, GtkWidget *target2 = NULL, GtkWidget *target3 = NULL);
    void wire_check_button(const char *preferences_key, const char *default_dir, const char *check_button_name, const char *filechooser_button_name, const char *button_clear_name);

    static void on_button_new_clicked(GtkButton *button, gpointer user_data);
    static void on_button_copy_clicked(GtkButton *button, gpointer user_data);
    static void on_button_delete_clicked(GtkButton *button, gpointer user_data);
    static void on_button_up_clicked(GtkButton *button, gpointer user_data);
    static void on_button_down_clicked(GtkButton *button, gpointer user_data);
    static void on_button_expand_clicked(GtkButton *button, gpointer user_data);
    static void on_button_collapse_clicked(GtkButton *button, gpointer user_data);
    static void on_button_ok_clicked(GtkButton *button, gpointer user_data);
    static void on_button_close_clicked(GtkButton *button, gpointer user_data);
    static void on_tree_selection_changed(GtkTreeSelection *selection, gpointer user_data);
    static void on_media_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);
    static void on_media_clear_button_clicked(GtkButton *button, gpointer user_data);
    static void on_media_filechooser_file_set(GtkFileChooserButton *filechooserbutton, gpointer user_data);
    
    static void on_display_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);
    static void on_display_scale_value_changed(GtkSpinButton *spin_button, gpointer user_data);
    static void on_display_effects_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_display_debug_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_display_mem_access_changed(GtkComboBox *combobox, gpointer user_data);
    
    static void on_profile_name_changed(GtkEditable *editable, gpointer user_data);
    static void on_comment_changed(GtkTextBuffer *textbuffer, gpointer user_data);
    static void on_system_type_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_system_variant_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_system_variant_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);
    
    static void on_module_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_modules_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);

    static void on_kc85_settings_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);
    static void on_kc85_d004_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_kc85_f8_rom_changed(GtkComboBox *combobox, gpointer user_data);
    static void on_kc85_busdrivers_changed(GtkSpinButton *spin_button, gpointer user_data);

    static void on_rom_changed(GtkComboBoxEntry *comboboxentry, gpointer user_data);
    static void on_rom_open_clicked(GtkButton *button, gpointer user_data);
    static void on_roms_settings_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);

    static void on_network_settings_check_button_toggled(GtkToggleButton *togglebutton, gpointer user_data);
    static void on_network_changed(GtkEntry *entry, gpointer user_data);

    static gboolean tree_model_foreach_func_delete(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
    
public:
    OptionsWindow(const char *ui_xml_file);
    virtual ~OptionsWindow(void);
    
    void save(void);
    void reject(void);
};

#endif /* __ui_gtk_options_h */
