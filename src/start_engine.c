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

#include <stdlib.h>
#include <string.h>

#include "fm.h"
#include "sound.h"
#include "diapo.h"
#include "engine.h"
#include "event_inputs.h"

int main( int argc, char *argv[] ){
  bool is_video;
  
  if (argc != 4){
    return -1;
  }
  if( load_config(&config) == false ){
   fprintf( stderr, "Error while loading config\n" );
    return -1;
  }
  is_video = false;
  if (argv[2] != NULL){
    if (strncmp(argv[3],"VIDEO",5) == 0)
      is_video = true;
  }

  init_engine(is_video);

  /* Activate FM transmitter if needed */
  if (config.enable_fm_transmitter){
    if (!fm_set_state(1) ||
        !fm_set_freq(config.fm_transmitter) ||
        !fm_set_power(115) ||
        !fm_set_stereo(1)){
      fprintf(stderr,"Error while activating FM transmitter\n");
    }
    if (config.int_speaker == CONF_INT_SPEAKER_AUTO){
      snd_mute_internal(true);
    }
  }
  /* Initialize diaporama */
  if (config.diapo_enabled){
    if (diapo_init(&config.diapo) == false){
      fprintf(stderr,"Diaporama initialization failed ...\n");
      config.diapo_enabled = false;
    }
  }

  launch_mplayer("", argv[1], atoi(argv[2])); 
  event_loop();
    
  /* Desactivate FM transmitter if needed */
  if (config.enable_fm_transmitter){
    fm_set_state(0);
    snd_mute_internal(false);
  }
  
  /* Free resources */
  release_engine();
  diapo_release();
  config_free();
  return 0;
}
