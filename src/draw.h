/**
 * \file draw.h 
 * \brief This module implements low level drawing functions.
 *
 * Contrary to the main menu which relies on directfb, 
 * Tomplayer engine implements its own drawing functions.
 *
 * When in video mode, the frame buffer is used directly 
 * When in audio mode, the display is sent to mplayer to be displayed as overlay
 *  
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
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

#ifndef __DRAW_H__
#define __DRAW_H__

#include <stdbool.h>
#include <IL/ilu.h>

void draw_RGB_buffer(unsigned char * buffer, int x, int y, int w, int h, bool transparency);
void draw_img(ILuint img);
void draw_text(const char * text, int x, int y, int w, int h, const struct font_color *color, int size);
void draw_cursor(ILuint cursor_id, ILuint frame_id, int x, int y );
void draw_refresh(void);

#endif
