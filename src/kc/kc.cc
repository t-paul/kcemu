/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: kc.cc,v 1.32 2002/10/31 01:46:35 torsten_paul Exp $
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "kc/system.h"

#ifdef HAVE_GETOPT
#include <getopt.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#undef __CALL_MTRACE
#ifdef HAVE_MCHECK_H
#ifdef DBG_LEVEL
#if DBG_LEVEL > 1
#define __CALL_MTRACE 1
#include <mcheck.h>
#warning including mcheck.h
#endif
#endif
#endif

#include "kc/kc.h"
#include "kc/rc.h"
#include "kc/ctc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/wav.h"
#include "kc/tape.h"
#include "kc/disk.h"
#include "kc/timer.h"
#include "kc/module.h"
#include "kc/mod_list.h"
#include "kc/sh_mem.h"
#include "kc/disk_io.h"
#include "kc/fdc.h"
#include "kc/sound.h"
#include "kc/ctc_fdc.h"
#include "kc/z80_fdc.h"

#include "ui/log.h"
#include "cmd/cmd.h"
#include "fileio/load.h"
#include "libdisk/libdisk.h"
#include "libaudio/libaudio.h"

#include "kc/pio1.h"
#include "kc/ctc1.h"
#include "kc/keyb1.h"
#include "kc/timer1.h"
#include "kc/memory1.h"
#include "kc/memory7.h"

#include "kc/pio2.h"
#include "kc/memory2.h"

#include "kc/pio3.h"
#include "kc/ctc3.h"
#include "kc/keyb3.h"
#include "kc/timer3.h"
#include "kc/sound3.h"
#include "kc/memory3.h"

#include "kc/pio4.h"
#include "kc/ports4.h"
#include "kc/memory4.h"

#include "kc/ctc8.h"
#include "kc/pio8.h"
#include "kc/keyb8.h"
#include "kc/sound8.h"
#include "kc/memory8.h"

#ifdef HOST_OS_LINUX
# ifdef USE_UI_GTK
#  include "ui/gtk/ui_gtk1.h"
#  include "ui/gtk/ui_gtk3.h"
#  include "ui/gtk/ui_gtk4.h"
#  include "ui/gtk/ui_gtk8.h"
#  define UI_1 UI_Gtk1
#  define UI_3 UI_Gtk3
#  define UI_4 UI_Gtk4
#  define UI_8 UI_Gtk8
# endif /* USE_UI_GTK */
# ifdef USE_UI_SDL
#  include "ui/sdl/ui_sdl1.h"
#  include "ui/sdl/ui_sdl3.h"
#  include "ui/sdl/ui_sdl4.h"
#  include "ui/sdl/ui_sdl8.h"
#  define UI_1 UI_SDL1
#  define UI_3 UI_SDL3
#  define UI_4 UI_SDL4
#  define UI_8 UI_SDL8
# endif /* USE_UI_SDL */
#endif /* HOST_OS_LINUX */

#ifdef HOST_OS_BEOS
# include "ui/beos/ui_beos1.h"
# include "ui/beos/ui_beos3.h"
# include "ui/beos/ui_beos4.h"
# include "ui/beos/ui_beos8.h"
# define UI_1 UI_BeOS1
# define UI_3 UI_BeOS3
# define UI_4 UI_BeOS4
# define UI_8 UI_BeOS8
#endif /* HOST_OS_BEOS */

using namespace std;

Z80        *z80;
UI         *ui;
Ports      *ports;
Ports      *ports_fdc;
Module     *module;
Memory     *memory;
CTC        *ctc;
PIO        *pio;
PIO        *pio2;
Tape       *tape;
Disk       *disk;
Timer      *timer;
Keyboard   *keyboard;
ModuleList *module_list;
CMD        *cmd;
Sound      *sound;
WavPlayer  *wav;

Z80_FDC         *fdc_z80;
FloppyIO        *fdc_io;
FloppySharedMem *fdc_shmem;
Ports           *fdc_ports;
FDC             *fdc_fdc;
CTC             *fdc_ctc;


int   kcemu_ui_scale;
int   kcemu_ui_debug;
char *kcemu_datadir;
char *kcemu_localedir;
char *kcemu_tape;
char *kcemu_emulate;

static kc_type_t     kcemu_kc_type;
static kc_variant_t  kcemu_kc_variant;
static const char   *kcemu_kc_variant_name;

static kc_variant_names_t kc_types[] = {
  { "z9001",    -1, KC_TYPE_85_1, KC_VARIANT_85_1_10 },
  { "z9001.10", -1, KC_TYPE_85_1, KC_VARIANT_85_1_10 },
  { "z9001.11", -1, KC_TYPE_85_1, KC_VARIANT_85_1_11 },
  { "kc85/1",    1, KC_TYPE_85_1, KC_VARIANT_85_1_10 },
  { "hc900",    -1, KC_TYPE_85_2, KC_VARIANT_NONE    },
  { "hc-900",   -1, KC_TYPE_85_2, KC_VARIANT_NONE    },
  { "kc85/2",    2, KC_TYPE_85_2, KC_VARIANT_NONE    },
  { "kc85/3",    3, KC_TYPE_85_3, KC_VARIANT_NONE    },
  { "kc85/4",    4, KC_TYPE_85_4, KC_VARIANT_NONE    },
  { "kc87",     -1, KC_TYPE_87,   KC_VARIANT_87_11   },
  { "kc87.10",  -1, KC_TYPE_87,   KC_VARIANT_87_10   },
  { "kc87.11",   7, KC_TYPE_87,   KC_VARIANT_87_11   },
  { "kc87.21",  -1, KC_TYPE_87,   KC_VARIANT_87_21   },
  { "lc80",      8, KC_TYPE_LC80, KC_VARIANT_NONE    },
  { NULL,       -1, KC_TYPE_NONE, KC_VARIANT_NONE    },
};

static void
banner(void)
{
  cout << ("   _  ______\n"
	   "  | |/ / ___|___ _ __ ___  _   _               KCemu " KCEMU_VERSION "\n"
	   "  | ' / |   / _ \\ '_ ` _ \\| | | |      (c) 1997-2002 Torsten Paul\n"
	   "  | . \\ |__|  __/ | | | | | |_| |        <Torsten.Paul@gmx.de>\n"
	   "  |_|\\_\\____\\___|_| |_| |_|\\__,_|     http://kcemu.sourceforge.net/\n");
}

static void
usage(char *argv0)
{
  banner();
  cout << _("\n"
	    "KCemu comes with ABSOLUTELY NO WARRANTY; for details run `kcemu --warranty'.\n"
	    "This is free software, and you are welcome to redistribute it\n"
	    "under certain conditions; run `kcemu --license' for details.\n"
	    "\n"
	    "usage: kcemu [-123478sthdlvLW]\n"
	    "\n"
	    "  -1:             run in Z9001 / KC 85/1 mode\n"
	    "  -2:             run in KC 85/2 mode\n"
	    "  -3:             run in KC 85/3 mode\n"
	    "  -4:             run in KC 85/4 mode (default)\n"
	    "  -7:             run in KC 87 mode (= KC 85/1 with color expansion)\n"
	    "  -8:             run in LC 80 mode\n"
	    "  -s --scale:     scale display (allowed values: 1, 2 and 3)\n"
	    "  -t --tape:      attach tape on startup\n"
	    "  -h --help:      display help\n"
	    "  -d --datadir:   set data directory (for ROM images)\n"
	    "  -l --localedir: set locale directory\n"
	    "  -v --version:   show KCemu version and configuration\n"
	    "  -L --license:   show license\n"
	    "  -W --warranty:  show warranty\n");

  exit(0);
}

static void
show_version(char *argv0)
{
  banner();
  cout << endl;
  fileio_show_config();
  cout << endl;
  libdisk_show_config();
  cout << endl;
  libaudio_show_config();
  exit(0);
}

void
license(char *argv0)
{
  banner();
  cout << _("\n"
	    "		    GNU GENERAL PUBLIC LICENSE\n"
	    "		       Version 2, June 1991\n"
	    "\n"
	    " Copyright (C) 1989, 1991 Free Software Foundation, Inc.\n"
	    "     59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
	    " Everyone is permitted to copy and distribute verbatim copies\n"
	    " of this license document, but changing it is not allowed.\n"
	    "\n"
	    "			    Preamble\n"
	    "\n"
	    "  The licenses for most software are designed to take away your\n"
	    "freedom to share and change it.  By contrast, the GNU General Public\n"
	    "License is intended to guarantee your freedom to share and change free\n"
	    "software--to make sure the software is free for all its users.  This\n"
	    "General Public License applies to most of the Free Software\n"
	    "Foundation's software and to any other program whose authors commit to\n"
	    "using it.  (Some other Free Software Foundation software is covered by\n"
	    "the GNU Library General Public License instead.)  You can apply it to\n"
	    "your programs, too.\n"
	    "\n"
	    "  When we speak of free software, we are referring to freedom, not\n"
	    "price.  Our General Public Licenses are designed to make sure that you\n"
	    "have the freedom to distribute copies of free software (and charge for\n"
	    "this service if you wish), that you receive source code or can get it\n"
	    "if you want it, that you can change the software or use pieces of it\n"
	    "in new free programs; and that you know you can do these things.\n"
	    "\n"
	    "  To protect your rights, we need to make restrictions that forbid\n"
	    "anyone to deny you these rights or to ask you to surrender the rights.\n"
	    "These restrictions translate to certain responsibilities for you if you\n"
	    "distribute copies of the software, or if you modify it.\n"
	    "\n"
	    "  For example, if you distribute copies of such a program, whether\n"
	    "gratis or for a fee, you must give the recipients all the rights that\n"
	    "you have.  You must make sure that they, too, receive or can get the\n"
	    "source code.  And you must show them these terms so they know their\n"
	    "rights.\n"
	    "\n"
	    "  We protect your rights with two steps: (1) copyright the software, and\n"
	    "(2) offer you this license which gives you legal permission to copy,\n"
	    "distribute and/or modify the software.\n"
	    "\n"
	    "  Also, for each author's protection and ours, we want to make certain\n"
	    "that everyone understands that there is no warranty for this free\n"
	    "software.  If the software is modified by someone else and passed on, we\n"
	    "want its recipients to know that what they have is not the original, so\n"
	    "that any problems introduced by others will not reflect on the original\n"
	    "authors' reputations.\n"
	    "\n"
	    "  Finally, any free program is threatened constantly by software\n"
	    "patents.  We wish to avoid the danger that redistributors of a free\n"
	    "program will individually obtain patent licenses, in effect making the\n"
	    "program proprietary.  To prevent this, we have made it clear that any\n"
	    "patent must be licensed for everyone's free use or not licensed at all.\n"
	    "\n"
	    "  The precise terms and conditions for copying, distribution and\n"
	    "modification follow.\n"
	    "\n"
	    "		    GNU GENERAL PUBLIC LICENSE\n"
	    "   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n"
	    "\n"
	    "  0. This License applies to any program or other work which contains\n"
	    "a notice placed by the copyright holder saying it may be distributed\n"
	    "under the terms of this General Public License.  The \"Program\", below,\n"
	    "refers to any such program or work, and a \"work based on the Program\"\n"
	    "means either the Program or any derivative work under copyright law:\n"
	    "that is to say, a work containing the Program or a portion of it,\n"
	    "either verbatim or with modifications and/or translated into another\n"
	    "language.  (Hereinafter, translation is included without limitation in\n"
	    "the term \"modification\".)  Each licensee is addressed as \"you\".\n"
	    "\n"
	    "Activities other than copying, distribution and modification are not\n"
	    "covered by this License; they are outside its scope.  The act of\n"
	    "running the Program is not restricted, and the output from the Program\n"
	    "is covered only if its contents constitute a work based on the\n"
	    "Program (independent of having been made by running the Program).\n"
	    "Whether that is true depends on what the Program does.\n"
	    "\n"
	    "  1. You may copy and distribute verbatim copies of the Program's\n"
	    "source code as you receive it, in any medium, provided that you\n"
	    "conspicuously and appropriately publish on each copy an appropriate\n"
	    "copyright notice and disclaimer of warranty; keep intact all the\n"
	    "notices that refer to this License and to the absence of any warranty;\n"
	    "and give any other recipients of the Program a copy of this License\n"
	    "along with the Program.\n"
	    "\n"
	    "You may charge a fee for the physical act of transferring a copy, and\n"
	    "you may at your option offer warranty protection in exchange for a fee.\n"
	    "\n"
	    "  2. You may modify your copy or copies of the Program or any portion\n"
	    "of it, thus forming a work based on the Program, and copy and\n"
	    "distribute such modifications or work under the terms of Section 1\n"
	    "above, provided that you also meet all of these conditions:\n"
	    "\n"
	    "    a) You must cause the modified files to carry prominent notices\n"
	    "    stating that you changed the files and the date of any change.\n"
	    "\n"
	    "    b) You must cause any work that you distribute or publish, that in\n"
	    "    whole or in part contains or is derived from the Program or any\n"
	    "    part thereof, to be licensed as a whole at no charge to all third\n"
	    "    parties under the terms of this License.\n"
	    "\n"
	    "    c) If the modified program normally reads commands interactively\n"
	    "    when run, you must cause it, when started running for such\n"
	    "    interactive use in the most ordinary way, to print or display an\n"
	    "    announcement including an appropriate copyright notice and a\n"
	    "    notice that there is no warranty (or else, saying that you provide\n"
	    "    a warranty) and that users may redistribute the program under\n"
	    "    these conditions, and telling the user how to view a copy of this\n"
	    "    License.  (Exception: if the Program itself is interactive but\n"
	    "    does not normally print such an announcement, your work based on\n"
	    "    the Program is not required to print an announcement.)\n"
	    "\n"
	    "These requirements apply to the modified work as a whole.  If\n"
	    "identifiable sections of that work are not derived from the Program,\n"
	    "and can be reasonably considered independent and separate works in\n"
	    "themselves, then this License, and its terms, do not apply to those\n"
	    "sections when you distribute them as separate works.  But when you\n"
	    "distribute the same sections as part of a whole which is a work based\n"
	    "on the Program, the distribution of the whole must be on the terms of\n"
	    "this License, whose permissions for other licensees extend to the\n"
	    "entire whole, and thus to each and every part regardless of who wrote it.\n"
	    "\n"
	    "Thus, it is not the intent of this section to claim rights or contest\n"
	    "your rights to work written entirely by you; rather, the intent is to\n"
	    "exercise the right to control the distribution of derivative or\n"
	    "collective works based on the Program.\n"
	    "\n"
	    "In addition, mere aggregation of another work not based on the Program\n"
	    "with the Program (or with a work based on the Program) on a volume of\n"
	    "a storage or distribution medium does not bring the other work under\n"
	    "the scope of this License.\n"
	    "\n"
	    "  3. You may copy and distribute the Program (or a work based on it,\n"
	    "under Section 2) in object code or executable form under the terms of\n"
	    "Sections 1 and 2 above provided that you also do one of the following:\n"
	    "\n"
	    "    a) Accompany it with the complete corresponding machine-readable\n"
	    "    source code, which must be distributed under the terms of Sections\n"
	    "    1 and 2 above on a medium customarily used for software interchange; or,\n"
	    "\n"
	    "    b) Accompany it with a written offer, valid for at least three\n"
	    "    years, to give any third party, for a charge no more than your\n"
	    "    cost of physically performing source distribution, a complete\n"
	    "    machine-readable copy of the corresponding source code, to be\n"
	    "    distributed under the terms of Sections 1 and 2 above on a medium\n"
	    "    customarily used for software interchange; or,\n"
	    "\n"
	    "    c) Accompany it with the information you received as to the offer\n"
	    "    to distribute corresponding source code.  (This alternative is\n"
	    "    allowed only for noncommercial distribution and only if you\n"
	    "    received the program in object code or executable form with such\n"
	    "    an offer, in accord with Subsection b above.)\n"
	    "\n"
	    "The source code for a work means the preferred form of the work for\n"
	    "making modifications to it.  For an executable work, complete source\n"
	    "code means all the source code for all modules it contains, plus any\n"
	    "associated interface definition files, plus the scripts used to\n"
	    "control compilation and installation of the executable.  However, as a\n"
	    "special exception, the source code distributed need not include\n"
	    "anything that is normally distributed (in either source or binary\n"
	    "form) with the major components (compiler, kernel, and so on) of the\n"
	    "operating system on which the executable runs, unless that component\n"
	    "itself accompanies the executable.\n"
	    "\n"
	    "If distribution of executable or object code is made by offering\n"
	    "access to copy from a designated place, then offering equivalent\n"
	    "access to copy the source code from the same place counts as\n"
	    "distribution of the source code, even though third parties are not\n"
	    "compelled to copy the source along with the object code.\n"
	    "\n"
	    "  4. You may not copy, modify, sublicense, or distribute the Program\n"
	    "except as expressly provided under this License.  Any attempt\n"
	    "otherwise to copy, modify, sublicense or distribute the Program is\n"
	    "void, and will automatically terminate your rights under this License.\n"
	    "However, parties who have received copies, or rights, from you under\n"
	    "this License will not have their licenses terminated so long as such\n"
	    "parties remain in full compliance.\n"
	    "\n"
	    "  5. You are not required to accept this License, since you have not\n"
	    "signed it.  However, nothing else grants you permission to modify or\n"
	    "distribute the Program or its derivative works.  These actions are\n"
	    "prohibited by law if you do not accept this License.  Therefore, by\n"
	    "modifying or distributing the Program (or any work based on the\n"
	    "Program), you indicate your acceptance of this License to do so, and\n"
	    "all its terms and conditions for copying, distributing or modifying\n"
	    "the Program or works based on it.\n"
	    "\n"
	    "  6. Each time you redistribute the Program (or any work based on the\n"
	    "Program), the recipient automatically receives a license from the\n"
	    "original licensor to copy, distribute or modify the Program subject to\n"
	    "these terms and conditions.  You may not impose any further\n"
	    "restrictions on the recipients' exercise of the rights granted herein.\n"
	    "You are not responsible for enforcing compliance by third parties to\n"
	    "this License.\n"
	    "\n"
	    "  7. If, as a consequence of a court judgment or allegation of patent\n"
	    "infringement or for any other reason (not limited to patent issues),\n"
	    "conditions are imposed on you (whether by court order, agreement or\n"
	    "otherwise) that contradict the conditions of this License, they do not\n"
	    "excuse you from the conditions of this License.  If you cannot\n"
	    "distribute so as to satisfy simultaneously your obligations under this\n"
	    "License and any other pertinent obligations, then as a consequence you\n"
	    "may not distribute the Program at all.  For example, if a patent\n"
	    "license would not permit royalty-free redistribution of the Program by\n"
	    "all those who receive copies directly or indirectly through you, then\n"
	    "the only way you could satisfy both it and this License would be to\n"
	    "refrain entirely from distribution of the Program.\n"
	    "\n"
	    "If any portion of this section is held invalid or unenforceable under\n"
	    "any particular circumstance, the balance of the section is intended to\n"
	    "apply and the section as a whole is intended to apply in other\n"
	    "circumstances.\n"
	    "\n"
	    "It is not the purpose of this section to induce you to infringe any\n"
	    "patents or other property right claims or to contest validity of any\n"
	    "such claims; this section has the sole purpose of protecting the\n"
	    "integrity of the free software distribution system, which is\n"
	    "implemented by public license practices.  Many people have made\n"
	    "generous contributions to the wide range of software distributed\n"
	    "through that system in reliance on consistent application of that\n"
	    "system; it is up to the author/donor to decide if he or she is willing\n"
	    "to distribute software through any other system and a licensee cannot\n"
	    "impose that choice.\n"
	    "\n"
	    "This section is intended to make thoroughly clear what is believed to\n"
	    "be a consequence of the rest of this License.\n"
	    "\n"
	    "  8. If the distribution and/or use of the Program is restricted in\n"
	    "certain countries either by patents or by copyrighted interfaces, the\n"
	    "original copyright holder who places the Program under this License\n"
	    "may add an explicit geographical distribution limitation excluding\n"
	    "those countries, so that distribution is permitted only in or among\n"
	    "countries not thus excluded.  In such case, this License incorporates\n"
	    "the limitation as if written in the body of this License.\n"
	    "\n"
	    "  9. The Free Software Foundation may publish revised and/or new versions\n"
	    "of the General Public License from time to time.  Such new versions will\n"
	    "be similar in spirit to the present version, but may differ in detail to\n"
	    "address new problems or concerns.\n"
	    "\n"
	    "Each version is given a distinguishing version number.  If the Program\n"
	    "specifies a version number of this License which applies to it and \"any\n"
	    "later version\", you have the option of following the terms and conditions\n"
	    "either of that version or of any later version published by the Free\n"
	    "Software Foundation.  If the Program does not specify a version number of\n"
	    "this License, you may choose any version ever published by the Free Software\n"
	    "Foundation.\n"
	    "\n"
	    "  10. If you wish to incorporate parts of the Program into other free\n"
	    "programs whose distribution conditions are different, write to the author\n"
	    "to ask for permission.  For software which is copyrighted by the Free\n"
	    "Software Foundation, write to the Free Software Foundation; we sometimes\n"
	    "make exceptions for this.  Our decision will be guided by the two goals\n"
	    "of preserving the free status of all derivatives of our free software and\n"
	    "of promoting the sharing and reuse of software generally.\n"
	    "\n"
	    "			    NO WARRANTY\n"
	    "\n"
	    "  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
	    "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
	    "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
	    "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
	    "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
	    "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
	    "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
	    "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
	    "REPAIR OR CORRECTION.\n"
	    "\n"
	    "  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
	    "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
	    "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
	    "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
	    "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
	    "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
	    "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
	    "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
	    "POSSIBILITY OF SUCH DAMAGES.\n"
	    "\n"
	    "		     END OF TERMS AND CONDITIONS\n"
	    "\n"
	    "	    How to Apply These Terms to Your New Programs\n"
	    "\n"
	    "  If you develop a new program, and you want it to be of the greatest\n"
	    "possible use to the public, the best way to achieve this is to make it\n"
	    "free software which everyone can redistribute and change under these terms.\n"
	    "\n"
	    "  To do so, attach the following notices to the program.  It is safest\n"
	    "to attach them to the start of each source file to most effectively\n"
	    "convey the exclusion of warranty; and each file should have at least\n"
	    "the \"copyright\" line and a pointer to where the full notice is found.\n"
	    "\n"
	    "    <one line to give the program's name and a brief idea of what it does.>\n"
	    "    Copyright (C) <year>  <name of author>\n"
	    "\n"
	    "    This program is free software; you can redistribute it and/or modify\n"
	    "    it under the terms of the GNU General Public License as published by\n"
	    "    the Free Software Foundation; either version 2 of the License, or\n"
	    "    (at your option) any later version.\n"
	    "\n"
	    "    This program is distributed in the hope that it will be useful,\n"
	    "    but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	    "    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	    "    GNU General Public License for more details.\n"
	    "\n"
	    "    You should have received a copy of the GNU General Public License\n"
	    "    along with this program; if not, write to the Free Software\n"
	    "    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
	    "\n"
	    "\n"
	    "Also add information on how to contact you by electronic and paper mail.\n"
	    "\n"
	    "If the program is interactive, make it output a short notice like this\n"
	    "when it starts in an interactive mode:\n"
	    "\n"
	    "    Gnomovision version 69, Copyright (C) year  name of author\n"
	    "    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
	    "    This is free software, and you are welcome to redistribute it\n"
	    "    under certain conditions; type `show c' for details.\n"
	    "\n"
	    "The hypothetical commands `show w' and `show c' should show the appropriate\n"
	    "parts of the General Public License.  Of course, the commands you use may\n"
	    "be called something other than `show w' and `show c'; they could even be\n"
	    "mouse-clicks or menu items--whatever suits your program.\n"
	    "\n"
	    "You should also get your employer (if you work as a programmer) or your\n"
	    "school, if any, to sign a \"copyright disclaimer\" for the program, if\n"
	    "necessary.  Here is a sample; alter the names:\n"
	    "\n"
	    "  Yoyodyne, Inc., hereby disclaims all copyright interest in the program\n"
	    "  `Gnomovision' (which makes passes at compilers) written by James Hacker.\n"
	    "\n"
	    "  <signature of Ty Coon>, 1 April 1989\n"
	    "  Ty Coon, President of Vice\n"
	    "\n"
	    "This General Public License does not permit incorporating your program into\n"
	    "proprietary programs.  If your program is a subroutine library, you may\n"
	    "consider it more useful to permit linking proprietary applications with the\n"
	    "library.  If this is what you want to do, use the GNU Library General\n"
	    "Public License instead of this License.\n\n");

  exit(0);
}

void
warranty(char *argv0)
{
  banner();
  cout << _("\n"
	    "NO WARRANTY\n"
	    "\n"
	    "  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n"
	    "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n"
	    "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n"
	    "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n"
	    "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
	    "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n"
	    "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n"
	    "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n"
	    "REPAIR OR CORRECTION.\n"
	    "\n"
	    "  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n"
	    "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n"
	    "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n"
	    "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n"
	    "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n"
	    "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n"
	    "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n"
	    "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n"
	    "POSSIBILITY OF SUCH DAMAGES.\n\n");

  exit(0);
}

kc_type_t
get_kc_type(void)
{
  return kcemu_kc_type;
}

kc_variant_t
get_kc_variant(void)
{
  return kcemu_kc_variant;
}

const char *
get_kc_variant_name(void)
{
  return kcemu_kc_variant_name;
}

void
set_kc_type(int type, const char *variant)
{
  int a;

  /*
   *  if no model type switch (-1, -2, ...) and no name (-e kc85/1) on
   *  the command line we use the value from the config file
   */
  if ((type < 0) && (variant == NULL))
    {
      /*
       *  for historical reasons we check for an integer value first
       *  as this was defined for older emulator versions
       *  if it's not an integer we take it to be the model name of
       *  the computer to emulate
       */
      type = RC::instance()->get_int("Default KC Model", -1);
      if (type < 0)
	variant = RC::instance()->get_string("Default KC Model", NULL);
    }

  /*
   *  check model names
   */
  if (variant != NULL)
    {
      for (a = 0;kc_types[a].name != NULL;a++)
	{
	  if (strcasecmp(variant, kc_types[a].name) == 0)
	    {
	      kcemu_kc_type = kc_types[a].kc_type;
	      kcemu_kc_variant = kc_types[a].kc_variant;
	      kcemu_kc_variant_name = kc_types[a].name;
	      return;
	    }
	}
    }

  /*
   *  if no name is given or the name is unknown check type switch;
   *  default is type 4
   */
  if (type < 0)
    type = 4;

  for (a = 0;kc_types[a].name != NULL;a++)
    {
      if (kc_types[a].type == type)
	{
	  kcemu_kc_type = kc_types[a].kc_type;
	  kcemu_kc_variant = kc_types[a].kc_variant;
	  kcemu_kc_variant_name = kc_types[a].name;
	  return;
	}
    }

  /*
   *  we should never come to this point but just in case default
   *  to kc85/4
   */
  kcemu_kc_type = KC_TYPE_85_4;
  kcemu_kc_variant = KC_VARIANT_NONE;
  kcemu_kc_variant_name = "kc85/4";
}

int
main(int argc, char **argv)
{
  int c;
  int type;
  char *ptr;
  bool do_quit;
  const char *tmp;
  int option_index;
#ifdef HAVE_GETOPT_LONG
  static struct option long_options[] =
  {
    { "help", 0, 0, 'h' },
    { "tape", 1, 0, 't' },
    { "scale", 1, 0, 's' },
    { "emulate", 1, 0, 'e' },
    { "datadir", 1, 0, 'd' },
    { "localedir", 1, 0, 'l' },
    { "license", 0, 0, 'L' },
    { "warranty", 0, 0, 'W' },
    { "version", 0, 0, 'v' },
    { "display-debug", 0, 0, 'D' },
    { 0, 0, 0, 0 }
  };
#endif /* HAVE_GETOPT_LONG */
  PortGroup *portg = NULL;
  PortInterface *porti = NULL;
  LOG *log;

#ifdef __CALL_MTRACE
  /*
   *  memory debugging with glibc goes like this:
   *
   *  1) run the program with MALLOC_CHECK_=1 ./kcemu to enable
   *     consistency checks (MALLOC_CHECK_=2 aborts immedately
   *     when encountering problems)
   *
   *  2) call mtrace() at the very beginning of the program and
   *     run it with MALLOC_TRACE=/tmp/mem.out ./kcemu to
   *     generate a memory trace that can be analyzed with
   *     mtrace ./kcemu /tmp/mem.out
   *
   *  The following mtrace is enababled if ./configure finds
   *  the mcheck.h header file, the mtrace() function and
   *  the debuglevel is set to a value greater than 1.
   */
  mtrace();
#endif /* __CALL_MTRACE */

  type = -1;
  kcemu_tape = 0;
  kcemu_emulate = 0;
  kcemu_ui_scale = 0;
  kcemu_ui_debug = -1;
  ptr = getenv("KCEMU_DATADIR");
  kcemu_datadir = (ptr) ? strdup(ptr) : strdup(DATADIR);
  ptr = getenv("KCEMU_LOCALEDIR");
  kcemu_localedir = (ptr) ? strdup(ptr) : strdup(LOCALEDIR);

#ifdef HAVE_SETLOCALE
  setlocale(LC_ALL, "");
#endif /* HAVE_SETLOCALE */
#ifdef ENABLE_NLS
  bindtextdomain(PACKAGE, kcemu_localedir);
  textdomain(PACKAGE);
#endif /* ENABLE_NLS */

  fileio_init();
  libdisk_init();
  libaudio_init(LIBAUDIO_TYPE_ALL);

  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      c = getopt_long(argc, argv, "123478hvDe:d:l:s:t:LW",
                      long_options, &option_index);
#else
#ifdef HAVE_GETOPT
      c = getopt(argc, argv, "123478hvDe:d:l:s:LW");
#else
#warning neither HAVE_GETOPT_LONG nor HAVE_GETOPT defined
#warning commandline parsing disabled!
      c = -1;
#endif /* HAVE_GETOPT */
#endif /* #ifdef HAVE_GETOPT_LONG */

      if (c == -1)
	break;

      switch (c)
        {
	case '1':
	  type = 1;
	  break;
        case '2':
          type = 2;
          break;
        case '3':
          type = 3;
          break;
        case '4':
          type = 4;
          break;
        case '7':
          type = 7;
          break;
	case '8':
	  type = 8;
	  break;
	case 'e':
	  kcemu_emulate = strdup(optarg);
	  break;
        case 'd':
          free(kcemu_datadir);
          kcemu_datadir = strdup(optarg);
          break;
        case 'l':
          free(kcemu_localedir);
          kcemu_localedir = strdup(optarg);
          break;
	case 's':
	  kcemu_ui_scale = strtoul(optarg, NULL, 0);
	  break;
	case 't':
	  kcemu_tape = strdup(optarg);
	  break;
	case 'L':
	  license(argv[0]);
	  break;
	case 'W':
	  warranty(argv[0]);
	  break;
	case 'D':
	  kcemu_ui_debug = 1;
	  break;
	case 'v':
	  show_version(argv[0]);
	  break;
        case ':':
        case '?':
        case 'h':
        default:
          usage(argv[0]);
          break;
        }
    }

#ifdef MEMORY_SLOW_ACCESS
  cout << "DEBUG: MEMORY_SLOW_ACCESS" << endl;
#endif /* MEMORY_SLOW_ACCESS */
#ifdef PROFILE_WINDOW
  cout << "DEBUG: PROFILE_WINDOW" << endl;
#endif /* PROFILE_WINDOW */

  RC::init();

  set_kc_type(type, kcemu_emulate);

  /*
   *  check display scale
   */
  if (kcemu_ui_scale == 0)
    kcemu_ui_scale = RC::instance()->get_int("Display Scale", 1);
  
  if (kcemu_ui_scale < 1)
    kcemu_ui_scale = 1;
  if (kcemu_ui_scale > 3)
    kcemu_ui_scale = 3;

  /*
   *  check display debug
   */
  if (kcemu_ui_debug < 0)
    kcemu_ui_debug = RC::instance()->get_int("Display Debug", 0);

  do
    {
      cmd   = new CMD("*");
      z80   = new Z80;
      ports = new Ports;
      portg = ports->register_ports("-", 0, 0x100, new NullPort, 256);

      PIO1_1 *p1;
      PIO2 *p2;
      PIO3 *p3;
      PIO4 *p4;
      PIO8_1 *p8;
      Keyboard1 *k1;
      Keyboard8 *k8;

      timer = NULL;
      memory = NULL;
      fdc_z80 = NULL;
      disk = NULL;

      switch (kcemu_kc_type)
	{
	case KC_TYPE_87:
	  memory   = new Memory7;
	  /* fall through */
	case KC_TYPE_85_1:
	  if (memory == NULL)
	    memory   = new Memory1;

	  fileio_set_kctype(FILEIO_KC85_1);
	  ui       = new UI_1;
	  ctc      = new CTC1;
	  pio2     = new PIO1_2;
	  tape     = new Tape(500, 1000, 2000, 0);
	  wav      = new WavPlayer(500, 1000, 2000);

	  p1 = new PIO1_1;
	  k1 = new Keyboard1;
	  tape->set_tape_callback(p1);

	  pio = p1;
	  keyboard = k1;

	  //tape->setAutoplay(false);
	  pio2->register_callback_A_in(k1);
	  pio2->register_callback_B_in(k1);
	  break;
	case KC_TYPE_85_2:
	  memory   = new Memory2;
	  ui       = new UI_3;
	  
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  wav      = new WavPlayer(364, 729, 1458);
	  keyboard = new Keyboard3;
	  p2       = new PIO2;
	  tape->set_tape_callback(p2);
	  pio = p2;
	  break;
	case KC_TYPE_85_3:
	  memory   = new Memory3;
	  ui       = new UI_3;
	  
	  p3       = new PIO3;
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  wav      = new WavPlayer(364, 729, 1458);
	  keyboard = new Keyboard3;
	  tape->set_tape_callback(p3);
	  pio = p3;
	  break;
	case KC_TYPE_85_4:
	  memory   = new Memory4;
	  ui       = new UI_4;
	  porti    = new Ports4;
	  p4       = new PIO4;
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  wav      = new WavPlayer(364, 729, 1458);
	  keyboard = new Keyboard3;
	  tape->set_tape_callback(p4);
	  pio      = p4;
	  break;
	case KC_TYPE_LC80:
	  ui       = new UI_8;
	  memory   = new Memory8;
	  p8       = new PIO8_1;
	  pio2     = new PIO8_2;
	  ctc      = new CTC8;
	  tape     = new Tape(500, 1000, 2000, 0);
	  wav      = new WavPlayer(500, 1000, 2000);

	  k8       = new Keyboard8;
	  pio2->register_callback_A_in(k8);
	  pio2->register_callback_B_in(k8);
	  tape->set_tape_callback(p8);
	  keyboard = k8;
	  pio = p8;
	  break;
	default:
	  break;
	}

      module      = new Module;
      module_list = new ModuleList;
      
      switch (kcemu_kc_type)
	{
	case KC_TYPE_87:
	case KC_TYPE_85_1:
	  timer = new Timer1;

	  portg = ports->register_ports("CTC",  0x80, 4, ctc,  10);
	  portg = ports->register_ports("PIO1", 0x88, 4, pio,  10);
	  portg = ports->register_ports("PIO2", 0x90, 4, pio2, 10);
          /*
           *  build interrupt daisy chain
           */
	  ctc->next(pio->get_first());
	  pio->next(pio2->get_first());
          pio2->next(0);
          ctc->iei(1);
          z80->daisy_chain_set_first(pio->get_first()); // highest priority
          z80->daisy_chain_set_last(pio->get_last());  // lowest priority
	  break;
	case KC_TYPE_85_2:
	case KC_TYPE_85_3:
	  timer = new Timer3;
	  sound = new Sound3;
	  if (RC::instance()->get_int("Enable Sound"))
	    sound->init();

	  portg = ports->register_ports("Module", 0x80, 1, module, 10);
	  portg = ports->register_ports("PIO",    0x88, 4, pio,    10);
	  portg = ports->register_ports("CTC",    0x8c, 4, ctc,    10);
          /*
           *  build interrupt daisy chain
           */
          ctc->next(pio->get_first());
          pio->next(0);
          ctc->iei(1);
          z80->daisy_chain_set_first(ctc->get_first()); // highest priority
          z80->daisy_chain_set_last(ctc->get_last());  // lowest priority
	  break;
	case KC_TYPE_85_4:
	  timer = new Timer3;
	  sound = new Sound3;
	  if (RC::instance()->get_int("Enable Sound"))
	    sound->init();

	  portg = ports->register_ports("Module", 0x80, 1, module, 10);
	  portg = ports->register_ports("Port84", 0x84, 1, porti,  10);
	  portg = ports->register_ports("Port86", 0x86, 1, porti,  10);
	  portg = ports->register_ports("PIO",    0x88, 4, pio,    10);
	  portg = ports->register_ports("CTC",    0x8c, 4, ctc,    10);
          /*
           *  build interrupt daisy chain
           */
          ctc->next(pio->get_first());
          pio->next(0);
          ctc->iei(1);
          z80->daisy_chain_set_first(ctc->get_first()); // highest priority
          z80->daisy_chain_set_last(ctc->get_last());  // lowest priority
	  break;
	case KC_TYPE_LC80:
	  sound = new Sound8;
	  if (RC::instance()->get_int("Enable Sound"))
	    sound->init();

	  portg = ports->register_ports("CTC",  0xec, 4, ctc,  10);
	  portg = ports->register_ports("PIO1", 0xf4, 4, pio,  10);
	  portg = ports->register_ports("PIO2", 0xf8, 4, pio2, 10);
          /*
           *  build interrupt daisy chain
           */
	  ctc->next(pio->get_first());
	  pio->next(pio2->get_first());
          pio2->next(0);
          ctc->iei(1);
          z80->daisy_chain_set_first(pio->get_first()); // highest priority
          z80->daisy_chain_set_last(pio->get_last());  // lowest priority
	  break;
	default:
	  break;
	}
      
      if (get_kc_type() & KC_TYPE_85_2_CLASS)
	if (RC::instance()->get_int("Floppy Disk Basis"))
	  {
	    fdc_z80 = new Z80_FDC();
	    fdc_ports = new Ports();
	    fdc_io = new FloppyIO();
	    fdc_shmem = new FloppySharedMem();
	    fdc_shmem->set_memory(&fdc_mem[0xfc00]);
	    fdc_fdc = new FDC();
	    fdc_ctc = new CTC_FDC();
	    
	    fdc_ports->register_ports("-", 0, 0x100, new NullPort(), 256);
	    fdc_ports->register_ports("FDC", 0xf0, 12, fdc_fdc, 10);
	    fdc_ports->register_ports("CTC", 0xfc, 4, fdc_ctc, 10);
	    
	    portg = ports->register_ports("FloppyIO", 0xf4, 1, fdc_io, 10);
	    portg = ports->register_ports("FloppySHMEM", 0xf0, 4, fdc_shmem, 10);
	    
	    disk = new Disk();
	  }
      
      log = new LOG();
      ui->init(&argc, &argv);
      module_list->init();

      if ((kcemu_kc_type == KC_TYPE_85_1) ||
	  (kcemu_kc_type == KC_TYPE_87) ||
	  (kcemu_kc_type == KC_TYPE_LC80))
	tape->power(true);

      if (kcemu_tape != 0)
	{
	  tape->attach(kcemu_tape);
	  free(kcemu_tape);
	}
      else
	{
	  tmp = RC::instance()->get_string("Tape File", "files.kct");
	  if (tmp)
	    {
	      ptr = (char *)malloc(strlen(kcemu_datadir) + strlen(tmp) + 2);
	      if (tmp[0] == '/')
		strcpy(ptr, tmp);
	      else
		{
		  strcpy(ptr, kcemu_datadir);
		  strcat(ptr, "/");
		  strcat(ptr, tmp);
		}
	      tape->attach(ptr);
	      free(ptr);
	    }
	}

      do_quit = z80->run();
      
      if (porti != NULL)
	delete porti;

      if (timer != NULL)
	delete timer;
      
      delete module_list;
      delete module;
      delete keyboard;
      delete tape;
      delete disk;
      delete pio;
      delete ctc;
      delete ports;
      delete ui;
      delete memory;
      delete z80;
    }
  while (0); // (!do_quit);

  RC::done();
      
  free(kcemu_datadir);
  free(kcemu_localedir);
  
  return 0;
}
