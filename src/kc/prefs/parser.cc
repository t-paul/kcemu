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

#include <fcntl.h>
#include <unistd.h>

#include "kc/system.h"

#include "kc/kc.h"
#include "kc/prefs/parser.h"

#include "libdbg/dbg.h"

const char * ProfileParser::signature = "# kcemu configuration file";

ProfileParser::ProfileParser(string filename, profile_level_t level) {
    _filename = filename;
    _level = level;
    _current_profile = NULL;
}

ProfileParser::~ProfileParser(void) {
}

bool
ProfileParser::is_newline(GScanner *scanner, GTokenType token_type) {
    // we have 'char to token' enabled, so we only need to check the token type...
    return (token_type == '\r') || (token_type == '\n');
}

GTokenType
ProfileParser::handle_line(GScanner *scanner, const char *key, GTokenType token_type) {
    _current_profile = get_profile();
    switch (token_type) {
        case G_TOKEN_IDENTIFIER_NULL:
            _current_profile->set_null_value(key);
            break;
        case G_TOKEN_INT:
            _current_profile->set_int_value(key, scanner->value.v_int);
            break;
        case G_TOKEN_FLOAT:
            break;
        case G_TOKEN_STRING:
            _current_profile->set_string_value(key, scanner->value.v_string);
            break;
        default:
            break;
    }
    return G_TOKEN_NONE;
}

void
ProfileParser::parse() {
    int fd = open(_filename.c_str(), O_RDONLY);
    if (fd < 0) {
        DBG(1, form("KCemu/ProfileParser",
                "ProfileParser::parse(): Failed to open file '%s'. Skipping ...\n",
                _filename.c_str()));
        return;
    }
    DBG(1, form("KCemu/ProfileParser",
            "ProfileParser::parse(): Parsing file '%s'...\n",
            _filename.c_str()));
    

    const char signature_length = strlen(signature);
    char signature_buf[signature_length];

    int len = read(fd, signature_buf, signature_length);
    if ((len != signature_length) || (strncmp(signature_buf, signature, signature_length) != 0)) {
        DBG(1, form("KCemu/ProfileParser",
                "ProfileParser::parse(): Signature '%s' not found in file '%s'. Skipping ...\n",
                signature, _filename.c_str()));
        close(fd);
        return;
    }
    
    lseek(fd, 0, SEEK_SET);
    
    GScanner *scanner = g_scanner_new(NULL);
    g_scanner_input_file(scanner, fd);
    
    scanner->input_name = _filename.c_str();
    
    scanner->config->cset_skip_characters = (char*)" \t";
    scanner->config->cset_identifier_first = (char*)G_CSET_a_2_z;
    scanner->config->cset_identifier_nth = (char*)G_CSET_a_2_z "_" G_CSET_DIGITS;
    scanner->config->scan_identifier_NULL = TRUE;
    
    int state = 1;
    gchar *key = NULL;
    
    while (242) {
        GTokenType ttype = g_scanner_get_next_token(scanner);
        if (ttype == G_TOKEN_EOF) {
            g_free(key);
            break;
        }
        
        switch (state) {
            case 0: // STATE_SCAN_TO_NEWLINE
                if (is_newline(scanner, ttype))
                    state = 1;
                break;
            case 1: // IDENTIFIER
                if (ttype == G_TOKEN_IDENTIFIER) {
                    g_free(key);
                    key = g_strdup(scanner->value.v_string);
                    state = 2;
                } else if (!is_newline(scanner, ttype)) { // eat up empty lines without reporting an error
                    g_scanner_unexp_token(scanner, G_TOKEN_IDENTIFIER, NULL, NULL, NULL, NULL, FALSE);
                    state = 0;
                }
                break;
            case 2: // EQUAL_SIGN
                if (ttype == G_TOKEN_EQUAL_SIGN) {
                    state = 3;
                } else {
                    g_scanner_unexp_token(scanner, G_TOKEN_EQUAL_SIGN, NULL, NULL, NULL, NULL, FALSE);
                    is_newline(scanner, ttype) ? state = 1 : state = 0;
                }
                break;
            case 3: // VALUE
                switch (ttype) {
                    case G_TOKEN_INT:
                    case G_TOKEN_STRING:
                    case G_TOKEN_IDENTIFIER_NULL: {
                        GTokenType expected = handle_line(scanner, key, ttype);
                        if (expected != G_TOKEN_NONE) {
                            g_scanner_unexp_token(scanner, expected, NULL, NULL, NULL, NULL, FALSE);
                        }
                        state = 0;
                        break;
                    }
                    default:
                        g_scanner_unexp_token(scanner, G_TOKEN_STRING, NULL, NULL, NULL, NULL, FALSE);
                        is_newline(scanner, ttype) ? state = 1 : state = 0;
                        break;
                }
                break;
            default:
                g_assert("must not be reached");
                break;
        }
    }
    
    g_scanner_destroy(scanner);
    close(fd);
}

profile_list_t *
ProfileParser::get_profiles(void) {
    return &_profile_list;
}

SingleProfileParser::SingleProfileParser(string filename, profile_level_t level, string config_name, string name) : ProfileParser(filename, level) {
    _name = name;
    _profile = new Profile(_level, filename, config_name, _name);
    _profile_list.push_back(_profile);
}

SingleProfileParser::~SingleProfileParser(void) {
}

Profile *
SingleProfileParser::get_profile(void) {
    return _profile;
}

MultiProfileParser::MultiProfileParser(string filename, profile_level_t level) : ProfileParser(filename, level) {
}

MultiProfileParser::~MultiProfileParser(void) {
}

Profile *
MultiProfileParser::get_profile(void) {
    Profile *profile = new Profile(_level, "", "", "");
    _profile_list.push_back(profile);
    return profile;
}
