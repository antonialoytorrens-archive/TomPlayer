/***************************************************************************
 *  19/06/2008
 *  Copyright  2008  nullpointer
 *  Email nullpointer[at]lavabit[dot]com
 ****************************************************************************/
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

/*!
 * \file gui.h
 * \author nullpointer
 */

#ifndef __GUI_H__
#define __GUI_H__

#include <directfb.h>
#include "window.h"

/**
 * \def FONT
 * \brief The default font used by the application
 */
#define FONT "./res/font/decker.ttf"


/**
 * \struct gui_config
 * \todo move to config.h/config.c
 */
struct gui_config{
	char * default_font;
	int font_height;
	char * first_window;
	char * messagebox_window;
};


extern IDirectFBFont *default_font;

IDirectFBSurface * load_image_to_surface( char * filename );
IDirectFBSurface * create_surface( int width, int height );
IDirectFBFont * load_font( char * filename, int height );
void draw_window( struct gui_window * window);
void show_message_box( char * title, char * message );
void show_information_message( char * msg );

#endif /* __GUI_H__ */
