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

/**
 * \file window.h
 * \author nullpointer
 * \brief GUI Window declarations
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdbool.h>
#include <directfb.h>
#include "list.h"


/**
 * \struct gui_window
  */
struct gui_window{
	IDirectFBSurface * background_surface; /*!<DirectFB background bitmap */
	struct list_object * controls; /*!<list of controls */
	IDirectFBFont *font;	/*!<DirectFB font used by the window */
	int r,g,b,a; 		/*!<Text color */
	bool (*callback)(struct gui_window *, char * param, int , int ); /*!<periodic callback of the window */
};

extern struct gui_window * main_window;

struct gui_window *  load_window( char * filename, bool );
bool load_window_config( char * filename, struct gui_window * window );
void unload_window( struct gui_window * window, bool );

#endif /* __WINDOW_H__ */
