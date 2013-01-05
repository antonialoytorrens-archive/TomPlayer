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

#include <unistd.h>
#include <directfb.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "config.h"
#include "debug.h"
#include "engine.h"
#include "power.h"
#include "screens.h"
#include "window.h"
#include "gps.h"
#include "resume.h"
#include "pwm.h"

static IDirectFB	      *dfb;
static IDirectFBDisplayLayer  *layer;   
static IDirectFBEventBuffer   *keybuffer;


/** release DirectFB */
static void release_resources( void )
{
  PRINTD( "deinit_resources\n");

  /* Free all windows */
  gui_window_release_all();
  
  if (layer != NULL)
    layer->Release(layer);
  if (keybuffer != NULL)
    keybuffer->Release( keybuffer );   
  if (dfb != NULL)
    dfb->Release( dfb );

}

/* Auto resume function */
static int auto_resume (void){
    int pos = 0;
    struct stat ftype;
    char filename[PATH_MAX - 32];    
    char mv_command[PATH_MAX];       
    
    if (resume_get_file_infos(MODE_UNKNOWN, filename, PATH_MAX, &pos) == 0){                    
        if( stat(filename, &ftype) == 0){            
            snprintf(mv_command, sizeof(mv_command), "mv %s " RESUME_VOLATILE_PLAYLIST, filename);
            system(mv_command);             
            setup_engine(RESUME_VOLATILE_PLAYLIST, pos, (strstr(filename, RESUME_PLAYLIST_FILENAME(MODE_AUDIO)) == NULL));                
            return 0;
        }
    }
    return -1;
}
        
/** Set up DirectFB and load resources
 *
 * \param argc argument count
 * \param argv argument values
 */
static bool init_resources( int argc, char *argv[] ) {
        DFBDisplayLayerConfig layer_config;
	PRINTD( "init_resources\n" );


  if (
	(DirectFBInit( &argc, &argv ) != DFB_OK ) ||
	(DirectFBCreate( &dfb ) != DFB_OK  ) ||
	(dfb->CreateInputEventBuffer( dfb, DICAPS_ALL,DFB_FALSE, &keybuffer ) != DFB_OK  ) ||
	(dfb->SetCooperativeLevel( dfb, DFSCL_FULLSCREEN ) != DFB_OK  ) ||
        (dfb->GetDisplayLayer( dfb, DLID_PRIMARY, &layer ) != DFB_OK )  ||
        (layer->SetCooperativeLevel( layer, DLSCL_EXCLUSIVE) !=  DFB_OK )  ||  
        (layer->EnableCursor (layer, 0 )  != DFB_OK )){
          return false;
        }

        layer_config.flags = DLCONF_BUFFERMODE;
        layer_config.buffermode = DLBM_BACKSYSTEM;
        layer->SetConfiguration( layer, &layer_config );

        return true;
}


static void init_settings(void) {
    struct general_settings settings;

    if (resume_get_general_settings(&settings) == 0) {
        pwm_set_brightness(settings.brightness);
    }
}

static void save_settings(void) {
    struct general_settings settings;

    if (pwm_get_brightness(&settings.brightness) == 0) {    
        resume_set_general_settings(&settings);
    }
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
    gui_window_handle_click( mouse_x, mouse_y);
  }
  else if (evt->type == DIET_KEYPRESS){
      gui_window_handle_key(evt->key_id); 	  
  }

  return true;
}


void setup_engine(const char * path, int pos, bool is_video){
  int fd, i ;
  char buffer[128];
      
  fd = open ("/tmp/start_engine.sh", O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
  if (fd >= 0){
    i = snprintf(buffer, sizeof(buffer) - 1,"./start_engine \"%s\" %i %s\n", path, pos, is_video?"VIDEO":"AUDIO"  );
    buffer[i] = '\0';
    write(fd,buffer,i);
    fsync(fd);
    close(fd);
  }
  return;
}

/** Everything begins here ;-)  */
int main( int argc, char *argv[] ){
  static bool splash_wanted = true ;
  static bool first_launch = false;
  bool power_off_asked = false;
  
  static struct option long_options[] = {               
               {"no-splash", no_argument,(int *)&splash_wanted, 0},
               {"first-launch", no_argument,(int *)&first_launch, true},
               {0, 0, 0, 0}
               };
 
  int option_index = 0;
  int c;
  int input_fd;
  DFBInputEvent evt;

  /* Parse arguments */
  
  do {
    c = getopt_long(argc, argv, "",long_options, &option_index);
  } while (c != -1);

  /* Load tomplayer configuration file */
  if(config_init() == false){
    fprintf( stderr, "Error while loading config\n" );
    exit(1);
  }
  if (init_resources( argc, argv ) == true){
    if ((first_launch) && 
      (config_get_auto_resume())){
      if (auto_resume() == 0){
        release_resources();
        exit(0);
      }    
    }
  
    init_settings();
  
    /* Initialize GPS module */
    gps_init();
  
    /* FIFO for key events */
    input_fd = open(KEY_INPUT_FIFO,O_RDONLY|O_NONBLOCK);
    /* Purge FIFO if available */
    if (input_fd >= 0)
        while (read(input_fd,  &evt.key_id, sizeof(evt.key_id)) > 0);
  
    if (screen_init(dfb, layer, splash_wanted)){
        while( screen_is_end_asked()  == false ){
              if (keybuffer->WaitForEventWithTimeout( keybuffer, 0, 50 ) == DFB_TIMEOUT){
                if (input_fd > 0){
                    if (read(input_fd, &evt.key_id, sizeof(evt.key_id)) > 0){
                        
                        evt.type = DIET_KEYPRESS;
                        dispatch_ts_event( &evt );
                    }
                }     
                /* Update info from GPS */
                gps_update() ;
              }
              while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt)) == DFB_OK) {
                      dispatch_ts_event( &evt );
              }
              /* Test OFF button */
              if (power_is_off_button_pushed()){
                power_off_asked = true;
                break;
              }
        }
    }
    
    save_settings();
    
    /*FIXME proper release of directfb may hang...
      Pb seems to appear from time to time when releasing directfb layer : i have not found the root of this pb */
#ifndef NATIVE
    if (power_off_asked){
      exit(51);
    } else {
      exit(0);
    }
#endif
    release_resources();  
    printf("Leaving tomplayer IHM \n");
    return 0;
  }
  return -1;
}

