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
 * \file gui_control.h
 * \brief GUI utilies
 * \author nullpointer
 */

#ifndef __CONTROLS_H__
#define __CONTROLS_H__
#include <stdbool.h>
#include <directfb.h>
#include "window.h"


#define ICON_W	80
#define ICON_H	80
#define DEFAULT_FILE_ICON "./res/icon/file.png"
#define DEFAULT_AUDIO_ICON "./res/icon/audio.png"
#define DEFAULT_VIDEO_ICON "./res/icon/video.png"
#define DEFAULT_FOLDER_ICON "./res/icon/folder.png"

/**
 * \enum E_GUI_TYPE_CTRL
 * \brief define the type of control
 */
enum E_GUI_TYPE_CTRL{
	GUI_TYPE_CTRL_ICON = 0,		/*!< a simple icon */
	GUI_TYPE_CTRL_STATIC_TEXT,	/*!< a static text */
	GUI_TYPE_CTRL_BUTTON,		/*!< a button */
	GUI_TYPE_CTRL_LISTVIEW,		/*!< a text listview */
	GUI_TYPE_CTRL_LISTVIEW_ICON	/*!< an icon listview */
};

/**
 * \struct gui_control
 * \brief define a GUI control
 */
struct gui_control{
	enum E_GUI_TYPE_CTRL type;		/*!<type of control */
	IDirectFBSurface * bitmap_surface;	/*!<DirectFB bitmap */
	IDirectFBFont *font;				/*!<Reference to the used font */
	int r,g,b,a;						/*!< color of text */
	int x, y;							/*!<position of the control within the window */
	int w,h;							/*!< size of the control */
	char * param;						/*!<parameter of the control */
	bool (*callback)(struct gui_window *, char * param, int , int );	/*!< callback adress of the control */
};


char * get_static_text( char * param );
int listview_get_nb_item_printable( void );
int listview_get_nb_line( void );
int listview_get_nb_item_by_line( void );

bool init_gui_ctrl_listview( struct gui_control *, bool );
bool is_gui_ctrl_selected( struct gui_control * control, int * ts_x, int * ts_y );
void update_gui_ctrl_listview_surface( void );
#endif /* __CONTROLS_H__ */
