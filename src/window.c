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
 * \file window.c
 * \author nullpointer
 * \brief window loading and releasing
 */

#include <dictionary.h>
#include <iniparser.h>
#include "gui.h"
#include "window.h"
#include "gui_control.h"
#include "callbacks.h"
#include "window_context.h"
#include "debug.h"

#define DEFAULT_FONT_HEIGHT 15

/**
 * \fn bool load_window( char * filename, struct gui_window * window )
 * \brief loading of a window, and initialization)
 *
 * \param filename window configuration file
 * \param window structure where to store configuration
 *
 * \return true on success, false on failure
 */
bool load_window( char * filename, struct gui_window * window ){
	bool return_code;
	struct list_object * list_controls;
	struct gui_control * control;

	PRINTDF( "load_window <%s>\n", filename );
	init_gui_window_context ();
	memset( window, 0, sizeof( struct gui_window ) );
	window->redirect_ts_event_to_controls = true;
	return_code = load_window_config( filename, window );

	if( return_code != true ){
		char msg[200];

		sprintf( msg, "Unable to load window config\n%s", filename );
		message_box ( "Crtitical Error", msg );
		exit( 1 );
	}

	if( window->font == NULL ) window->font = default_font;

	// Init each control
	list_controls = window->controls;
	while( list_controls != NULL ){
		control = (struct gui_control *) list_controls->object;
		switch( control->type ){
			case GUI_TYPE_CTRL_ICON:
			case GUI_TYPE_CTRL_STATIC_TEXT:
			case GUI_TYPE_CTRL_BUTTON:
				break;
			case GUI_TYPE_CTRL_LISTVIEW:
				init_gui_ctrl_listview( control, false );
				break;
			case GUI_TYPE_CTRL_LISTVIEW_ICON:
				init_gui_ctrl_listview( control, true );
				break;

		}
		list_controls = list_controls->next;
	}

	return return_code;
}


/**
 * \fn bool load_window_config( char * filename, struct gui_window * window )
 * \brief loading of a window configuration
 *
 * \param filename window configuration file
 * \param window structure where to store configuration
 *
 * \return true on success, false on failure
 */
bool load_window_config( char * filename, struct gui_window * window ){
	dictionary * ini ;
	char * s;
	int i;
	char * key_fmt = "control_%02d:%s";
	char key[200];
	bool return_code = false;
	struct gui_control * control;
	int num_control = 0;
	int font_height;

	PRINTDF( "load_window_config <%s>\n", filename );

	ini = iniparser_load(filename);
	if (ini == NULL) {
		PRINTDF( "Unable to load config file %s\n", filename);
		return false ;
	}

	s = iniparser_getstring(ini, "general:callback", NULL);
	if( s!=NULL) window->callback = get_gui_callback_by_name( s );

	s = iniparser_getstring(ini, "general:background", NULL);
	if( s != NULL ) window->background_surface = load_image_to_surface( s );

	window->r = iniparser_getint(ini, "general:r", 0);
	window->g = iniparser_getint(ini, "general:g", 0);
	window->b = iniparser_getint(ini, "general:b", 0);
	window->a = iniparser_getint(ini, "general:a", 0xFF);

	font_height = iniparser_getint(ini, "general:font_height", DEFAULT_FONT_HEIGHT);
	s = iniparser_getstring(ini, "general:font", NULL);

	if( s != NULL ) window->font = load_font (s, font_height );

	while( true ){
		sprintf( key, key_fmt, num_control, "type" );
		i = iniparser_getint(ini, key, -1);
		if( i < 0 ) break;

		if( i != GUI_TYPE_CTRL_ICON && i != GUI_TYPE_CTRL_STATIC_TEXT && i != GUI_TYPE_CTRL_BUTTON && i != GUI_TYPE_CTRL_LISTVIEW && i != GUI_TYPE_CTRL_LISTVIEW_ICON ){
			PRINTDF( "Control type unknown for control #%d\n", num_control );
			goto end;
		}

		control = ( struct gui_control * ) malloc( sizeof( struct gui_control ) );
		memset( control, 0, sizeof( struct gui_control ) );

		control->type = i;

		sprintf( key, key_fmt, num_control, "image" );
		s = iniparser_getstring(ini, key, NULL);
		if( s != NULL ) control->bitmap_surface = load_image_to_surface( s );

		sprintf( key, key_fmt, num_control, "x" );
		control->x = iniparser_getint(ini, key, -1);

		sprintf( key, key_fmt, num_control, "y" );
		control->y = iniparser_getint(ini, key, -1);

		sprintf( key, key_fmt, num_control, "w" );
		control->w = iniparser_getint(ini, key, -1);

		sprintf( key, key_fmt, num_control, "h" );
		control->h = iniparser_getint(ini, key, -1);

		sprintf( key, key_fmt, num_control, "param" );
		s = iniparser_getstring(ini, key, NULL);
		if( s != NULL ) control->param = strdup( s );

		sprintf( key, key_fmt, num_control, "font_height" );
		font_height = iniparser_getint(ini, key, DEFAULT_FONT_HEIGHT);

		sprintf( key, key_fmt, num_control, "font" );
		s = iniparser_getstring(ini, key, NULL);
		if( s != NULL ) control->font = load_font (s, font_height );


		sprintf( key, key_fmt, num_control, "callback" );
		s = iniparser_getstring(ini, key, NULL);
		if( s!=NULL) control->callback = get_gui_callback_by_name( s );

		sprintf( key, key_fmt, num_control, "r" );
		control->r = iniparser_getint(ini, key, 0);
		sprintf( key, key_fmt, num_control, "g" );
		control->g = iniparser_getint(ini, key, 0);
		sprintf( key, key_fmt, num_control, "b" );
		control->b = iniparser_getint(ini, key, 0);
		sprintf( key, key_fmt, num_control, "a" );
		control->a = iniparser_getint(ini, key, 0xFF);

		add_to_list( &window->controls, control );
		num_control++;
	}

	return_code = true;

end:
	iniparser_freedict(ini);
	return return_code;
}

/**
 * \fn void unload_window( struct gui_window * window )
 * \brief release a window
 *
 * \param window window structure
 */
void unload_window( struct gui_window * window ){
	struct list_object * list_controls;
	struct gui_control * control;

	PRINTD("unload_window\n");

	if( window->background_surface != NULL ) window->background_surface->Release( window->background_surface );
	if( window->font != NULL && window->font != default_font) window->font->Release( window->font );

	list_controls = window->controls;
	while( list_controls != NULL ){
		control = (struct gui_control *) list_controls->object;
		if( control->bitmap_surface != NULL )  control->bitmap_surface->Release( control->bitmap_surface );
		if( control->font != NULL && control->font != default_font )  control->font->Release( control->font );
		list_controls = list_controls->next;
	}

	release_list( window->controls, NULL );
	window->controls = NULL;
	release_gui_window_context();
}




