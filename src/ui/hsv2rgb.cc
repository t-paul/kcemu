/*
 *  KCemu -- The emulator for the KC85 homecomputer series and much more.
 *
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2000  Robert Gentleman, Ross Ihaka and the
 *			      R Development Core Team
 *  Copyright (C) 2001        Torsten Paul
 *
 *  For the original licence statement of the graphics.c file
 *  see below...
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
 *
 *  2001-03-28:
 *  - Code for colorspace conversion from HSV to RGB stolen from
 *    the file R-1.1.0/src/main/graphics.c of the R statistical
 *    language.
 *  - modified to use different input/output ranges
 *
 */

/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 1997--2000  Robert Gentleman, Ross Ihaka and the
 *			      R Development Core Team
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *  This is an extensive reworking by Paul Murrell of an original
 *  quick hack by Ross Ihaka designed to give a superset of the
 *  functionality in the AT&T Bell Laboratories GRZ library.
 *
 */

#include <math.h>
#include <stdio.h>

#include "kc/system.h"

#include "ui/hsv2rgb.h"

#include "libdbg/dbg.h"

/* hsv2rgb -- HSV to RGB conversion  */
/* Based on HSV_TO_RGB from Foley and Van Dam First Ed. Page 616 */
/* See Alvy Ray Smith, Color Gamut Transform Pairs, SIGGRAPH '78 */

void
hsv2rgb(double h, double s, double v, int *r, int *g, int *b)
{
    int i;
    double f, p, q, t;

    h = h / 360.0; /* original function had h in [0,1] */
    t = 6 * modf(h, &f);/* h = t/6 + f = fract. + int. */
    i = (int)floor(t+1e-5);/* 0..5 */
    f = modf(t, &p);
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - (s * (1 - f)));
    switch (i) {
    case 0:
	*r = (int)(v * 255);
	*g = (int)(t * 255);
	*b = (int)(p * 255);
	break;
    case 1:
	*r = (int)(q * 255);
	*g = (int)(v * 255);
	*b = (int)(p * 255);
	break;
    case 2:
	*r = (int)(p * 255);
	*g = (int)(v * 255);
	*b = (int)(t * 255);
	break;
    case 3:
	*r = (int)(p * 255);
	*g = (int)(q * 255);
	*b = (int)(v * 255);
	break;
    case 4:
	*r = (int)(t * 255);
	*g = (int)(p * 255);
	*b = (int)(v * 255);
	break;
    case 5:
	*r = (int)(v * 255);
	*g = (int)(p * 255);
	*b = (int)(q * 255);
	break;
    default:
	*r = 0;
	*g = 0;
	*b = 0;
        DBG(1, form("KCemu/warning",
	            "bad hsv to rgb color conversion"));
	break;
    }
}
