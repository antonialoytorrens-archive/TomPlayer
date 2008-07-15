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

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

#include <stdbool.h>
#include "window.h"

/**
 * \struct gui_control_callback
 * \brief structure of association between a callback adress and a callback name
 */
struct gui_control_callback{
	char * callback_name; /*!< Name of the callback */
	bool (*callback)(struct gui_window *, char *, int, int ); /*!< Adress of the callback */
};

void * get_gui_callback_by_name( char * callback_name );

bool show_not_yet_implemented( struct gui_window *, char *, int, int );

bool scroll_up_listview( struct gui_window *, char *, int, int );
bool scroll_down_listview( struct gui_window *, char *, int, int );
bool listview_selected( struct gui_window * window, char * param, int x, int y );

bool show_param_window( struct gui_window *, char *, int, int );


bool play_video( struct gui_window *, char *, int, int );
bool resume_video( struct gui_window * window, char * param, int x, int y );
bool play_audio( struct gui_window *, char *, int, int );
bool playlist_audio( struct gui_window *, char *, int, int );
bool exit_tomplayer( struct gui_window *, char *, int, int );
bool select_skin( struct gui_window *, char *, int, int );



#endif /* __CALLBACKS_H__ */
