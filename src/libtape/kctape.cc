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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>

#include "kc/system.h"

#ifdef USE_INCLUDED_GETOPT
#include "getopt/getopt.h"
#else
#ifdef HAVE_GETOPT
#include <getopt.h>
#endif
#endif

#include "libtape/kct.h"
#include "fileio/load.h"

using namespace std;

typedef enum {
  ADD_MODE_NONE    = 0,
  ADD_MODE_RAW     = 1,
  ADD_MODE_KC85_1  = 2,
  ADD_MODE_KC85_3  = 3,
} add_mode_t;

static add_mode_t _add_mode    	    = ADD_MODE_NONE;
static int        _verbose          = 0;
static bool       _list_flag   	    = false;
static bool       _create_flag 	    = false;
static bool       _print_bam   	    = false;
static bool       _print_block_list = false;
static char *     _tape_name        = 0;
static char *     _extract_name     = 0;
static char *     _output_name      = 0;
static char *     _remove_name      = 0;
static char *     _dump_name        = 0;

static KCTFile kct_file;

static void
banner(void)
{
  cout << ("   _  ______ _\n"
	   "  | |/ / ___| |_ __ _ _ __   ___                KCtape 0.3\n"
	   "  | ' / |   | __/ _` | '_ \\ / _ \\       (c) 1997-2003 Torsten Paul\n"
	   "  | . \\ |___| || (_| | |_) |  __/         <Torsten.Paul@gmx.de>\n"
	   "  |_|\\_\\____|\\__\\__,_| .__/ \\___|      http://kcemu.sourceforge.net/\n"
	   "                     |_|\n");
}

static void
usage(char *argv0, int exit_value)
{
  banner();
  cout << _("\n"
	    "KCtape is part of KCemu the KC 85/4 Emulator and comes with\n"
	    "ABSOLUTELY NO WARRANTY; for details run `kcemu --warranty'.\n"
	    "This is free software, and you are welcome to redistribute it\n"
	    "under certain conditions; run `kcemu --license' for details.\n"
	    "\n"
	    "usage: kctape -t tapefile [command [command_args]]\n"
	    "\n"
	    " commands:\n"
	    " ---------\n"
	    "\n"
	    "  -h|--help              show this help text\n"
	    "  -v|--verbose           be verbose about what's going on\n"
	    "  -t|--tape              tape archive to process\n"
	    "  -l|--list              list content of tape archive\n"
	    "  -c|--create            create tape archive if it doesn't exist\n"
	    "  -a|--add               add files (KC85/3 mode)\n"
	    "  -1|--add1              add files (KC85/1 mode)\n"
	    "  -A|--add-raw           add binary files\n"
	    "  -r|--remove            remove file from tape archive\n"
	    "  -x|--extract           extract file from tape archive\n"
	    "  -d|--dump              hexdump file\n"
	    "  -o|--output            output file for extract\n"
	    "  -b|--print-bam         show internal block allocation map\n"
	    "  -B|--print-block-list  show internal block list\n");

  exit(exit_value);
}

bool
dump_block(istream *s, int addr)
{
  int c, x;
  bool end;

  c = s->get();
  if (c == EOF)
    {
      end = true;
      return false;
    }

  cout << "Block " << hex << setw(2) << setfill('0')
       << (c & 0xff) << "h (" << dec << (c & 0xff) << ")"
       << endl;

  end = false;
  for (int a = 0;a < 8;a++)
    {
      int c;
      char buf[16];

      cout << hex << setw(4) << setfill('0') << (addr + a * 16) << "h: ";
      x = 16;
      for (int b = 0;b < 16;b++)
        {
          c = s->get();
          if (c == EOF)
            {
              x = b;
              end = true;
              break;
            }
          buf[b] = c;
        }
      for (int b = 0;b < 16;b++)
        {
	  if (b == 8)
	    cout << "- ";
          if (b >= x)
            cout << "   ";
          else
	    cout << hex << setw(2) << setfill('0') << (buf[b] & 0xff) << " ";
        }
      cout << "| ";
      for (int b = 0;b < 16;b++)
        {
          if (b >= x)
            break;
          cout << (char)(isprint(buf[b]) ? buf[b] : '.');
        }
      cout << endl;
      if (end)
        break;
    }
  cout << endl;

  return !end;
}


static kct_error_t
add_one_file(KCTFile &f, fileio_prop_t *prop, kct_file_type_t type)
{
  char *name;
  kct_error_t err;
  int len, idx, nr;
  
  len = strlen((const char *)&prop->name[0]);
  name = new char[len + 3];
  strcpy(name, (const char *)&prop->name[0]);

  idx = min(len, KCT_NAME_LENGTH - 3);

  nr = 0;
  do
    {
      err = kct_file.write(name,
			   prop->data, prop->size,
			   prop->load_addr, prop->start_addr,
			   type, KCT_MACHINE_ALL);
      
      if (err != KCT_ERROR_EXIST)
	break;

      name[idx    ] = '_';
      name[idx + 1] = '1' + nr;
      name[idx + 2] = '\0';
      nr++;

      if (_verbose > 0)
	cout << "  + trying to add file with new name `" << name << "'..." << endl;
    }
  while (nr < 9);

  if (_verbose > 0)
    {
      if (err == KCT_OK)
	cout << "  + added file `" << name << "'" << endl;
      else
	cout << "  - can't add file `" << prop->name << "'" << endl;
    }

  return err;
}

static kct_error_t
add_raw(KCTFile &kct_file, char *filename)
{
  FILE *f;
  int len;
  kct_error_t err;
  byte_t buf[65536];

  if (_verbose > 0)
    cout << "* processing raw file `" << filename << "'...\n";

  f = fopen(filename, "rb");
  if (f == NULL)
    return KCT_ERROR_IO;

  len = fread(buf, 1, 65536, f);
  fclose(f);

  err = kct_file.write(filename, buf, len, 0x0000, 0x0000,
		       KCT_TYPE_BIN, KCT_MACHINE_ALL);

  return err;
}

static kct_error_t
add_file(KCTFile &kct_file, char *filename)
{
  kct_error_t err;
  kct_file_type_t type;
  fileio_prop_t *ptr, *prop;

  if (_verbose > 0)
    cout << "* processing file `" << filename << "'...\n";

  if (fileio_load_file(filename, &prop) != 0)
    return KCT_ERROR_IO;

  if (_verbose > 1)
    fileio_debug_dump(prop, 0);

  for (ptr = prop;ptr != NULL;ptr = ptr->next)
    {
      switch (ptr->type)
	{
	case FILEIO_TYPE_COM:
	  type = KCT_TYPE_COM;
	  break;
	case FILEIO_TYPE_BAS:
	  type = KCT_TYPE_BAS;
	  break;
	case FILEIO_TYPE_PROT_BAS:
	  type = KCT_TYPE_BAS_P;
	  break;
	default:
	  cerr << "ERROR: file with unknown type ignored!" << endl;
	  continue;
	}
      
      err = add_one_file(kct_file, ptr, type);

      if (err != KCT_OK)
	cout << "ERROR: can't add `" << ptr->name << "': "
	     << kct_file.get_error_string(err) << "." << endl;
    }

  return KCT_OK;
}

static kct_error_t
open(KCTFile &f, char *name)
{
  kct_error_t error;

  error = f.open(name);
  if (error == KCT_OK_READONLY)
    return KCT_OK;

  if (f.open(name) != KCT_OK)
    {
      if (_create_flag)
	{
	  if (f.create(name) != KCT_OK)
	    {
	      cerr << "ERROR: Can't create file `" << name << "'" << endl;
	      return KCT_ERROR_IO;
	    }
	}
      else
	{
	  cerr << "ERROR: Can't open file `" << name << "'" << endl;
	  return KCT_ERROR_NOENT;
	}
    }

  return KCT_OK;
}

static void
do_remove(KCTFile &f, const char *name)
{
  if (name == 0)
    return;

  f.readdir();
  if (f.remove(name) != KCT_OK)
    cerr << "ERROR: File not found" << endl;
}

static FILE *
open_output(const char *name)
{
  if (name == 0)
    return stdout;

  if (access(name, F_OK) == 0)
    return 0;

  return fopen(name, "wb+");
}

static void
do_extract(KCTFile &f, const char *name, const char *output)
{
  int a, c;
  FILE *out;
  istream *s;
  kct_file_props_t props;

  if (name == 0)
    return;

  out = open_output(output);
  if (out == 0)
    {
      cerr << "ERROR: can't open output file" << endl;
      return;
    }

  f.readdir();
  s = f.read(name, &props);
  if (s == NULL)
    {
      cerr << "ERROR: File not found" << endl;
      return;
    }
  
  cerr << name << ": "
       << "load = "  << hex << setw(4) << props.load_addr << ", "
       << "size = "  << hex << setw(4) << props.size << ", "
       << "start = " << hex << setw(4) << props.start_addr
       << (props.auto_start ? " [autostart]" : "")
       << endl;

  fputs("\xc3KC-TAPE by AF. ", out);
  a = 0;
  while (242)
    {
      c = s->get();
      if (c == EOF)
	break;
      a++;
      fputc(c, out);
    }
  while ((a % 129) != 0)
    {
      a++;
      fputc('\0', out); /* pad to block size */
    }
}

/*
 *  the output parameter is not yet used; need to unify the
 *  type of the output stream (extract() still uses FILE * but
 *  dump() need an ostream)
 */
static void
do_dump(KCTFile &f, const char *name, const char *output)
{
  istream *s;
  kct_file_props_t props;

  if (name == 0)
    return;

  f.readdir();
  s = f.read(name, &props);
  if (s == NULL)
    {
      cerr << "ERROR: File not found: `" << _dump_name << "'" << endl;
      return ;
    }

  cout << "===========================================================================" << endl
       << "name: `" << name << "'" << endl
       << "load:  " << hex << setfill('0') << setw(4) << props.load_addr << "h" << endl
       << "size:  " << hex << setfill('0') << setw(4) << props.size << "h" << endl
       << "start: " << hex << setfill('0') << setw(4) << props.start_addr << "h"
       << (props.auto_start ? " [autostart]" : "") << endl
       << "===========================================================================" << endl
       << endl;

  int addr = 0;
  while (dump_block(s, addr))
    addr += 128;

  cout << "===========================================================================" << endl;
}

static void
do_add(KCTFile &f, int idx, int argc, char **argv)
{
  int a;

  switch (_add_mode)
    {
    case ADD_MODE_KC85_1:
      fileio_set_kctype(FILEIO_KC85_1);
      break;
      
    case ADD_MODE_KC85_3:
      fileio_set_kctype(FILEIO_KC85_3);
      break;

    case ADD_MODE_RAW:
      for (a = idx;a < argc;a++)
	{
	  if (add_raw(kct_file, argv[a]) != KCT_OK)
	    printf("ERROR\n");
	}
      return;

    default:
      return;
    }

  for (a = idx;a < argc;a++)
    {
      if (add_file(kct_file, argv[a]) != KCT_OK)
	printf("ERROR\n");
    }
}

int
main(int argc, char **argv)
{
  int c, idx;
  int option_index;

#ifdef HAVE_GETOPT_LONG
  static struct option long_options[] =
  {
    { "help",    	  0, 0, 'h' },
    { "list",    	  0, 0, 'l' },
    { "create",  	  0, 0, 'c' },
    { "add",     	  0, 0, 'a' },
    { "add1",    	  0, 0, '1' },
    { "add-raw",          0, 0, 'A' },
    { "print-bam",        0, 0, 'b' },
    { "print-block-list", 0, 0, 'B' },
    { "verbose",          0, 0, 'v' },
    { "tape",    	  1, 0, 't' },
    { "remove",  	  1, 0, 'r' },
    { "extract", 	  1, 0, 'x' },
    { "dump",    	  1, 0, 'd' },
    { "output",    	  1, 0, 'o' },
    { 0, 0, 0, 0 }
  };
#endif /* HAVE_GETOPT_LONG */

  idx = 1;

  if (argc < 2)
    usage(argv[0], 0);

  while (242)
    {
#ifdef HAVE_GETOPT_LONG
      c = getopt_long(argc, argv, "hlca1AbBvt:x:r:d:o:",
                      long_options, &option_index);
#else
#ifdef HAVE_GETOPT
      c = getopt(argc, argv, "hlca1AbBvt:x:r:d:o:");
#else
#error neither HAVE_GETOPT_LONG nor HAVE_GETOPT defined
#endif /* HAVE_GETOPT */
#endif /* #ifdef HAVE_GETOPT_LONG */

      if (c == -1)
	break;

      switch (c)
	{
	case 'l':
	  idx++;
	  _list_flag = true;
	  break;

	case 'c':
	  idx++;
	  _create_flag = true;
	  break;

	case '1':
	  idx++;
	  _add_mode = ADD_MODE_KC85_1;
	  break;

	case 'a':
	  idx++;
	  _add_mode = ADD_MODE_KC85_3;
	  break;

	case 'A':
	  idx++;
	  _add_mode = ADD_MODE_RAW;
	  break;

	case 'b':
	  idx++;
	  _print_bam = true;
	  break;

	case 'B':
	  idx++;
	  _print_block_list = true;
	  break;

	case 'v':
	  idx++;
	  _verbose++;
	  break;

	case 't':
	  idx += 2;
	  _tape_name = strdup(optarg);
	  break;

	case 'x':
	  idx += 2;
	  _extract_name = strdup(optarg);
	  break;

	case 'r':
	  idx += 2;
	  _remove_name = strdup(optarg);
	  break;

	case 'd':
	  idx += 2;
	  _dump_name = strdup(optarg);
	  break;

	case 'o':
	  idx += 2;
	  _output_name = strdup(optarg);
	  break;

        case ':':
        case '?':
        case 'h':
        default:
          usage(argv[0], 0);
          break;
        }
    }

  if (_tape_name == 0)
    {
      printf(_("ERROR: no tape specified, use the -t option to select\n"
	       "       the tape archive you want to process\n\n"));
      usage(argv[0], 1);
    }

  
  fileio_init();
  kct_file.test();

  if (open(kct_file, _tape_name) != KCT_OK)
    exit(1);

  do_remove(kct_file, _remove_name);
  do_extract(kct_file, _extract_name, _output_name);
  do_dump(kct_file, _dump_name, _output_name);
  do_add(kct_file, idx, argc, argv);

  kct_file.readdir();

  if (_list_flag)
    kct_file.list();

  if (_print_bam)
    kct_file.print_bam();
  
  if (_print_block_list)
    kct_file.print_block_list();

  kct_file.close();
}
