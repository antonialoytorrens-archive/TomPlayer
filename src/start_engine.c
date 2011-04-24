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
#include "config.h"
#include "engine.h"

int main( int argc, char *argv[] ){
  bool is_video;
  
  if (argc != 4){
    return -1;
  }
  if (config_init() == false){
   fprintf( stderr, "Error while loading config\n" );
    return -1;
  }
  is_video = false;
  if (argv[2] != NULL){
    if (strncmp(argv[3],"VIDEO",5) == 0)
      is_video = true;
  }

  eng_init(is_video);

  /* Activate FM transmitter if needed */
  if (config_get_fm_activation()){
    if (!fm_set_state(1) ||
        !fm_set_freq(config_get_fm(CONFIG_FM_DEFAULT)) ||
        !fm_set_power(115) ||
        !fm_set_stereo(1)){
      fprintf(stderr,"Error while activating FM transmitter\n");
    }
    if (config_get_speaker() == CONF_INT_SPEAKER_AUTO){
      snd_mute_internal(true);
    }
  }
  /* Initialize diaporama */
  if (config_get_diapo_activation()){
    if (diapo_init(config_get_diapo()) == false){
      fprintf(stderr,"Diaporama initialization failed ...\n");      
      config_toggle_enable_diapo();
    }
  }

  eng_play(argv[1], atoi(argv[2]));     
    
  /* Desactivate FM transmitter if needed */
  if (config_get_fm_activation()){
    fm_set_state(0);
    snd_mute_internal(false);
  }
  
  /* Free resources */
  eng_release();
  diapo_release();
  config_free();
  return 0;
}
