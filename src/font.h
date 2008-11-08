/**
 * \file font.h
 * \author nullpointer & Wolfgar 
 * \brief This module enables to generate RGBA bitmap buffer with some text in it
 * 
 * $URL:$
 * $Rev:$
 * $Author:$
 * $Date:$
 *
 */

/*
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


#ifndef __FONT_H__
#define __FONT_H__

struct font_color{
  unsigned char r;
  unsigned char g;
  unsigned char b;
};


bool font_draw(const struct font_color * ,  const char *, unsigned char ** , int * , int *);
bool font_init(int );
void font_release(void);

#endif
