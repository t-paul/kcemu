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
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "kc/system.h"
#include "kc/prefs/types.h"

#ifdef HOST_OS_MINGW
#ifdef HAVE_LIBSDL
#include "SDL.h"
#endif /* HAVE_LIBSDL */
#include <windows.h>
#endif /* HOST_OS_MINGW */

#ifdef USE_INCLUDED_GETOPT
#include "getopt/getopt.h"
#else
#ifdef HAVE_GETOPT
#include <getopt.h>
#endif
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
#include "kc/ctc.h"
#include "kc/pio.h"
#include "kc/z80.h"
#include "kc/wav.h"
#include "kc/tape.h"
#include "kc/disk.h"
#include "kc/daisy.h"
#include "kc/timer.h"
#include "kc/module.h"
#include "kc/mod_list.h"
#include "kc/sh_mem.h"
#include "kc/disk_io.h"
#include "kc/fdc4.h"
#include "kc/fdc9.h"
#include "kc/sound.h"
#include "kc/ctc_fdc.h"
#include "kc/z80_fdc.h"
#include "kc/gide.h"
#include "kc/vdip.h"
#include "kc/plotter.h"
#include "kc/poly880.h"
#include "kc/prefs/prefs.h"
#include "kc/kcnet/kcnet.h"

#include "ui/log.h"
#include "cmd/cmd.h"
#include "fileio/load.h"
#include "libdisk/libdisk.h"
#include "libaudio/libaudio.h"

#include "kc/pio0.h"
#include "kc/keyb0.h"
#include "kc/ports0.h"
#include "kc/memory0.h"

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

#include "kc/memory5.h"

#include "kc/ctc6.h"
#include "kc/pio6.h"
#include "kc/keyb6.h"
#include "kc/ports6.h"
#include "kc/memory6.h"

#include "kc/ctc8.h"
#include "kc/pio8.h"
#include "kc/keyb8.h"
#include "kc/sound8.h"
#include "kc/memory8.h"

#include "kc/gdc.h"
#include "kc/vis.h"
#include "kc/svg.h"
#include "kc/ctc9.h"
#include "kc/pio9.h"
#include "kc/keyb9.h"
#include "kc/memory9.h"

#include "kc/kramermc/pio.h"
#include "kc/kramermc/memory.h"
#include "kc/kramermc/keyboard.h"

#include "kc/muglerpc/ctc.h"
#include "kc/muglerpc/pio.h"
#include "kc/muglerpc/ports.h"
#include "kc/muglerpc/memory.h"
#include "kc/muglerpc/keyboard.h"

#include "kc/vcs80/pio.h"
#include "kc/vcs80/memory.h"
#include "kc/vcs80/keyboard.h"

#include "kc/c80/pio.h"
#include "kc/c80/memory.h"
#include "kc/c80/display.h"
#include "kc/c80/keyboard.h"

#include "sys/sysdep.h"

# include "ui/gtk/ui_gtk.h"

#include "libdbg/dbg.h"

using namespace std;

Z80        *z80;
DaisyChain *daisy;
UI         *ui;
Ports      *ports;
Ports      *ports_fdc;
Module     *module;
Memory     *memory;
CTC        *ctc;
CTC        *ctc2;
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
PortInterface *porti;
GDC           *gdc;
VIS           *vis;
SVG           *svg;
VDIP          *vdip;
KCNET         *kcnet;
Poly880       *poly880;
DisplayC80    *display_c80;
Plotter       *plotter;

Z80_FDC         *fdc_z80;
FloppyIO        *fdc_io;
FloppySharedMem *fdc_shmem;
Ports           *fdc_ports;
FDC             *fdc_fdc;
CTC             *fdc_ctc;
GIDE            *fdc_gide;

int   kcemu_ui_scale;
int   kcemu_ui_debug;
int   kcemu_ui_fullscreen;
int   kcemu_ui_display_effect;
int   kcemu_select_profile;
char *kcemu_homedir;
char *kcemu_datadir;
char *kcemu_localedir;
char *kcemu_debug_output;
char *kcemu_tape;
char *kcemu_disk;
char *kcemu_modules;
char *kcemu_autostart_file;
char *kcemu_autostart_addr;
char *kcemu_vdip_root;
char *kcemu_profile;

static void
banner(void)
{
  cout << ("   _  ______\n"
	   "  | |/ / ___|___ _ __ ___  _   _               KCemu " KCEMU_VERSION "\n"
	   "  | ' / |   / _ \\ '_ ` _ \\| | | |      (c) 1997-2010 Torsten Paul\n"
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
	    "usage: kcemu [-0123456789aAcestfhdlvVHFMELW]\n"
	    "\n"
	    "  -0:                  select Z1013 profile\n"
	    "  -1:                  select Z9001 / KC 85/1 profile\n"
	    "  -2:                  select KC 85/2 profile\n"
	    "  -3:                  select KC 85/3 profile\n"
	    "  -4:                  select KC 85/4 profile\n"
	    "  -5:                  select KC 85/5 profile\n"
	    "  -6:                  select Poly-880 profile\n"
	    "  -7:                  select KC 87 profile\n"
	    "  -8:                  select LC 80 profile\n"
	    "  -9:                  select BIC/A5105 profile\n"
	    "  -a --autostart:      autostart program on startup (kc85/3 - kc85/5 only)\n"
	    "  -A --address:        override start address of autostart program\n"
	    "  -d --datadir:        set data directory (for ROM images)\n"
	    "  -E --effects:        enable display effects for scaled screen output\n"
	    "  -f --floppy:         attach disk on startup\n"
	    "  -F --fullscreen:     start in fullscreen mode (if supported by gui)\n"
	    "  -h --help:           display help\n"
	    "  -H --home:           overwrite setting for home directory\n"
	    "  -l --localedir:      set locale directory\n"
	    "  -L --license:        show license\n"
            "  -M --modules:        insert modules on startup (e.g. -M M011,M027)\n"
	    "  -o --debug-output:   write debug output to file\n"
            "  -p --profile:        start with given profile\n"
            "  -P --select-profile: show profile selector\n"
	    "  -s --scale:          scale display (allowed values: 1, 2 and 3)\n"
	    "  -t --tape:           attach tape on startup\n"
	    "  -v --version:        show KCemu version and configuration\n"
	    "  -V --viewlist:       view verbose list of available emulations\n"
	    "  -W --warranty:       show warranty\n");

  exit(0);
}

static void
show_version(char *argv0)
{
  banner();
  cout << endl;
  SystemInformation::instance()->show_types();
  cout << endl;
  fileio_show_config();
  cout << endl;
  libdisk_show_config();
  cout << endl;
  libaudio_show_config();
  exit(0);
}

static void
show_variants(char *argv0)
{
  banner();
  cout << endl;
  SystemInformation::instance()->show_types_with_description();
  exit(0);
}

const char *
kc_get_license(void)
{
  return _("\n"
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
	   "of promoting the sharing and reuse of software generally.\n\n");
}

const char *
kc_get_warranty(void)
{
  return _("\n"
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
}

const char *
kc_get_license_trailer(void)
{
  return _("\n"
	   "                     END OF TERMS AND CONDITIONS\n"
	   "\n\n"
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
	   "    Copyright (C) 19yy  <name of author>\n"
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
	   "    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n"
	   "\n"
	   "Also add information on how to contact you by electronic and paper mail.\n"
	   "\n"
	   "If the program is interactive, make it output a short notice like this\n"
	   "when it starts in an interactive mode:\n"
	   "\n"
	   "    Gnomovision version 69, Copyright (C) 19yy name of author\n"
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
}

void
license(char *argv0)
{
  banner();
  cout << kc_get_license();
  cout << kc_get_warranty();
  cout << kc_get_license_trailer();
  exit(0);
}

void
warranty(char *argv0)
{
  banner();
  cout << kc_get_warranty();
  exit(0);
}

void
attach_tape(void)
{
  if (kcemu_tape != 0)
    {
      tape->attach(kcemu_tape);
      free(kcemu_tape);
    }
  else
    {
      const char *tmp = Preferences::instance()->get_string_value("tape_file", "files.kct");
      if (tmp)
	{
	  string tapefile;
	  
	  if (tmp[0] != '/')
	    {
	      tapefile += kcemu_datadir;
	      tapefile += "/tapes/";
	    }
	  tapefile += tmp;
	  tape->attach(tapefile.c_str());
	}
    }
}

void
attach_disk(void)
{
  const char *filename;

  if (disk == NULL)
    return;

  if (kcemu_disk != NULL)
    {
      disk->attach(0, kcemu_disk);
      free(kcemu_disk);
    }
  else
    {
      filename = Preferences::instance()->get_string_value("disk_1", NULL);
      if (filename != NULL)
	disk->attach(0, filename);
    }

  filename = Preferences::instance()->get_string_value("disk_2", NULL);
  if (filename != NULL)
    disk->attach(1, filename);

  filename = Preferences::instance()->get_string_value("disk_3", NULL);
  if (filename != NULL)
    disk->attach(2, filename);

  filename = Preferences::instance()->get_string_value("disk_4", NULL);
  if (filename != NULL)
    disk->attach(3, filename);
}

char *
get_datadir_from_program_path(void)
{
  char *ptr = sys_getprogrampath();
  if (ptr == NULL)
    return NULL;

  int a = strlen(ptr);
  if (a < 4)
    {
      free(ptr);
      return NULL;
    }

  if (strncasecmp(ptr + a - 4, "bin", 3) != 0)
    {
      free(ptr);
      return NULL;
    }

  ptr[a - 4] = '\0';
  char sep = ptr[a - 5];

  char buf[2048];
  int size = sizeof(buf);
  if (snprintf(buf, size, "%sshare%cKCemu", ptr, sep) >= size)
    {
      free(ptr);
      return NULL;
    }

  return strdup(buf);
}

char *
get_kcemu_datadir(void)
{
  char *ptr = getenv("KCEMU_DATADIR");
  if (ptr)
    return strdup(ptr);

  ptr = get_datadir_from_program_path();
  if (ptr != NULL)
    return ptr;

  if (access("share/KCemu/roms/kc85/caos__c0.854", F_OK) == 0)
    return strdup("share/KCemu");

  if (access("../share/KCemu/roms/kc85/caos__c0.854", F_OK) == 0)
    return strdup("../share/KCemu");

  return strdup(KCEMU_DATADIR);
}

char *
get_kcemu_homedir(void)
{
    string home;
    
    char *ptr = getenv("XDG_CONFIG_HOME");
    if (ptr != NULL) {
        home = ptr;
    } else {
        ptr = sys_gethome();
        if (ptr != NULL) {
            home = ptr;
            home += "/.config";
            free(ptr);
        } else {
            return NULL;
        }
    }
    
    home += "/KCemu";

    return strdup(home.c_str());
}

char *
get_kcemu_localedir(void)
{
  char *ptr = getenv("KCEMU_LOCALEDIR");
  if (ptr)
    return strdup(ptr);

  if (access("share/locale/de/LC_MESSAGES/KCemu.mo", F_OK) == 0)
    return strdup("share/locale");

  if (access("../share/locale/de/LC_MESSAGES/KCemu.mo", F_OK) == 0)
    return strdup("../share/locale");

  return strdup(KCEMU_LOCALEDIR);
}

void
open_debug_output(char *filename)
{
  if (filename == NULL)
    return;

  ofstream *ofs = new ofstream(filename);
  if (!(*ofs))
    return;

  DBGI()->set_output_stream(ofs);
}

#ifdef HOST_OS_MINGW
void
close_output(void)
{
  FILE *f;

  /*
   *  Prevent the dos box from popping up by redirecting
   *  stdout and stdin to /dev/null. If opening /dev/null
   *  doesn't work we use the magically present NUL file.
   *  If both fail we fall back to creating a KCemu.log.
   */
  f = fopen("/dev/null", "wb");
  if (f == NULL)
    {
      f = fopen("NUL", "wb");
      if (f == NULL)
	{
	  f = fopen("KCemu.log", "wb");
	}
    }

  if (f == NULL)
    return;

  *stdout = *f;
  *stderr = *f;
}
#endif /* HOST_OS_MINGW */

#ifdef HOST_OS_MINGW
int WINAPI
WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nStil)
#define argc _argc
#define argv _argv
#else
int
main(int argc, char **argv)
#endif /* HOST_OS_MINGW */
{
  int c;
  int type;
  bool do_quit;
  int option_index;
  const char *getopt_args = "0123456789a:A:d:DEf:FhH:l:LM:o:p:Ps:t:u:vVW";
#ifdef HAVE_GETOPT_LONG
  static struct option long_options[] =
  {
    { "autostart",      1, 0, 'a' },
    { "address",        1, 0, 'A' },
    { "datadir",        1, 0, 'd' },
    { "display-debug",  0, 0, 'D' },
    { "effects",        0, 0, 'E' },
    { "floppy",         1, 0, 'f' },
    { "fullscreen",     0, 0, 'F' },
    { "help",           0, 0, 'h' },
    { "home",           1, 0, 'H' },
    { "localedir",      1, 0, 'l' },
    { "license",        0, 0, 'L' },
    { "modules",        1, 0, 'M' },
    { "debug-output",   1, 0, 'o' },
    { "profile",        1, 0, 'p' },
    { "select-profile", 0, 0, 'P' },
    { "scale",          1, 0, 's' },
    { "tape",           1, 0, 't' },
    { "vdip-root",      1, 0, 'u' },
    { "version",        0, 0, 'v' },
    { "viewlist",       0, 0, 'V' },
    { "warranty",       0, 0, 'W' },
    { 0,                0, 0, 0   }
  };
#endif /* HAVE_GETOPT_LONG */
  PortGroup *portg = NULL;
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
  kcemu_disk = 0;
  kcemu_modules = 0;
  kcemu_profile = 0;
  kcemu_debug_output = 0;
  kcemu_autostart_file = 0;
  kcemu_autostart_addr = 0;
  kcemu_vdip_root = 0;
  kcemu_ui_scale = -1;
  kcemu_ui_debug = -1;
  kcemu_ui_fullscreen = 0;
  kcemu_ui_display_effect = -1;
  kcemu_select_profile = 0;
  kcemu_datadir = get_kcemu_datadir();
  kcemu_homedir = get_kcemu_homedir();
  kcemu_localedir = get_kcemu_localedir();

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
  sys_socket_init();

  while (1)
    {
#ifdef HAVE_GETOPT_LONG
        c = getopt_long(argc, argv, getopt_args, long_options, &option_index);
#else
#ifdef HAVE_GETOPT
        c = getopt(argc, argv, getopt_args);
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
	case '0':
	  type = 0;
	  break;
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
	case '5':
	  type = 5;
	  break;
	case '6':
	  type = 6;
	  break;
	case '7':
	  type = 7;
	  break;
	case '8':
	  type = 8;
	  break;
	case '9':
	  type = 9;
	  break;
	case 'a':
	  kcemu_autostart_file = strdup(optarg);
	  break;
	case 'A':
	  kcemu_autostart_addr = strdup(optarg);
	  break;
	case 'd':
	  free(kcemu_datadir);
	  kcemu_datadir = strdup(optarg);
	  break;
	case 'D':
	  kcemu_ui_debug = 1;
	  break;
	case 'E':
	  kcemu_ui_display_effect = 1;
	  break;
	case 'f':
	  kcemu_disk = strdup(optarg);
	  break;
	case 'F':
	  kcemu_ui_fullscreen = 1;
	  break;
	case 'H':
	  if (kcemu_homedir)
	    free(kcemu_homedir);
	  kcemu_homedir = strdup(optarg);
	  break;
	case 'l':
	  free(kcemu_localedir);
	  kcemu_localedir = strdup(optarg);
	  break;
	case 'L':
	  license(argv[0]);
	  break;
	case 'M':
	  kcemu_modules = strdup(optarg);
	  break;
	case 'o':
	  kcemu_debug_output = strdup(optarg);
	  break;
        case 'p':
          kcemu_profile = strdup(optarg);
          break;
        case 'P':
          kcemu_select_profile = 1;
          break;
	case 's':
	  kcemu_ui_scale = strtoul(optarg, NULL, 0);
	  break;
	case 't':
	  kcemu_tape = strdup(optarg);
	  break;
        case 'u':
          kcemu_vdip_root = strdup(optarg);
          break;
	case 'v':
	  show_version(argv[0]);
	  break;
	case 'V':
	  show_variants(argv[0]);
	  break;
	case 'W':
	  warranty(argv[0]);
	  break;
        case ':':
	case '?':
	case 'h':
	default:
	  usage(argv[0]);
	  break;
	}
    }

  open_debug_output(kcemu_debug_output);

#ifdef HOST_OS_MINGW
  close_output();
#endif /* HOST_OS_MINGW */

  if (kcemu_homedir == NULL)
    kcemu_homedir = strdup(".");

  string home = kcemu_homedir;
  string sys_config_dir = string(kcemu_datadir) + "/config";
  string user_config_dir = home + "/config";
  string user_profiles_dir = home + "/profiles";

  Preferences::init(sys_config_dir.c_str(), user_config_dir.c_str(), user_profiles_dir.c_str());

  ui = new UI_Gtk();
  ui->init(&argc, &argv);
  
  if (kcemu_select_profile) {
      kcemu_profile = ui->select_profile();
  }

  Preferences::instance()->set_current_profile(kcemu_profile, type);

  /*
   *  check display scale
   */
  if (kcemu_ui_scale < 0)
    kcemu_ui_scale = Preferences::instance()->get_int_value("display_scale", 1);

  if (kcemu_ui_scale < 1)
    kcemu_ui_scale = 1;
  if (kcemu_ui_scale > 3)
    kcemu_ui_scale = 3;

  /*
   *  check display effect
   */
  if (kcemu_ui_display_effect < 0)
    kcemu_ui_display_effect = Preferences::instance()->get_int_value("display_effects", 0);

  if (kcemu_ui_display_effect < 0)
    kcemu_ui_display_effect = 0;
  if (kcemu_ui_display_effect > 1)
    kcemu_ui_display_effect = 1;

  /*
   *  check display debug
   */
  if (kcemu_ui_debug < 0)
    kcemu_ui_debug = Preferences::instance()->get_int_value("display_debug", 0);

  do
    {
      cmd   = new CMD("*");
      z80   = new Z80;
      disk  = new Disk;
      ports = new Ports;
      daisy = new DaisyChain;
      vdip  = new VDIP;
      kcnet = new KCNET;
      plotter = new Plotter;
      portg = ports->register_ports("-", 0, 0x100, new NullPort("NullPort"), 256);

      PIO0 *p0;
      PIO1_1 *p1;
      PIO2 *p2;
      PIO3 *p3;
      PIO4 *p4;
      PIO8_1 *p8;
      PIO9 *p9;
      Keyboard0 *k0;
      Keyboard1 *k1;
      Keyboard8 *k8;
      KeyboardKramerMC *k_kramer;
      KeyboardMuglerPC *k_mugler;
      KeyboardVCS80 *k_vcs80;
      KeyboardC80 *k_c80;

      timer = NULL;
      memory = NULL;
      fdc_z80 = NULL;
      gdc = NULL;
      vis = NULL;
      svg = NULL;
      poly880 = NULL;
      pio = pio2 = NULL;
      ctc = ctc2 = NULL;

      switch (Preferences::instance()->get_kc_type())
	{
	case KC_TYPE_87:
	  memory   = new Memory7;
	  /* fall through */
	case KC_TYPE_85_1:
	  if (memory == NULL)
	    memory   = new Memory1;

	  fileio_set_kctype(FILEIO_KC85_1);
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

	  p3       = new PIO3;
	  ctc      = new CTC3;
	  tape     = new Tape(364, 729, 1458, 1);
	  wav      = new WavPlayer(364, 729, 1458);
	  keyboard = new Keyboard3;
	  tape->set_tape_callback(p3);
	  pio = p3;
	  break;
	case KC_TYPE_85_4:
	case KC_TYPE_85_5:
	  memory   = new Memory4;
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
	case KC_TYPE_Z1013:
	  fileio_set_kctype(FILEIO_Z1013);
	  memory   = new Memory0;
	  ctc      = new CTC1;
	  p0       = new PIO0;
	  porti    = new Ports0;
	  k0       = new Keyboard0;
	  tape     = new Tape(500, 1000, 2000, 0);
	  wav      = new WavPlayer(500, 1000, 2000);
	  pio      = p0;
	  keyboard = k0;
	  tape->set_tape_callback(p0);
	  pio->register_callback_B_in(k0);
	  pio->register_callback_B_out(k0);
	  break;
	case KC_TYPE_A5105:
	  p9       = new PIO9;
	  ctc      = new CTC9;
	  memory   = new Memory9;
	  keyboard = new Keyboard9;
	  tape     = new Tape(500, 1000, 2000, 0);
	  wav      = new WavPlayer(781, 1562, 3125);
	  tape->set_tape_callback(p9);
	  pio      = p9;
	  break;
	case KC_TYPE_POLY880:
	  pio      = new PIO6_1;
	  pio2     = new PIO6_2;
	  ctc      = new CTC6;
	  porti    = new Ports6;
	  memory   = new Memory6;
	  tape     = new Tape(500, 1000, 2000, 0);
	  keyboard = new Keyboard6;
	  poly880  = new Poly880;
	  pio->register_callback_B_in((Keyboard6 *)keyboard);
	  pio->register_callback_A_out((Ports6 *)porti);
	  break;
	case KC_TYPE_KRAMERMC:
	  pio      = new PIOKramerMC;
	  memory   = new MemoryKramerMC;
	  k_kramer = new KeyboardKramerMC;
	  tape     = new Tape(500, 1000, 2000, 0); // FIXME: 
	  keyboard = k_kramer;
	  pio->register_callback_A_out(k_kramer);
	  pio->register_callback_B_in(k_kramer);
	  break;
	case KC_TYPE_MUGLERPC:
	  pio      = new PIOMuglerPCSystem;
	  pio2     = new PIOMuglerPCUser;
	  ctc      = new CTCMuglerPCSystem;
	  ctc2     = new CTCMuglerPCUser;
	  porti    = new PortsMuglerPC;
	  memory   = new MemoryMuglerPC;
	  k_mugler = new KeyboardMuglerPC;
	  tape     = new Tape(500, 1000, 2000, 0); // FIXME:
	  keyboard = k_mugler;
	  pio->register_callback_A_in(k_mugler);
	  break;
	case KC_TYPE_VCS80:
	  pio      = new PIOVCS80;
	  memory   = new MemoryVCS80;
	  tape     = new Tape(500, 1000, 2000, 0); // FIXME:
	  k_vcs80  = new KeyboardVCS80;
	  keyboard = k_vcs80;
	  pio->register_callback_A_in(k_vcs80);
	  break;
	case KC_TYPE_C80:
	  pio      = new PIOC80_1;
	  pio2     = new PIOC80_2;
	  memory   = new MemoryC80;
	  tape     = new Tape(500, 1000, 2000, 0); // FIXME:
	  k_c80  = new KeyboardC80;
	  display_c80 = new DisplayC80;
	  keyboard = k_c80;
	  pio->register_callback_A_in(k_c80);
	  pio->register_callback_A_out(display_c80);
	  pio->register_callback_B_out(display_c80);
	  break;
	default:
	  DBG(0, form("KCemu/internal_error",
		      "KCemu: setup with undefined system type\n"));
	  break;
	}

      module      = new Module;
      module_list = new ModuleList;
      
      ui->init2();

      switch (Preferences::instance()->get_kc_type())
	{
	case KC_TYPE_87:
	case KC_TYPE_85_1:
	  timer = new Timer1;

	  portg = ports->register_ports("CTC",  0x80, 4, ctc,  10);
	  portg = ports->register_ports("PIO1", 0x88, 4, pio,  10);
	  portg = ports->register_ports("PIO2", 0x90, 4, pio2, 10);

	  daisy->add_last(ctc);
	  daisy->add_last(pio);
	  daisy->add_last(pio2);
	  break;
	case KC_TYPE_85_2:
	case KC_TYPE_85_3:
	  timer = new Timer3;
	  sound = new Sound3;
	  if (Preferences::instance()->get_int_value("enable_sound", 0))
	    sound->init();

	  portg = ports->register_ports("Module", 0x80, 1, module, 10);
	  portg = ports->register_ports("PIO",    0x88, 4, pio,    10);
	  portg = ports->register_ports("CTC",    0x8c, 4, ctc,    10);

	  daisy->add_last(ctc);
	  daisy->add_last(pio);
	  break;
	case KC_TYPE_85_4:
	case KC_TYPE_85_5:
	  timer = new Timer3;
	  sound = new Sound3;
	  if (Preferences::instance()->get_int_value("enable_sound", 0))
	    sound->init();

	  portg = ports->register_ports("Module", 0x80, 1, module, 10);
	  portg = ports->register_ports("Port84", 0x84, 1, porti,  10);
	  portg = ports->register_ports("Port86", 0x86, 1, porti,  10);
	  portg = ports->register_ports("PIO",    0x88, 4, pio,    10);
	  portg = ports->register_ports("CTC",    0x8c, 4, ctc,    10);

	  daisy->add_last(ctc);
	  daisy->add_last(pio);
	  break;
	case KC_TYPE_LC80:
	  sound = new Sound8;
	  if (Preferences::instance()->get_int_value("enable_sound", 0))
	    sound->init();

	  portg = ports->register_ports("CTC",  0xec, 4, ctc,  10);
	  portg = ports->register_ports("PIO1", 0xf4, 4, pio,  10);
	  portg = ports->register_ports("PIO2", 0xf8, 4, pio2, 10);
          // lc80e
	  portg = ports->register_ports("PIO1", 0x74, 4, pio,  10);
	  portg = ports->register_ports("PIO2", 0x78, 4, pio2, 10);

          daisy->add_last(ctc);
	  daisy->add_last(pio);
	  daisy->add_last(pio2);
	  break;
	case KC_TYPE_Z1013:
	  portg = ports->register_ports("PIO",    0x00, 4, pio,   10);
	  portg = ports->register_ports("Port08", 0x08, 1, porti, 10);

	  daisy->add_last(pio);
	  break;
	case KC_TYPE_A5105:
	  gdc = new GDC;
	  vis = new VIS;
	  svg = new SVG;
	  fdc_fdc = new FDC9();

	  portg = ports->register_ports("FDC", 0x40, 12, fdc_fdc, 10);
	  portg = ports->register_ports("CTC", 0x80,  4, ctc,     10);
	  portg = ports->register_ports("PIO", 0x90,  4, pio,     10);
	  portg = ports->register_ports("GDC", 0x98,  2, gdc,     10);
	  portg = ports->register_ports("VIS", 0x9c,  3, vis,     10);
	  portg = ports->register_ports("SVG", 0xa0, 12, svg,     10);

	  daisy->add_last(ctc);
	  daisy->add_last(pio);
	  break;
	case KC_TYPE_POLY880:
	  portg = ports->register_ports("PIO1",  0x80,  4, pio,   10);
	  portg = ports->register_ports("PIO2",  0x84,  4, pio2,  10);
	  portg = ports->register_ports("CTC",   0x88,  4, ctc,   10);
	  portg = ports->register_ports("DIGIT", 0xc0, 32, porti, 10);
	  portg = ports->register_ports("DIGIT", 0xe0, 32, porti, 10);
	  break;
	case KC_TYPE_KRAMERMC:
	  portg = ports->register_ports("PIO",    0xfc, 4, pio,   10);
	  break;
	case KC_TYPE_MUGLERPC:
	  portg = ports->register_ports("CTC (system)", 0x80, 4, ctc,   10);
	  portg = ports->register_ports("PIO (system)", 0x84, 4, pio,   10);
	  portg = ports->register_ports("CTC (user)",   0x8c, 4, ctc2,  10);
	  portg = ports->register_ports("PIO (user)",   0x90, 4, pio2,  10);
	  portg = ports->register_ports("Port 94h",     0x94, 4, porti, 10);
	  break;
	case KC_TYPE_VCS80:
	  portg = ports->register_ports("PIO", 0x04, 4, pio, 10);
	  daisy->add_last(pio);
	  break;
	case KC_TYPE_C80:
	  portg = ports->register_ports("PIO (system)", 0xbc, 4, pio, 10);
	  portg = ports->register_ports("PIO (user)", 0x7c, 4, pio2, 10);
	  daisy->add_last(pio);
	  daisy->add_last(pio2);
	  break;
	default:
	  DBG(0, form("KCemu/internal_error",
		      "KCemu: setup with undefined system type\n"));
	  break;
	}

      if (Preferences::instance()->get_kc_type() & KC_TYPE_85_2_CLASS)
	if (Preferences::instance()->get_int_value("d004", 0))
	  {
	    fdc_z80 = new Z80_FDC();
	    fdc_ports = new Ports();
	    fdc_io = new FloppyIO();
	    fdc_shmem = new FloppySharedMem();
	    fdc_shmem->set_memory(&fdc_mem[0xfc00]);
	    fdc_fdc = new FDC4();
	    fdc_ctc = new CTC_FDC();
	    fdc_gide = new GIDE();

	    fdc_ports->register_ports("-", 0, 0x100, new NullPort("NullPort (FDC)"), 256);
	    fdc_ports->register_ports("GIDE", 0, 16, fdc_gide, 10);
	    fdc_ports->register_ports("FDC", 0xf0, 12, fdc_fdc, 10);
	    fdc_ports->register_ports("CTC", 0xfc, 4, fdc_ctc, 10);

	    portg = ports->register_ports("FloppyIO", 0xf4, 1, fdc_io, 10);
	    portg = ports->register_ports("FloppySHMEM", 0xf0, 4, fdc_shmem, 10);
	  }

      log = new LOG();
      
      module_list->init();

      if ((Preferences::instance()->get_kc_type() == KC_TYPE_85_1) ||
	  (Preferences::instance()->get_kc_type() == KC_TYPE_87) ||
	  (Preferences::instance()->get_kc_type() == KC_TYPE_LC80) ||
	  (Preferences::instance()->get_kc_type() == KC_TYPE_Z1013))
	tape->power(true);

      attach_tape();
      attach_disk();

      ui->show();
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
      delete memory;
      delete ports;
      delete ui;
      delete z80;
    }
  while (0); // (!do_quit);

  free(kcemu_datadir);
  free(kcemu_localedir);

  return 0;
}
