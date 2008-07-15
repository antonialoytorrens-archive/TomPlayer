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
 * \file window_context.c
 * \author nullpointer
 * \brief GUI context initialization and release
 */

#include <string.h>
#include <stdlib.h>
#include "window_context.h"
#include "file_list.h"
#include "debug.h"

/**
 * \var context
 * \brief GUI context
 */
struct gui_window_context context;

/**
 * \fn void init_gui_window_context( void )
 * \brief Initialization of the context
 */
void init_gui_window_context( void ){
	PRINTD( "init_context\n" );
	memset( &context, 0, sizeof( struct gui_window_context ) );
}

/**
 * \fn void release_gui_window_context( void )
 * \brief Release the context
 */
void release_gui_window_context( void ){
	PRINTD( "release_context\n" );
	if( context.current_path != NULL ){
		free( context.current_path );
		context.current_path = NULL;
	}/*
	if( context.selected_file != NULL ){
		free( context.selected_file );
		context.selected_file = NULL;
	}*/

	context.listview_control = NULL;
	release_list (context.list_files, release_file_elt );
	context.list_files = NULL;

	context.filter_ext = NULL;

	context.listview_first_item = 0;
	context.listview_selected_item = -1;

	context.title = NULL;
	context.message = NULL;
}
