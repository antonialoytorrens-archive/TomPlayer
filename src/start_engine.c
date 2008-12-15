/**
 * \file start_engine.c
 * \author nullpointer & Wolfgar 
 * \brief This module is a launcher for mplayer
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


#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#include "tslib.h"
#include "debug.h"
#include "engine.h"
#include "widescreen.h"
#include "font.h"
#include "fm.h"
#include "sound.h"
#include "diapo.h"

static void alarm_handler(int sig) { 
 return;
}

static void get_events(void){
  struct tsdev *ts;
  char *tsdevice=NULL;
  struct ts_sample samp;
  int ret;
  int last_pressure = 0;
  int h,w; 
  bool is_rotated;

  if( (tsdevice = getenv("TSLIB_TSDEVICE")) != NULL ) {
          ts = ts_open(tsdevice,0);
  } else {
    return ;
  }

  if (ts_config(ts)) {
          perror("ts_config");
          return;
  }

  is_rotated = ws_are_axes_inverted();
  ws_get_size(&h,&w);
  
  /* Purge events */
  do {
    if (ts_read(ts, &samp, 1) < 0) {
      break;
    }
  } while (samp.pressure == 0);

  /* Main events loop */
  while (is_mplayer_finished == false) { 
    /* To interrupt blocking read */
    alarm(1);
    ret = ts_read(ts, &samp, 1);
    if (ret < 0) {
            perror("ts_read");
            continue;
    }
    if (ret != 1)
            continue;

    if (samp.pressure > 0){
      if  (last_pressure == 0){
        /*printf("delievring events %i %i %i \n", samp.x, samp.y,samp.pressure);*/
        if (is_rotated){
#ifdef NATIVE
           handle_mouse_event( samp.y, h- samp.x);
#else
           handle_mouse_event( samp.x, samp.y);
#endif
        } else {
          handle_mouse_event( samp.x, samp.y);
        }
        last_pressure = samp.pressure ;
      }
    } else {
      /*printf("Not delievred  event %i %i %i \n", samp.x, samp.y,samp.pressure);*/
      last_pressure = 0;
    }
  }
}


int main( int argc, char *argv[] ){
  struct sigaction new_action;

  if (argc != 3){
    return -1;
  }

  /*Handler on alarm signal */
  new_action.sa_handler=alarm_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags=0;
  sigaction(SIGALRM, &new_action, NULL);

  init_engine();	


  if( load_config(&config) == false ){
    fprintf( stderr, "Error while loading config\n" );
  } else {
    /* Activate FM transmitter if needed */
    if (config.fm_transmitter){
      if (!fm_set_state(1) ||
          !fm_set_freq(config.fm_transmitter) ||
          !fm_set_power(115) ||
          !fm_set_stereo(1)){
        fprintf(stderr,"Error while activating FM transmitter\n");
      }
      snd_mute_internal(true);
    }
    if (config.diapo_enabled){
      if (diapo_init(&config.diapo) == false){
        fprintf(stderr,"Diaporama initialization failed ...\n");
        config.diapo_enabled = false;
      }
    }
    launch_mplayer("", argv[1], atoi(argv[2]));
  }
  get_events();
  /* Desactivate FM transmitter if needed */
  if (config.fm_transmitter){
    fm_set_state(0);
    snd_mute_internal(false);
  }
  release_engine();
  return 0;
}
