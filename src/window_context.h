/***************************************************************************
 *  19/06/2008
 *  Copyright  2008  nullpointer
 *  Email
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
 * \file window_context.h
 * \author nullpointer
 * \brief GUI context initialization and release
 */

#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include "list.h"
#include "gui_control.h"

/**
 * \struct gui_window_context
 * \brief window context (use as a shared memory by GUI callback
 */
struct gui_window_context{
	struct list_object * list_files;	/*!<list of object used by the listview */
	char * current_path;				/*!<current path of the listview */
	char * filter_ext;					/*!<filter extension use by the listview */
	struct gui_control * listview_control; /*!<reference to the listview control in the window */

	char * title;						/*!<title of the message box */
	char * message;						/*!<message of the message box */

	int listview_first_item;			/*!<index of the first item shown in the listview */
	int listview_selected_item;			/*!<index of the selected item in the listview */
	bool listview_icon;					/*!<Indicate if the listview uses icons or is text only */
};

extern struct gui_window_context context;

void init_gui_window_context( void );

#endif /* __CONTEXT_H__ */
