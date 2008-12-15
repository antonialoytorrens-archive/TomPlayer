/**
 * \file diapo.c
 * \author Stephan Rafin
 *
 * This module implements a slide show engine
 *
 * $URL: $
 * $Rev: $
 * $Author: $
 * $Date: $
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <pthread.h>

#include "widescreen.h"
#include "zip_skin.h"
#include "engine.h"
#include "file_list.h"
#include "pwm.h"
#include "diapo.h"


static struct {
  char * path;
  unsigned int delay ;   
  regex_t compiled_re;
  flenum pict_list;
  const char * first_file;
  bool end_asked; 
  int screen_x;
  int screen_y;
  pthread_t thread_id;
} diapo_state;


static inline bool init_list(void){
  file_list list;

  list = fl_create(diapo_state.path, &diapo_state.compiled_re, true);
  if (list == NULL){    
    return false;
  }
  if (fl_select_all(list) == false){
    return false;
  }
  diapo_state.pict_list = fl_get_selection(list);
  fl_release(list);

  diapo_state.first_file = flenum_get_next_file(diapo_state.pict_list,true);
  if (diapo_state.first_file == NULL){
    /* Not even a file to display ! */
    return false;
  }
  return true;
}

static void img_transition(ILuint cur ,ILuint next){
  int init_bright;
  int bright;

  pwm_get_brightness(&init_bright);
  for (bright=init_bright; bright>=1; bright--) {
    pwm_set_brightness(bright);
  }
  display_image_to_fb(next);  
  for (bright=1; bright<=init_bright; bright++) {
    pwm_set_brightness(bright);
  }
  return;
}

static void * preriodic_thread(void *param){
  const char * next_file;
  static ILuint current_image_id, next_image_id;

  if (current_image_id == 0){
    load_bitmap(&current_image_id, diapo_state.first_file);
    iluScale(diapo_state.screen_x, diapo_state.screen_y,1);
  }
  display_image_to_fb(current_image_id);

  while (!diapo_state.end_asked){
    sleep(diapo_state.delay);
    next_file = flenum_get_next_file(diapo_state.pict_list,true);

    if (next_file == NULL){
      flenum_release(diapo_state.pict_list);
      if (init_list() == false){
        break;
      } 
      next_file = diapo_state.first_file;
    }
      load_bitmap(&next_image_id, next_file);
      iluScale(diapo_state.screen_x, diapo_state.screen_y,1);
      img_transition(current_image_id, next_image_id);
      ilDeleteImages( 1, &current_image_id);
      current_image_id = next_image_id;
      next_image_id = 0;
  }
  return NULL;
}


void diapo_release(void){
  if (diapo_state.path != NULL){
    free(diapo_state.path);
    diapo_state.path = NULL;
  }

  regfree(&diapo_state.compiled_re);
 
  if (diapo_state.pict_list != NULL){
    flenum_release(diapo_state.pict_list);
    diapo_state.pict_list = NULL;
  }
  return;
}
       
bool diapo_init(const  struct diapo_config * conf ){
  diapo_state.path = strdup(conf->file_path);
  if (diapo_state.path == NULL){
    return false;
  }
  if (regcomp(&diapo_state.compiled_re, conf->filter, REG_NOSUB | REG_EXTENDED | REG_ICASE )){    
    diapo_release();
    return false;
  }
  diapo_state.delay = conf->delay;
  if (init_list() == false){
    diapo_release();
    return false;
  }
  if (ws_get_size(&diapo_state.screen_x, &diapo_state.screen_y) == false){
    diapo_release();
    return false;
  }
  return true;
}

bool diapo_resume (void){
  diapo_state.end_asked = false;
  if (pthread_create(&diapo_state.thread_id,NULL, preriodic_thread, NULL) != 0){
    return false;
  }
  return true;
}

bool diapo_stop (void){  
  if (diapo_state.thread_id == 0){
    return false;
  }
  diapo_state.end_asked = true;   
  pthread_join(diapo_state.thread_id, NULL);
  diapo_state.thread_id = 0;
  return true;
}
