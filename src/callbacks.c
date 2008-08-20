/***************************************************************************
 *  19/06/2008
 *  Copyright  2008  nullpointer
 *  Email nullpointer[at]lavabit[dot]com
 *
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
 * \file callbacks.c
 * \brief definition of all GUI callbacks
 * \author nullpointer
 */

#include <linux/limits.h>
#include <dictionary.h>
#include <iniparser.h>
#include "callbacks.h"
#include "gui.h"
#include "window_context.h"
#include "debug.h"
#include "list.h"
#include "file_list.h"
#include "engine.h"
#include "resume.h"
#include "playlist.h"
#include "zip_skin.h"
#include "config.h"

/* Correct iniparser bug in naming function iniparser_setstring/iniparser_set */
extern int iniparser_set(dictionary * ini, char * entry, char * val);
#define iniparser_setstring iniparser_set

/**
 * \def DEFAULT_PLAYLIST
 * \brief fullpath of the generated playlist file
 */
#define DEFAULT_PLAYLIST        "/tmp/tomplayer.m3u"

/**
 * \var gui_control_callbacks
 * \brief Array of association callback adress <-> callback name
 */
static struct gui_control_callback gui_control_callbacks[]={
	{"show_not_yet_implemented", show_not_yet_implemented},
	{"show_param_window", show_param_window},
	{"scroll_up_listview", scroll_up_listview},
	{"scroll_down_listview", scroll_down_listview},
	{"listview_selected", listview_selected},
	{"play_video", play_video},
	{"resume_video", resume_video },
	{"play_audio", play_audio},
	{"playlist_audio", playlist_audio},
	{"select_skin",select_skin},
	{"exit", exit_tomplayer}
};


/**
 * \fn void * get_gui_callback_by_name( char * callback_name )
 * \brief Retrieve a callback method by name
 *
 * \param callback_name name of the callback
 * \return address of the callback function, NULL if not found
 */
void * get_gui_callback_by_name( char * callback_name ){
	int i;

	PRINTDF( "get_callback <%s>\n", callback_name );
	for( i = 0; i < sizeof( gui_control_callbacks )/sizeof( struct gui_control_callback); i++ ){
		if( !strcmp( callback_name, gui_control_callbacks[i].callback_name ) ) return gui_control_callbacks[i].callback;
	}

	return NULL;
}

/**
 * \fn bool show_param_window( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Load and show a new windows
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool show_param_window( struct gui_window * window, char * param, int x, int y ){
	PRINTD( "show_main_window\n" );

	if( window != NULL ) unload_window( main_window, true );
	main_window = load_window ( param, true );
	if( main_window == NULL ){
		PRINTDF( "Unable to show window <%s>\n", param );
		return false;
	}
	else{
		draw_window( main_window );
		return true;
	}
}

/**
 * \fn bool show_not_yet_implemented( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Show the not yet implemented window
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool show_not_yet_implemented( struct gui_window * window, char * param, int x, int y ){
	show_message_box ( "Error", "Not yet implemented\n Wait next version :)" );
	return true;
}



/**
 * \fn int listview_get_selected_item( int x, int y)
 * \brief return the index of the selected item in the listview
 *
 * \param x x touchscreen event
 * \param y y touchscreen event
 *
 * \return the index of the selected item
 */
static int listview_get_selected_item( int x, int y){

	return (context.listview_control->w / listview_get_item_width()) * (y/listview_get_item_height()) + x / listview_get_item_width() + context.listview_first_item;
}

/**
 * \fn bool scroll_up_listview( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Scroll up the listview of the current window
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool scroll_up_listview( struct gui_window * window, char * param, int x, int y ){
	int nb_item_by_line;
	int previous_first_item = context.listview_first_item;

	/* Check if the listview exist */
	if( context.listview_control == NULL ){
		show_message_box ( "Error", "No listview in this window" );
		return false;
	}

	if( context.listview_icon ){
		nb_item_by_line = listview_get_nb_item_by_line();
		if( context.listview_first_item >= nb_item_by_line ) context.listview_first_item-=nb_item_by_line;
	}
	else if( context.listview_first_item > 0 ) context.listview_first_item--;

	/* Update screen if the listview changed */
	if( previous_first_item != context.listview_first_item ){
		update_gui_ctrl_listview_surface();
		draw_window( window );
	}

	return true;
}

/**
 * \fn bool scroll_down_listview( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Scroll down the listview of the current window
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool scroll_down_listview( struct gui_window * window, char * param, int x, int y ){
	int num_elt_printable;
	int nb_item_by_line;
	int previous_first_item = context.listview_first_item;

	PRINTD("scroll_down_listview\n");

	/* Check if the listview exist */
	if( context.listview_control == NULL ){
		show_message_box ( "Error", "No listview in this window" );
		return false;
	}

	if( context.listview_icon ){
		nb_item_by_line = listview_get_nb_item_by_line();

		if( (context.listview_first_item + nb_item_by_line) < get_list_count( context.list_files ) ) context.listview_first_item+=nb_item_by_line;
	}
	else{
		num_elt_printable = listview_get_nb_line();
		if( context.listview_first_item < (get_list_count( context.list_files ) - num_elt_printable)  ) context.listview_first_item++;
	}

	/* Update screen if the listview changed */
	if( previous_first_item != context.listview_first_item ){
		update_gui_ctrl_listview_surface();
		draw_window( window );
	}


	return true;
}


/**
 * \fn bool listview_selected( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Select a new item or change current directory, then update the listview
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool listview_selected( struct gui_window * window, char * param, int x, int y ){
	int i;
	struct file_elt * file_elt;
	char current_path[200];

	/* Get the selected item index */
	context.listview_selected_item = listview_get_selected_item( x, y);

	file_elt = (struct file_elt *) get_object_from_list( context.list_files, context.listview_selected_item );
	PRINTDF( "Selected element <%s> <%d>\n", file_elt->name, file_elt->type );

	/* If selected item is a directory, then the current path changed */
	if( file_elt->type == TYPE_DIR ){
		/* Upper folder */
		if( !strcmp( file_elt->name, ".." ) ){
			i = strlen( context.current_path );
			while( context.current_path[i] != '/' && i > 0 ) i--;
			if( i > 0 )	context.current_path[i] = 0;
		}
		else{
			sprintf( current_path , "%s/%s", context.current_path, file_elt->name );
			free( context.current_path );
			context.current_path = strdup( current_path );
		}
		PRINTDF( "Current path changed : <%s>\n", context.current_path);

		/* Release the current file list */
		release_list (context.list_files, release_file_elt );

		/* Init listview control */
		context.list_files = NULL;
		context.listview_first_item = 0;
		context.listview_selected_item = -1;

		/* Construct the list of elements in the new directory */
		fill_list_files(context.current_path, context.filter_ext,  &context.list_files);

		/* If we are looking to video file, then we try to generate thumbnail */
		if( !strcmp( context.listview_control->param, "video" ) && context.listview_icon ) generate_thumbnail();

	}

	/* Update the window */
	update_gui_ctrl_listview_surface();
	draw_window( window );

	return true;
}

/**
 * \fn bool play_video( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Play the selected video file
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool play_video( struct gui_window * window, char * param, int x, int y ){
	if( context.listview_selected_item < 0 )
		show_message_box ( "Error", "No file selected !" );
	else{
		struct file_elt * elt;
		elt = (struct file_elt *) get_object_from_list( context.list_files, context.listview_selected_item );
		display_current_file( elt->name, &config.video_config, config.bitmap_loading );
		launch_mplayer( context.current_path, elt->name, 0 );
	}
	return true;
}

/**
 * \fn bool resume_video( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Resume the video file
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool resume_video( struct gui_window * window, char * param, int x, int y ){
    int pos = 0;
    char filename[PATH_MAX];

    if (resume_get_file_infos(filename, PATH_MAX, &pos) != 0){
    	show_message_box ("TomPlayer", "Unable to retrieve resume informations");
    }
    else{
    	display_current_file( filename, &config.video_config, config.bitmap_loading );
        launch_mplayer( "", filename, pos );
    }
    return true;
}

/**
 * \fn bool play_audio( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Play the selected audio file
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool play_audio( struct gui_window * window, char * param, int x, int y ){
	if( context.listview_selected_item < 0 )
		show_message_box ( "Error", "No file selected !" );
	else{
		struct file_elt * elt;
		elt = (struct file_elt *) get_object_from_list( context.list_files, context.listview_selected_item );
		display_current_file( elt->name, &config.audio_config, config.audio_config.bitmap );
		launch_mplayer( context.current_path, elt->name, 0 );
	}
	return true;
}

/**
 * \fn bool play_video( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Create and play an audio playlist
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool playlist_audio( struct gui_window * window, char * param, int x, int y ){
	if( generate_random_playlist( context.current_path, DEFAULT_PLAYLIST ) == true ){
		display_current_file( DEFAULT_PLAYLIST, &config.audio_config, config.audio_config.bitmap );
		launch_mplayer( "", DEFAULT_PLAYLIST, 0 );
	}
	else show_message_box ( "Error", "Unable to create playlist" );

	return true;
}

/**
 * \fn bool exit_tomplayer( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. Display the "exit" bitmap and exit the application
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool exit_tomplayer( struct gui_window * window, char * param, int x, int y ){
	PRINTD( "Exiting...\n" );
	display_image_to_fb( config.bitmap_exiting );
	return false;
}

/**
 * \fn bool select_skin( struct gui_window * window, char * param, int x, int y )
 * \brief callback function. todo
 *
 * \param window actual window
 * \param param parameter associated to the control
 * \param x mouse x
 * \param y mouse y
 *
 * \return true on succes, false on failure
 */
bool select_skin( struct gui_window * window, char * param, int x, int y ){
	struct skin_config * conf = NULL;
	char * original_skin_filename = NULL;
	char * config_skin_filename = NULL;
	char * config_skin_filename_key = NULL;
	char cmd[200];
	struct file_elt * elt;
	char zip_file[PATH_MAX+1];
	dictionary * ini ;
	FILE * fp;

	PRINTDF( "select_skin %s\n", param);
	if( context.listview_selected_item < 0 )
		show_message_box ( "Error", "No skin selected !" );
	else{
		if( !strcmp( param, "video_skin" ) ){
			conf = &config.video_config;
			original_skin_filename = config.video_skin_filename;
			config_skin_filename = config.video_skin_filename;
			config_skin_filename_key = SECTION_VIDEO_SKIN":"KEY_SKIN_FILENAME;
		}
		else if( !strcmp( param, "audio_skin" ) ){
			conf = &config.audio_config;
			original_skin_filename = config.audio_skin_filename;
			config_skin_filename = config.audio_skin_filename;
			config_skin_filename_key = SECTION_AUDIO_SKIN":"KEY_SKIN_FILENAME;
		}

		if( conf != NULL ){
			elt = (struct file_elt *) get_object_from_list( context.list_files, context.listview_selected_item );
			unload_skin( conf );
			sprintf( zip_file, "%s/%s", context.current_path, elt->name );
			if( load_skin_from_zip( zip_file, conf ) == false ){
				show_message_box( "Error", "Unable to load this skin");
				load_skin_from_zip( original_skin_filename, conf );
			}
			else{
				strcpy( config_skin_filename, zip_file );

				ini = iniparser_load(CONFIG_FILE);
				if( ini == NULL ){
					PRINTD( "Unable to save main configuration\n" );
					goto error;
				}

				fp = fopen( CONFIG_FILE, "w+" );
				if( fp == NULL ){
					PRINTD( "Unable to open main config file\n" );
					goto error;
				}

				iniparser_setstring( ini, config_skin_filename_key, NULL );
				iniparser_setstring( ini, config_skin_filename_key, zip_file );
				iniparser_dump_ini( ini, fp );

				fclose( fp );


				sprintf( cmd, "cp -f %s ./conf/tomplayer.ini", CONFIG_FILE );
				system( cmd );
				system( "unix2dos ./conf/tomplayer.ini" );

				show_message_box( "Info", "Skin successfully loaded\n");
error:
				iniparser_freedict(ini);
			}
		}
	}

	return true;
}
