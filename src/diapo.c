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
#include <time.h>

#include "widescreen.h"
#include "skin.h"
#include "engine.h"
#include "file_list.h"
#include "pwm.h"
#include "font.h"
#include "gps.h"
#include "draw.h"
#include "diapo.h"


static struct {
  char * path;
  unsigned int delay ;   
  regex_t compiled_re;
  flenum pict_list; 
  bool end_asked; 
  int screen_x;
  int screen_y;
  pthread_t thread_id;
  pthread_mutex_t mutex;
  bool error;
  bool inv_axes;         /**< Are the axes inverted */
  enum diapo_type type;
} diapo_state = {
  .mutex = PTHREAD_MUTEX_INITIALIZER
};


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

  return true;
}

static void img_transition(ILuint cur ,ILuint next){
  int init_bright;
  int bright;

  pwm_get_brightness(&init_bright);
  for (bright=init_bright; bright>=1; bright--) {
    pwm_set_brightness(bright);
    usleep(15000);
    if (diapo_state.end_asked){
      pwm_set_brightness(init_bright);
      break;
   }
  }
  draw_img(next);  
  for (bright=1; bright<=init_bright; bright++) {
    pwm_set_brightness(bright);
    usleep(15000);
    if (diapo_state.end_asked){
       pwm_set_brightness(init_bright);
      break;
    }
  }
  return;
}

static inline void wait_next_cycle(int delay){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);      
    ts.tv_sec  += delay;
    if (pthread_mutex_timedlock(&diapo_state.mutex, &ts) == 0){
      pthread_mutex_unlock(&diapo_state.mutex);
    }
}

static void * diapo_thread(void *param){
  
  bool no_file = false;
  const char * next_file;
  static ILuint current_image_id, next_image_id;  
  int loop;
  loop = 0;
  

  while (!diapo_state.end_asked){
    do{   
      next_file = flenum_get_next_file(diapo_state.pict_list,true);    
      if (next_file == NULL){            
        flenum_release(diapo_state.pict_list);
        init_list();
        next_file = flenum_get_next_file(diapo_state.pict_list,true);    
        loop++;
        if ((loop>=2) || (next_file == NULL)) {
          no_file = true;
        }
    }} while ( (!no_file) && (!skin_load_bitmap(&next_image_id, next_file)) ) ;
    if (no_file) break;   
    loop = 0;
    if (diapo_state.inv_axes){
        iluScale(diapo_state.screen_y, diapo_state.screen_x,1);
    } else{
        iluScale(diapo_state.screen_x, diapo_state.screen_y,1);
    }
    if (current_image_id != 0){
      img_transition(current_image_id, next_image_id);
      ilDeleteImages( 1, &current_image_id);
    }
    current_image_id = next_image_id;
    next_image_id = 0;

    wait_next_cycle(diapo_state.delay);
  }
  if (current_image_id != 0){
    ilDeleteImages( 1, &current_image_id);
  }
  if (no_file)
    diapo_state.error = true;
  return NULL;
}
    
static void draw_string(ILuint back_id, const char * text, int x, int y, int size){
    int text_width, text_height;
    struct font_color  color ;     
    ILuint text_id;
    unsigned char * text_buffer;  
     
    /* Set clok color */
    color.r = 255;
    color.g = 255;
    color.b = 255;   
    
    ilGenImages(1, &text_id);
    ilBindImage(text_id);    
    font_change_size(size);      
    font_draw(&color, text, &text_buffer, &text_width, &text_height);        
    font_restore_default_size();
    ilTexImage(text_width, text_height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, text_buffer);  
    iluFlipImage();
 
    ilBindImage(back_id);  
    ilOverlayImage(text_id, x, y, 0);
    
    free(text_buffer);
    ilDeleteImages( 1, &text_id);
    return;
}

                          
void clock_thread(void){
  char buff_text[32];
  time_t curr_time;
  struct tm * ptm;   
  unsigned char * img_buffer;   
  int text_width, text_height, orig;  
  struct gps_data info;
  int y, i;
  ILuint  img_id; 
 

  /* Generate ILut image for background */
  img_buffer = calloc(4 * diapo_state.screen_x * diapo_state.screen_y, 1); 
  for (i = 0; i < diapo_state.screen_x * diapo_state.screen_y; i++){
      img_buffer[i*4 + 3] = 255;
  }
  /* FIXME cas degrade */
  ilGenImages(1, &img_id);   
  memset (&info, 0, sizeof(struct gps_data));
  
  while (!diapo_state.end_asked){
    /* Get GPS info */
    gps_get_data(&info);
    
    /* Set background to black */
    memset(img_buffer, 0, 4 * diapo_state.screen_x * diapo_state.screen_y);     
    for (i = 0; i < diapo_state.screen_x * diapo_state.screen_y; i++){
      img_buffer[i*4 + 3] = 255;
    }
    ilBindImage(img_id);
    ilTexImage(diapo_state.screen_x, diapo_state.screen_y, 1, 
               4, IL_RGBA, IL_UNSIGNED_BYTE, img_buffer);
               
    
    /* Display time */
    time(&curr_time);
    ptm = localtime(&curr_time);
    snprintf(buff_text,sizeof(buff_text),"%02d : %02d",ptm->tm_hour, ptm->tm_min);   
    font_change_size(50);      
    font_get_size(buff_text, &text_width, &text_height, &orig);
    draw_string(img_id, buff_text,  diapo_state.screen_x - text_width - 20, 20, 50);
     

    /* Display GPS infos */    
    y = 20;
    snprintf(buff_text,sizeof(buff_text),"Lat    : %02i %02i", info.lat_deg, info.lat_mins);
    font_change_size(15); 
    font_get_size(buff_text, &text_width, &text_height, &orig);
    draw_string(img_id, buff_text,  20, y, 15);
    y += text_height + 15;
    snprintf(buff_text,sizeof(buff_text),"Long : %02i %02i", info.long_deg, info.long_mins);  
    draw_string(img_id, buff_text,  20, y, 15);    
    y += text_height + 15;
    snprintf(buff_text,sizeof(buff_text),"Alt     : %04i m", info.alt_cm / 100);
    draw_string(img_id, buff_text,  20, y, 15);        
    y += text_height + 15;
    
    /* Display speed */    
    snprintf(buff_text,sizeof(buff_text),"%03i km/h", info.speed_kmh);
    font_change_size(40);      
    font_get_size(buff_text, &text_width, &text_height, &orig);
    draw_string(img_id, buff_text, (diapo_state.screen_x - text_width) / 2, 
                (diapo_state.screen_y + y - text_height) / 2, 40);            

    /* Display final image on framebuffer */
    
    ilBindImage(img_id);      
    ilCopyPixels(0, 0, 0, diapo_state.screen_x, diapo_state.screen_y, 1,
                 IL_RGBA, IL_UNSIGNED_BYTE, img_buffer);    
    draw_RGB_buffer(img_buffer, 0, 0, diapo_state.screen_x, diapo_state.screen_y, true);        
    
    wait_next_cycle(1);
  }
  free(img_buffer);    
  ilDeleteImages( 1, &img_id);
}

static void * periodic_thread(void *param){
    if (diapo_state.type == DIAPO_CLOCK ){
        clock_thread();
    } else {
        diapo_thread(param);
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
  if (conf->type != DIAPO_CLOCK){
    if (init_list() == false){
        diapo_release();
        return false;
    }  
  }
  if (ws_get_size(&diapo_state.screen_x, &diapo_state.screen_y) == false){
    diapo_release();
    return false;
  }
  
  diapo_state.inv_axes = ws_are_axes_inverted();
  diapo_state.type = conf->type;
  return true;
}

bool diapo_resume (void){
  pthread_attr_t attr;

  if ((diapo_state.error) ||
      (diapo_state.thread_id != 0)){
    return false;
  }
  pthread_attr_init(&attr);   
  diapo_state.end_asked = false;  
  pthread_mutex_lock(&diapo_state.mutex);
  if (pthread_create(&diapo_state.thread_id, &attr, periodic_thread, NULL) != 0){
   pthread_attr_destroy(&attr);
    return false;
  }
  pthread_attr_destroy(&attr);
  return true;
}

bool diapo_stop (void){  
  if (diapo_state.thread_id == 0){
    return false;
  }  
  diapo_state.end_asked = true;   
  pthread_mutex_unlock(&diapo_state.mutex);
  pthread_join(diapo_state.thread_id, NULL);
  diapo_state.thread_id = 0;
  return true;
}

bool diapo_get_error(void){
  return diapo_state.error;
}
