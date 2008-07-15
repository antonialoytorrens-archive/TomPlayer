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
 * \file gui.c
 * \author nullpointer
 */


#include <directfb.h>
#include <direct/util.h>
#include <dictionary.h>
#include <iniparser.h>
#include "gui.h"
#include "window.h"
#include "gui_control.h"
#include "callbacks.h"
#include "debug.h"
#include "window_context.h"
#include "engine.h"
#include "pwm.h"
#include "power.h"

/**
 * \var gui_config
 * \brief the GUI configuration structure
 */
static struct gui_config gui_config;


/**
 * \var is_wide_screen
 * \brief Indicate if the device has a wide screen or not
 */
static bool is_wide_screen = false;

static IDirectFB				*dfb;
static IDirectFBSurface			*primary;
static IDirectFBEventBuffer		*keybuffer;

/**
 * \var default_font
 * \brief default font of the application
 */
IDirectFBFont 					*default_font;

static int mouse_x=0, mouse_y=0;
static int screen_width=0, screen_height = 0;


static struct gui_window messagebox_window;
struct gui_window main_window;


/**
 * \fn IDirectFBSurface * load_image_to_surface( char * filename )
 * \brief load an image to a DirectFB surface
 *
 * \param filename of the bitmap
 *
 * \return DirectFB surface
 */
IDirectFBSurface * load_image_to_surface( char * filename ){
	DFBResult err;
	IDirectFBImageProvider *provider;
	DFBSurfaceDescription dsc;
	IDirectFBSurface * surface;

	PRINTDF( "load_image_to_surface <%s>\n", filename );

	DFBCHECK( dfb->CreateImageProvider( dfb, filename, &provider ) );
	DFBCHECK( provider->GetSurfaceDescription (provider, &dsc) );
	DFBCHECK( dfb->CreateSurface( dfb, &dsc, &surface ) );
	DFBCHECK( provider->RenderTo( provider, surface, NULL ) );

	provider->Release( provider );

	return surface;
}

/**
 * \fn IDirectFBFont * load_font( char * filename, int height )
 * \brief load a font
 *
 * \param filename of the font
 * \param height size of the font
 *
 * \return DirectFB font
 */
IDirectFBFont * load_font( char * filename, int height ){
	DFBFontDescription desc;
	IDirectFBFont * font;
	DFBResult err;

	PRINTDF( "Loading font <%s> <%d>\n", filename, height );

	desc.flags = DFDESC_HEIGHT;
	desc.height = height;

	DFBCHECK(dfb->CreateFont( dfb, filename, &desc, &font ));

	return font;
}

/**
 * \fn IDirectFBSurface * create_surface( int width, int height )
 * \brief Create an empty DirectFB surface
 *
 * \param width witdh of the surface
 * \param height height of the surface
 *
 * \return DirectFB surface
 */
IDirectFBSurface * create_surface( int width, int height ){
	IDirectFBSurface * surface;
	DFBSurfaceDescription dsc;
	DFBResult err;

	PRINTDF( "create_surface %d %d\n", width, height );

	dsc.flags =  DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
	dsc.width = width;
	dsc.height = height;
	dsc.pixelformat = DSPF_ARGB;

	DFBCHECK( dfb->CreateSurface( dfb, &dsc, &surface ) );

	return surface;
}

/**
 * \fn void draw_window( struct gui_window * window)
 * \brief draw the window given in argument to the screen
 *
 * \param window the window
 */
void draw_window( struct gui_window * window)
{
	struct list_object * list_controls;
	struct gui_control * control;
	char * p;
	int y;
	int font_height;

	PRINTD( "draw_window\n" );
	primary->SetBlittingFlags( primary,DSBLIT_BLEND_ALPHACHANNEL);
	primary->SetColor( primary, window->r, window->g, window->b, window->a );

	if( window->font != NULL ){
		primary->SetFont( primary, window->font );
		window->font->GetHeight( window->font, &font_height );
	}
	else{
		primary->SetFont( primary, default_font );
		default_font->GetHeight( default_font, &font_height );
	}

	primary->Clear( primary, 0, 0, 0, 0xFF );

	if( window->background_surface != NULL ){
		PRINTD( "Drawing background surface\n" );
		primary->Blit( primary, window->background_surface, NULL, 0, 0 );
	}

	list_controls = window->controls;
	while( list_controls != NULL ){
		control = (struct gui_control *) list_controls->object;
		PRINTDF( "Affichage du control de type %d\n", control->type );
		switch( control->type ){
			case GUI_TYPE_CTRL_ICON:
				primary->Blit( primary, control->bitmap_surface, NULL, control->x, control->y );
				break;
			case GUI_TYPE_CTRL_STATIC_TEXT:
				p = strdup( get_static_text( control->param ) );
				y = control->y;
				p = strtok( p, "\n" );
				while( p!= NULL ){
					primary->DrawString( primary, p, -1,control->x, y,  DSTF_CENTER | DSTF_TOP );
					p=strtok( NULL, "\n" );
					y += font_height;
				}
				break;
			case GUI_TYPE_CTRL_BUTTON:
				primary->Blit( primary, control->bitmap_surface, NULL, control->x, control->y );
				break;
			case GUI_TYPE_CTRL_LISTVIEW:
			case GUI_TYPE_CTRL_LISTVIEW_ICON:
				primary->Blit( primary, control->bitmap_surface, NULL, control->x, control->y );
				break;
		}

		list_controls = list_controls->next;
	}
}



/**
 * \fn static void init_resources( int argc, char *argv[] )
 * \brief set up DirectFB and load resources
 *
 * \param argc argument count
 * \param argv argument values
 */
static void init_resources( int argc, char *argv[] )
{
	DFBResult err;
	DFBSurfaceDescription dsc;

	PRINTD( "init_resources\n" );


	DFBCHECK(DirectFBInit( &argc, &argv ));

	DFBCHECK(DirectFBCreate( &dfb ));

	DFBCHECK(dfb->CreateInputEventBuffer( dfb, DICAPS_ALL,DFB_FALSE, &keybuffer ));

	err = dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN );
	if (err) DirectFBError( "Failed to get exclusive access", err );

	dsc.flags = DSDESC_CAPS;
	dsc.caps = DSCAPS_PRIMARY /*| DSCAPS_DOUBLE*/;

	err = dfb->CreateSurface( dfb, &dsc, &primary );

	DFBCHECK(primary->GetSize( primary, &screen_width, &screen_height ));

	if( screen_width == 480 ) is_wide_screen = true;
	if( screen_width == 240 ){
		s32 matrix[9];

		matrix[0] = 0;
		matrix[1] = 1;
		matrix[2] = 0;
		matrix[3] = 1;
		matrix[4] = 0;
		matrix[5] = 0;
		matrix[6] = 0;
		matrix[7] = 0;
		matrix[8] = 0;

		primary->SetRenderOptions( primary, DSRO_MATRIX );
		primary->SetMatrix( primary, matrix );
	}
	default_font = load_font( FONT, 20 );
}


/**
 * \fn static void release_resources( void )
 * \brief release DirectFB
 */
static void release_resources( void )
{
	PRINTD( "deinit_resources\n");
	default_font->Release( default_font );
	primary->Release( primary );
	keybuffer->Release( keybuffer );
	dfb->Release( dfb );
}

/**
 * \fn void message_box( char * title, char * message )
 * \brief show a message box on the screen
 *
 * \param title title of the message box
 * \param message message to display
 */
void message_box( char * title, char * message ){
	DFBInputEvent evt;

	context.title = title;
	context.message = message;
	draw_window( &messagebox_window );

	while ( 1 ) {
		keybuffer->WaitForEvent( keybuffer );
		if( keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK )
			if (evt.type == DIET_BUTTONPRESS) break;
	}

	draw_window( &main_window );
}

/**
 * \fn static bool dispatch_ts_event( struct gui_window * window, DFBInputEvent *evt )
 * \brief dispatch a touch screen event to the control selected or to the skin
 *
 * \param window current window
 * \param evt DirectFB event
 */
static bool dispatch_ts_event( struct gui_window * window, DFBInputEvent *evt )
{
	int x,y;
	struct list_object * list_controls;
	struct gui_control * control;

	PRINTD( "dispatch_ts_event\n" );
	if (evt->type == DIET_AXISMOTION) {
		if (evt->flags & DIEF_AXISABS) {
			switch (evt->axis) {
				case DIAI_X:
					mouse_x = evt->axisabs;
					break;
				case DIAI_Y:
					mouse_y = evt->axisabs;
					break;
				case DIAI_Z:
				case DIAI_LAST:
					break;
			}
		}

		mouse_x = CLAMP (mouse_x, 0, screen_width  - 1);
		mouse_y = CLAMP (mouse_y, 0, screen_height - 1);
	}
	else if (evt->type == DIET_BUTTONPRESS){
		if ( is_playing_video == true || is_playing_audio == true ) handle_mouse_event( mouse_x, mouse_y );
		else if( window->redirect_ts_event_to_controls ){
			list_controls = window->controls;
			while( list_controls != NULL ){
				control = (struct gui_control *) list_controls->object;
				x = mouse_x;
				y = mouse_y;

				if( is_gui_ctrl_selected( control, &x, &y ) )
					if( control->callback != NULL){
						return control->callback(window, control->param, x, y);
					}
				list_controls = list_controls->next;
			}
		}
		else if( window->callback!= NULL ) return window->callback(window, NULL, x, y);
	}

	return true;
}

/**
 * \fn static bool load_main_config( void )
 * \todo move this piece of configuration to the main configuration structure in config.h/config.c
 */
static bool load_main_config( void ){
	dictionary * ini ;
	char *s;

	memset( &gui_config, 0, sizeof( struct gui_config ) );

	ini = iniparser_load("./conf/gui.cfg");
	if (ini == NULL) {
		PRINTD( "Unable to load main config file\n");
		return false ;
	}

	if( !is_wide_screen ){
		s = iniparser_getstring(ini, "general:first_window", NULL);
		if( s != NULL ) gui_config.first_window = strdup( s );


		s = iniparser_getstring(ini, "general:messagebox_window", NULL);
		if( s != NULL ) gui_config.messagebox_window = strdup( s );
	}
	else{
		s = iniparser_getstring(ini, "general:first_window_ws", NULL);
		if( s != NULL ) gui_config.first_window = strdup( s );

		s = iniparser_getstring(ini, "general:messagebox_window_ws", NULL);
		if( s != NULL ) gui_config.messagebox_window = strdup( s );
	}

	iniparser_freedict(ini);

	return true;
}


int main( int argc, char *argv[] )
{
	bool quit = false;
	DFBInputEvent evt;

	init_engine();
	load_main_config();
    if( load_config(&config) == false ){
        fprintf( stderr, "Error while loading config\n" );
        exit(1);
    }
	init_resources( argc, argv );


	memset( &main_window, 0, sizeof( struct gui_window ) );

	load_window( gui_config.messagebox_window, &messagebox_window );

	show_param_window( &main_window, gui_config.first_window, 0, 0 );
	while( !quit ){
		keybuffer->WaitForEventWithTimeout( keybuffer, 0, 200 );
		while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
			quit = !dispatch_ts_event( &main_window, &evt );
		}

        if( ( is_playing_video == true || is_playing_audio == true ) && is_mplayer_finished == true ){
            is_playing_video = false;
            is_playing_audio = false;


            /* Turn ON screen if it is not */
            pwm_resume();
            draw_window( &main_window );
        }
        /* Test OFF button */
        if (power_is_off_button_pushed()){
        	display_image_to_fb( config.bitmap_exiting );
        	exit(0);
        }

	}
	release_engine();

	release_resources();


	return 0;
}



