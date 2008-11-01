/**
 * \file gui.c
 * \author nullpointer & Wolfgar
 * \brief This module launches the main GUI
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

#include "debug.h"
#include "engine.h"
#include "pwm.h"
#include "power.h"
#include "screens.h"
#include "window.h"


static IDirectFB	      *dfb;
static IDirectFBDisplayLayer  *layer;   
static IDirectFBEventBuffer   *keybuffer;


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

  //PRINTD( "dispatch_ts_event\n" );
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

/** Everything begins here ;-)  */
int main( int argc, char *argv[] ){
	DFBInputEvent evt;

	init_engine();
	init_resources( argc, argv );	
	if( load_config(&config) == false ){
		fprintf( stderr, "Error while loading config\n" );
		exit(1);
	}
        if (screen_init(dfb, layer)){
          while( screen_is_end_asked()  == false ){
                  keybuffer->WaitForEventWithTimeout( keybuffer, 0, 100 );
                  while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
                          dispatch_ts_event( &evt );
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
                          break;
                  }
          }
        }

	release_engine();
	release_resources();
	exit(0);
}

