/**
 * \file gui.c
 * \author nullpointer & Wolfgar
 * \brief This module implements the main IHM
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
 */


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
#include <direct/util.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*#include <dictionary.h>
#include <iniparser.h>*/
/*#include "gui.h"*/
#include "window.h"
/*#include "gui_control.h"*/
/*#include "callbacks.h"*/
#include "debug.h"
/*#include "window_context.h"*/
#include "engine.h"
#include "pwm.h"
#include "power.h"


#define CFG_FOLDER "./conf"

enum gui_screens_type {
  GUI_SCREEN_MAIN,
  GUI_SCREEN_SELECT_SETTINGS,
  GUI_SCREEN_AUDIO_SKIN,
  GUI_SCREEN_VIDEO_SKIN,
  GUI_SCREEN_AUDIO,
  GUI_SCREEN_VIDEO,
  GUI_SCREEN_ABOUT,
  GUI_SCREEN_SPLASH,
  GUI_SCREEN_MAX
};

static const char * graphic_conf_files[GUI_SCREEN_MAX] = {"main.cfg",
                                                   "settings.cfg",
                                                   "skin_video.cfg",
                                                   "skin_audio.cfg",
                                                   "audio.cfg",
                                                   "video.cfg",
                                                   "about.cfg",
                                                   "splash.cfg"
                                                  };

static char  graphic_conf_folder[32];

static IDirectFB	      *dfb;
static IDirectFBDisplayLayer  *layer;   
static IDirectFBEventBuffer   *keybuffer;
static bool is_rotated = false;


inline static const char * get_full_conf(enum gui_screens_type screen_type){
  static char buff[64];
  snprintf(buff, sizeof(buff) - 1, "%s%s",graphic_conf_folder , graphic_conf_files[screen_type]);
  return buff;    
}

/* Splash screen related code */
static void enter_main_screen(struct gui_control * ctrl, int x, int y){
   /* Dont want to keep the splash screen window => release it */
   gui_window_release(ctrl->win);
   gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_MAIN)); 
   
   return;
}

static bool load_first_screen(void){
  gui_window  win;
  win = gui_window_load(dfb, layer, get_full_conf(GUI_SCREEN_SPLASH));
  if (win == NULL){
    return false;
  }
  gui_window_attach_cb(win, "splash_screen", enter_main_screen);
  return true;
  
}

/** release DirectFB */
static void release_resources( void )
{
	PRINTD( "deinit_resources\n");
        /* TODO free all windows or we consider it is already performed ? */
        if (layer != NULL)
          layer->Release(layer);
        if (keybuffer != NULL)
	 keybuffer->Release( keybuffer );
        if (dfb != NULL)
	 dfb->Release( dfb );
}


/** Set up DirectFB and load resources
 *
 * \param argc argument count
 * \param argv argument values
 */
static bool init_resources( int argc, char *argv[] ) {
        int screen_width, screen_height, temp;	
	DFBSurfaceDescription dsc;	
        IDirectFBSurface * primary;
        struct stat stats;

	PRINTD( "init_resources\n" );

        if (
	(DirectFBInit( &argc, &argv ) != DFB_OK ) ||
	(DirectFBCreate( &dfb ) != DFB_OK  ) ||
	(dfb->CreateInputEventBuffer( dfb, DICAPS_ALL,DFB_FALSE, &keybuffer ) != DFB_OK  ) ||
	(dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN ) != DFB_OK  ) ||
        (dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ) != DFB_OK )  ||
        (layer->SetCooperativeLevel( layer, DLSCL_EXCLUSIVE ) !=  DFB_OK )  ||  
        (layer->EnableCursor (layer, 0 )  != DFB_OK )){
          return false;
        }

	dsc.flags = DSDESC_CAPS;
	dsc.caps = DSCAPS_PRIMARY;
	if (dfb->CreateSurface( dfb, &dsc, &primary ) != DFB_OK ){
          return false;
        }


	if (primary->GetSize( primary, &screen_width, &screen_height ) !=DFB_OK) {
            primary->Release(primary);
            return false;
        }

	if( screen_width < screen_height ){
		is_rotated = true;		
                temp = screen_height;
                screen_height = screen_width;
                screen_width = temp;
	}

        primary->Release(primary);
        snprintf(graphic_conf_folder, sizeof(graphic_conf_folder) - 1, "%s/%i_%i/", CFG_FOLDER, screen_width, screen_height);
        if (stat(graphic_conf_folder,&stats) != 0){
          PRINTDF("No available folder for your screen : %i x %i \n",screen_width,screen_height );
          return false;
        }

        return true;
}


/** Dispatch a touch screen event to the control selected or to the skin
 *
 * \param window current window
 * \param evt DirectFB event
 */
static bool dispatch_ts_event(DFBInputEvent *evt )
{
  static int mouse_x=0, mouse_y=0;

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
/*
          mouse_x = CLAMP (mouse_x, 0, screen_width  - 1);
          mouse_y = CLAMP (mouse_y, 0, screen_height - 1);
*/
  }
  else if (evt->type == DIET_BUTTONPRESS ){
          /* redirect TS event to handle_mouse_event when MPlayer is running */
          if ( is_playing_video == true || is_playing_audio == true ) {
            handle_mouse_event( mouse_x, mouse_y );
          }
          else{
            gui_window_handle_click( mouse_x, mouse_y);
          }
  }

  return true;
}


int main( int argc, char *argv[] )
{
	bool quit = false;
	DFBInputEvent evt;

	init_engine();
	init_resources( argc, argv );
	/*load_main_config();*/

	if( load_config(&config) == false ){
		fprintf( stderr, "Error while loading config\n" );
		exit(1);
	}
        if (load_first_screen()){

  /*	default_font = load_font( gui_config.default_font, gui_config.font_height );
          show_param_window( main_window, gui_config.first_window, 0, 0 );
  */
          while( quit == false ){
                  keybuffer->WaitForEventWithTimeout( keybuffer, 0, 100 );
                  while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
                          if( dispatch_ts_event( &evt ) == false ) quit=true;
                  }
  
                  if( ( is_playing_video == true || is_playing_audio == true ) && is_mplayer_finished == true ){
                          is_playing_video = false;
                          is_playing_audio = false;		
                          /* Turn ON screen if it is not */
                          pwm_resume();
                          /* TODO refresh display draw_window( main_window )*/;
                  }
                  /* Test OFF button */
                  if (power_is_off_button_pushed()){
                          display_image_to_fb( config.bitmap_exiting );
                          quit = true;
                  }
          }
        }

	release_engine();
	release_resources();

	exit(0);
}


#if 0

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
 * draw the window given in argument to the directfb surface given
 *
 * \param window the window
 * \param surface the directFB surface where the window has to be drawn
 */
void draw_window_on_surface( struct gui_window * window, IDirectFBSurface * surface)
{
	struct list_object * list_controls;
	struct gui_control * control;
	char * p, *s;
	int y;
	int font_height;

	PRINTD( "draw_window_on_surface\n" );
	surface->SetBlittingFlags( surface,DSBLIT_BLEND_ALPHACHANNEL);
	surface->SetColor( surface, window->r, window->g, window->b, window->a );

	if( window->font != NULL ){
		PRINTD( "Using window's font\n" );
		surface->SetFont( surface, window->font );
		window->font->GetHeight( window->font, &font_height );
	}
	else{
		PRINTD( "Using default font\n" );
		surface->SetFont( surface, default_font );
		default_font->GetHeight( default_font, &font_height );
	}

	surface->Clear( surface, 0, 0, 0, 0xFF );

	if( window->background_surface != NULL ){
		PRINTD( "Drawing background surface\n" );
		surface->Blit( surface, window->background_surface, NULL, 0, 0 );
	}

	list_controls = window->controls;
	while( list_controls != NULL ){
		control = (struct gui_control *) list_controls->object;
		PRINTDF( "Affichage du control de type %d\n", control->type );
		switch( control->type ){
			case GUI_TYPE_CTRL_STATIC_TEXT:
				p = get_static_text( control->param );
				s = strdup( p );
				p = s;
				y = control->y;
				p = strtok( p, "\n" );
				while( p!= NULL ){
					surface->DrawString( surface, p, -1,control->x, y,  DSTF_CENTER | DSTF_TOP );
					p=strtok( NULL, "\n" );
					y += font_height;
				}
				free( s );
				break;
			case GUI_TYPE_CTRL_BUTTON:
				surface->Blit( surface, control->bitmap_surface, NULL, control->x, control->y );
				break;
			case GUI_TYPE_CTRL_LISTVIEW:
			case GUI_TYPE_CTRL_LISTVIEW_ICON:
				surface->Blit( surface, control->bitmap_surface, NULL, control->x, control->y );
				break;
		}

		list_controls = list_controls->next;
	}


}

/**
 * Rotate the surface given in parameter
 *
 * \param surface the directFB surface to rotate
 */
void rotate_surface( IDirectFBSurface ** surface ){
	int w,h,pitch_src,pitch_dest;
	int x,y;
	void * ptr_src;
	void * ptr_dest;
	DFBSurfacePixelFormat pixel_format;
	IDirectFBSurface * dest_surface;
	IDirectFBSurface * src_surface;
	u32 *pixel_src;
	u32 *pixel_dest;

	src_surface = *surface;

	src_surface->GetSize( src_surface, &w, &h );
	src_surface->GetPixelFormat( src_surface, &pixel_format );

	dest_surface = create_surface( h, w );

	src_surface->Lock( src_surface, DSLF_READ, &ptr_src, &pitch_src );
	dest_surface->Lock( dest_surface, DSLF_WRITE, &ptr_dest, &pitch_dest );
	for( y = 0; y < h;y++ ){
		for( x = 0; x < w; x++ ){
			pixel_src = (u32 *) ( ptr_src + y * pitch_src + x * (pitch_src/w ) );
			pixel_dest = (u32 *) ( ptr_dest + x * pitch_dest + y * (pitch_dest/h ) );
			*pixel_dest = *pixel_src;
		}
	}
	dest_surface->Unlock( dest_surface );
	src_surface->Unlock( src_surface );

	/* swap surface and release unused surface */
	*surface = dest_surface;
	src_surface->Release( src_surface );
}

/**
 * Draw window on the screen
 *
 * \param window the window
 */
void draw_window( struct gui_window * window ){
	IDirectFBSurface * surface;
	if( is_rotated == true ){
		surface = create_surface( screen_width, screen_height );
		draw_window_on_surface( window, surface );
		rotate_surface( &surface );
		primary->Blit( primary, surface, NULL, 0, 0 );
		surface->Release( surface );
	}
	else draw_window_on_surface( window, primary);

#ifdef USE_DOUBLE_BUFFER
	primary->Flip( primary, NULL, DSFLIP_ONSYNC );
#endif /* USE_DOUBLE_BUFFER */
}



/**
 * \fn void show_message_box( char * title, char * message )
 * \brief show a message box on the screen
 *
 * \param title title of the message box
 * \param message message to display
 */
void show_message_box( char * title, char * message ){
	message_box( title, message, true );
}

/**
 * \fn void show_information_message( char * msg )
 * \brief show a message over the current window
 *
 * \param msg message to display
 */
void show_information_message( char * message ){
	message_box( "", message, false );
}



/**
 * \fn void message_box( char * title, char * message )
 * \brief show a message box on the screen
 *
 * \param title title of the message box
 * \param message message to display
 * \param wait_input boolean which indicate if window has to wait an input to 
 */
static void message_box( char * title, char * message, bool wait_input ){
	DFBInputEvent evt;
	struct gui_window * messagebox_window;

	context.title = title;
	context.message = message;

	messagebox_window = load_window( gui_config.messagebox_window, false );

	if( messagebox_window != NULL ){
		draw_window( messagebox_window );
		unload_window( messagebox_window, false );
	
		if( wait_input == true ){
			while ( 1 ) {
				keybuffer->WaitForEvent( keybuffer );
				if( keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK )
					if (evt.type == DIET_BUTTONPRESS) break;
			}
			
			/* Show main window */
			draw_window( main_window );
		}
	}
}


/**
 * \todo move this piece of configuration to the main configuration structure in config.h/config.c
 */
static bool load_main_config( void ){
	dictionary * ini ;
	char *s;
	char key[200];

	memset( &gui_config, 0, sizeof( struct gui_config ) );

	ini = iniparser_load("./conf/gui.cfg");
	if (ini == NULL) {
		PRINTD( "Unable to load main config file\n");
		return false ;
	}

	s = iniparser_getstring(ini, "general:font", NULL);
	if( s != NULL ) gui_config.default_font = strdup( s );
	else gui_config.default_font = strdup( FONT );

	gui_config.font_height = iniparser_getint( ini, "general:font_height", 20 );

	sprintf( key, "%d_%d:first_window", screen_width, screen_height );
	s = iniparser_getstring(ini, key, NULL);
	if( s != NULL ) gui_config.first_window = strdup( s );

	sprintf( key, "%d_%d:messagebox_window", screen_width, screen_height );
	s = iniparser_getstring(ini, key, NULL);
	if( s != NULL ) gui_config.messagebox_window = strdup( s );

	iniparser_freedict(ini);

	return true;
}

#endif
