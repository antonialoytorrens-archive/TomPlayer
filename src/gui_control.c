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

#include <directfb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gui.h"
#include "gui_control.h"
#include "window_context.h"
#include "debug.h"
#include "file_list.h"
#include "version.h"
#include "engine.h"



/*!
 * \file gui_control.c
 * \brief GUI utilies
 * \author nullpointer
 */


/**
 * \def VIDEO_SKIN_FOLDER
 * \brief define the default video skin folder
 */
#define VIDEO_SKIN_FOLDER   "./skins/video"

/**
 * \def AUDIO_SKIN_FOLDER
 * \brief define the default audio skin folder
 */
#define AUDIO_SKIN_FOLDER   "./skins/audio"

/**
 * \var about_message
 * \brief string used in the about window
 */
char * about_message = 	"TomPlayer "VERSION"\n"\
						"www.tomplayer.net\n"\
						"Wolfgar & nullpointer\n"\
						"This software is under GPL";


/**
 * \fn char * get_static_text( char * param )
 * \brief return a string
 *
 * \param param the string id
 * \return the real string to use
 */
char * get_static_text( char * param ){
	if( !strcmp( param, "title" ) ) return context.title; /* Use by the message box */
	else if( !strcmp( param, "message" ) ) return context.message;/* Use by the message box */
	else if( !strcmp( param, "about_message" ) ) return about_message;/* Use by the about box */
	else return param;
}

int listview_get_item_height( void ){
	int font_height;

	context.listview_control->font->GetHeight( context.listview_control->font, &font_height );

	if( context.listview_icon )	return (ICON_H + font_height + 5);
	else return font_height;;
}

int listview_get_item_width( void ){
	int font_height;

	context.listview_control->font->GetHeight( context.listview_control->font, &font_height );

	if( context.listview_icon )	return ICON_W+5;
	else return context.listview_control->w;
}

int listview_get_nb_item_by_line( void ){
	if( context.listview_icon ) return context.listview_control->w/ICON_W;
	else return 1;
}

int listview_get_nb_line( void ){
	return context.listview_control->h/listview_get_item_height();
}

int listview_get_nb_item_printable( void ){
	return listview_get_nb_item_by_line() * listview_get_nb_line();
}

/**
 * \fn void update_gui_ctrl_listview_surface( void )
 * \brief update the listview control of the main window
 */
void update_gui_ctrl_listview_surface( void ){
	struct file_elt * f;
	int x = 10;
	int y = 0;
	int i,j;
	int string_width;
	int nb_elt;
	int max_elt_printable;
	struct gui_control * control;
	char fullpath[PATH_MAX];
	IDirectFBSurface * surface;


	control = context.listview_control;


	PRINTD( "update_control_listview_surface\n" );
	nb_elt = get_list_count( context.list_files );

	max_elt_printable = listview_get_nb_item_printable();

	PRINTDF( "Nombre d'element affichable : %d\n", max_elt_printable );

	control->bitmap_surface->SetPorterDuff(control->bitmap_surface, DSPD_NONE);
	control->bitmap_surface->SetBlittingFlags( control->bitmap_surface, DSBLIT_SRC_PREMULTIPLY );
	control->bitmap_surface->Clear( control->bitmap_surface,0,0,0,0);

	PRINTDF( "Nombre d'element %d\n", get_list_count(context.list_files ) );
	for( i = context.listview_first_item; i < nb_elt && i < (context.listview_first_item + max_elt_printable); i++ ){
		PRINTDF( "->%d\n", i);
		f = (struct file_elt *) get_object_from_list( context.list_files, i );
		if( f == NULL ) break;
		PRINTDF( "Affichage de <%s> (%d)\n", f->name, i );

		if( context.listview_icon ){
			if( f->type == TYPE_DIR ){
				surface = load_image_to_surface( DEFAULT_FOLDER_ICON );
			}
			else{
				get_thumbnail_name( context.current_path, f->name, fullpath );
				if( file_exist( fullpath ) ) surface = load_image_to_surface( fullpath );
				else surface = load_image_to_surface( DEFAULT_FILE_ICON );
			}

			control->bitmap_surface->Blit( control->bitmap_surface, surface, NULL, x, y );
			surface->Release( surface );

			/* Look for the best length */
			for( j = strlen( f->name ); j > 0; j--){
				control->font->GetStringWidth( control->font, f->name, j, &string_width );

				if( string_width < listview_get_item_width() ) break;
			}
			PRINTDF("Print string <%s> %d\n", f->name, j );
			control->bitmap_surface->DrawString(control->bitmap_surface, f->name, j, x, y+ICON_H,  DSTF_LEFT | DSTF_TOP );

			x+=listview_get_item_width();
			if( ( x + listview_get_item_width() )  > control->w ){
				x = 0;
				y +=listview_get_item_height();
			}
		}
		else{
			if( i == context.listview_selected_item ){
				control->bitmap_surface->SetColor( control->bitmap_surface,control->r,control->g,control->b,control->a );
				control->bitmap_surface->FillRectangle( control->bitmap_surface, 0, y, control->w, listview_get_item_height() );
				control->bitmap_surface->SetColor( control->bitmap_surface,255-control->r,255-control->g,255-control->b,control->a );
			}
			else control->bitmap_surface->SetColor( control->bitmap_surface,control->r,control->g,control->b,control->a );
			control->bitmap_surface->DrawString(control->bitmap_surface, f->name, -1, 0, y,  DSTF_LEFT | DSTF_TOP );
			y+=listview_get_item_height();
		}
	}
}

/**
 * \fn bool init_gui_ctrl_listview( struct gui_control * control )
 * \brief initialisation of the listview crontol of the main window
 *
 * \param control the listview control
 *
 * \return true on succes, false on failure
 */
bool init_gui_ctrl_listview( struct gui_control * control, bool icon_view ){
	DFBResult err;
	PRINTD( "init_ctrl_listview\n" );

	context.listview_control = control;
	context.listview_first_item = 0;
	context.listview_selected_item = -1;
	context.listview_icon = icon_view;
	control->bitmap_surface = create_surface( control->w, control->h );

	if( control->font == NULL ) control->font = default_font;
	DFBCHECK(control->bitmap_surface->SetFont( control->bitmap_surface, control->font ));

	if( !strcmp( control->param, "video" ) ){
		context.current_path = strdup( config.video_folder );
		context.filter_ext = config.filter_video_ext;
	}
	else if( !strcmp( control->param, "audio" ) ){
		context.current_path = strdup( config.audio_folder );
		context.filter_ext = config.filter_audio_ext;
	}
	else if( !strcmp( control->param, "audio_skin" ) ){
		context.current_path = strdup( AUDIO_SKIN_FOLDER );
		context.filter_ext = ".zip";
	}
	else if( !strcmp( control->param, "video_skin" ) ){
		context.current_path = strdup( VIDEO_SKIN_FOLDER );
		context.filter_ext = ".zip";
	}
	else{
		PRINTDF( "Invalid listview parameter <%s>\n", control->param );
		context.current_path = NULL;
		context.filter_ext = NULL;
		return false;
	}

	if( fill_list_files(context.current_path, context.filter_ext, &context.list_files) == true ){
		if( context.listview_icon && !strcmp( control->param, "video" ) ) generate_thumbnail();
		update_gui_ctrl_listview_surface();
	}
	else return false;

	return true;
}


/**
 * \fn bool is_gui_ctrl_selected( struct gui_control * control, int * ts_x, int * ts_y )
 * \brief indicate if the event is within control area
 *
 * \param control the control
 * \param ts_x x event
 * \param ts_y y event
 *
 * \return true if selected, false if not
 */
bool is_gui_ctrl_selected( struct gui_control * control, int * ts_x, int * ts_y ){
	int width = 0, height = 0;
	int x,y;

	PRINTDF( "is_control_selected <%d> <%d>\n", *ts_x, *ts_y );

	x = control->x;
	y = control->y;

	if( control->bitmap_surface != NULL ) control->bitmap_surface->GetSize( control->bitmap_surface, &width, &height);

	switch( control->type ){
			case GUI_TYPE_CTRL_ICON:
			case GUI_TYPE_CTRL_STATIC_TEXT:
				return false;
				break;
			case GUI_TYPE_CTRL_BUTTON:
			case GUI_TYPE_CTRL_LISTVIEW:
			case GUI_TYPE_CTRL_LISTVIEW_ICON:
				if( *ts_x >= x && *ts_x <= (x+width) && *ts_y >= y && *ts_y <= (y+height) ){
					*ts_x = *ts_x - x;
					*ts_y = *ts_y - y;
					return true;
				}
				break;
	}

	return false;
}
