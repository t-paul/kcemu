/*
 *  KCemu -- the KC 85/3 and KC 85/4 Emulator
 *  Copyright (C) 1997-2001 Torsten Paul
 *
 *  $Id: hsv2rgb.h,v 1.2 2001/04/01 21:07:56 tp Exp $
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

#ifndef __ui_hsv2rgb_h
#define __ui_hsv2rgb_h

/*
 *  input:
 *    h    in [0,360]
 *    s, v in [0,1] 
 *  output:
 *    r, g, b in [0, 255]
 */
void hsv2rgb(double h, double s, double v, int *r, int *g, int *b);

#endif /* __ui_hsv2rgb_h */
