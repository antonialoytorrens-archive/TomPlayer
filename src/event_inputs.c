/**
 * \file event_inputs.c
 * \author Wolfgar 
 * \brief This module deals with inputs whatever the are (key posted in FIFO or event from touchscreen)
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tslib.h"
#include "debug.h"
#include "engine.h"
#include "widescreen.h"
#include "font.h"
#include "pwm.h"
#include "engine.h"
#include "gps.h"

static int selected_ctrl_idx;

static bool ctrl_is_selectable (enum SKIN_CMD type){
  bool ret;
  switch (type){
    case SKIN_CMD_PAUSE :
    case SKIN_CMD_STOP :
    case SKIN_CMD_MUTE :
    case SKIN_CMD_VOL_MOINS :
    case SKIN_CMD_VOL_PLUS : 
    case SKIN_CMD_LIGHT_MOINS :
    case SKIN_CMD_LIGHT_PLUS :
    case SKIN_CMD_DELAY_MOINS :
    case SKIN_CMD_DELAY_PLUS :
    case SKIN_CMD_GAMMA_MOINS :
    case SKIN_CMD_GAMMA_PLUS :
    case SKIN_CMD_FORWARD :
    case SKIN_CMD_BACKWARD :
    case SKIN_CMD_NEXT : 
    case SKIN_CMD_PREVIOUS :
      ret = true;
      break;
    case SKIN_CMD_BATTERY_STATUS :
    case SKIN_CMD_ANIM :
    default :
      ret = false;
      break;       
  }
  return ret;
}

static int get_ctrl(int idx, int step){
  int checked_idx = 0;
  int current_idx = idx;    
  const struct skin_config * skin = state_get_current_skin();
  
  while (checked_idx < skin->nb){
    checked_idx++;
    current_idx += step;
    if (current_idx >= skin->nb) current_idx = 0;
    if (current_idx < 0) current_idx = (skin->nb-1);
    if (skin->controls[current_idx].type < PROGRESS_SKIN_CONTROL_X) 
      if (ctrl_is_selectable(skin->controls[current_idx].cmd)){
        return current_idx;        
      }
  }
  return -1;
}

static void alarm_handler(int sig) { 
 return;
}

static void handle_key(DFBInputDeviceKeyIdentifier id){
  static bool is_selection_active = false;
  int new_idx = -1;  
  const struct skin_config * skin = state_get_current_skin();  
    
  if (is_selection_active){
    if ( ask_menu() == 0 ){
    switch(id){ 
      case DIKI_F10:        /*menu*/ 
      case DIKI_BACKSPACE : /*back*/
        is_selection_active = false;
        /* update selected control */   
        control_set_select(&skin->controls[selected_ctrl_idx], false);  
        handle_gui_cmd(SKIN_CMD_EXIT_MENU, -1); 
        break;   
      case DIKI_KP_4: 
      case DIKI_LEFT :
        new_idx = get_ctrl(selected_ctrl_idx, -1);
        break;
      case DIKI_KP_6:
      case DIKI_RIGHT :
        new_idx = get_ctrl(selected_ctrl_idx, +1);          
        break;   
      case DIKI_KP_MINUS: /*top left*/
        pwm_modify_brightness(-5);
        break;
      case DIKI_KP_PLUS:  /*top right*/
        pwm_modify_brightness(5);
        break;  
      case DIKI_DOWN :
      case DIKI_UP : 
          break;
      case DIKI_ENTER :{        
        handle_gui_cmd(skin->controls[selected_ctrl_idx].cmd, -1);
        break;
      }
      case DIKI_F9:  /*Map*/                               
      case DIKI_F5:  /*dest*/
      case DIKI_F6:  /*repeat*/
      case DIKI_F7:  /*light*/
      case DIKI_F8:  /*info*/
        break;      
      default :
        break;  
    }
    }
  } else {
    switch(id){
      case DIKI_F10: /*menu*/ 
        ask_menu();
        is_selection_active = true;
        new_idx = selected_ctrl_idx;
        break;
      case DIKI_BACKSPACE : /*back*/
        handle_gui_cmd(SKIN_CMD_STOP, -1);        
        break; 
      case DIKI_KP_4:
      case DIKI_LEFT :        
        handle_gui_cmd(SKIN_CMD_BACKWARD,-1);
        break;
      case DIKI_KP_6:
      case DIKI_RIGHT :
        handle_gui_cmd(SKIN_CMD_FORWARD,-1);
        break;   
      case DIKI_KP_MINUS: /*top left*/
        pwm_modify_brightness(-5);
        break;
      case DIKI_KP_PLUS:  /*top right*/
        pwm_modify_brightness(5);
        break;  
      case DIKI_DOWN :
        handle_gui_cmd(SKIN_CMD_NEXT,-1);  
        break;
      case DIKI_UP : 
        handle_gui_cmd(SKIN_CMD_PREVIOUS,-1);  
        break;
      case DIKI_ENTER :
        handle_gui_cmd(SKIN_CMD_PAUSE,-1);  
        break;      
      case DIKI_F8:  /*info*/  
      case DIKI_F9:  /*Map*/  
        eng_display_time();          
        break;
      case DIKI_F5:  /*dest*/
        handle_gui_cmd(SKIN_CMD_MUTE,-1);
        break;
      case DIKI_F6:  /*repeat*/        
        handle_gui_cmd(SKIN_CMD_VOL_MOINS, -1);
        break;
      case DIKI_F7:  /*light*/
        handle_gui_cmd(SKIN_CMD_VOL_PLUS, -1);
        break;
      default :
        break;  
    }
  }
  if (new_idx != -1){
    /* update selected control */   
    control_set_select(&skin->controls[selected_ctrl_idx], false);  
    control_set_select(&skin->controls[new_idx], true);
    selected_ctrl_idx = new_idx;      
  }  
}

static void handle_ts_coord(int x, int y){
  int p;
  int cmd;
  if (ask_menu() == 0){
    cmd = get_command_from_xy( x, y, &p );
    handle_gui_cmd(cmd, p);
  }
}

static void handle_ts(struct ts_sample * sample){
  static int last_pressure = 0;
  static int h,w; 
  static bool is_rotated;
  
  if  (h == 0){
    is_rotated = ws_are_axes_inverted();
    ws_get_size(&h,&w);
  }
  
  if (sample->pressure > 0){
    if  (last_pressure == 0){
      /*printf("delievring events %i %i %i \n", samp.x, samp.y,samp.pressure);*/
      if (is_rotated){
#ifdef NATIVE
        handle_ts_coord( sample->y, h-sample->x);
#else
        handle_ts_coord( sample->x, sample->y);
#endif
      } else {
        handle_ts_coord( sample->x, sample->y);
      }
      last_pressure = sample->pressure ;
    }
  } else {
    /*printf("Not delievred  event %i %i %i \n", samp.x, samp.y,samp.pressure);*/
    last_pressure = 0;
  }
}

void event_loop(void){
  struct sigaction new_action;
  struct tsdev *ts;
  char *tsdevice=NULL;
  struct ts_sample samp;
  int ts_samp;
  bool ts_available = true;
  int input_fd;
  DFBInputDeviceKeyIdentifier key;
  const struct skin_config * skin;
  #define TIMER_PERIOD_US 100000
  const struct itimerval timer_value = {.it_interval = {0,TIMER_PERIOD_US},
                                        .it_value = {0,TIMER_PERIOD_US}
                                       };               

  printf("Enter Main event loop\n");
  
  /*Handler on alarm signal */
  new_action.sa_handler=alarm_handler;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags=0;
  sigaction(SIGALRM, &new_action, NULL);
  
  /* Initialize GPS module */
  gps_init();
  
  if( (tsdevice = getenv("TSLIB_TSDEVICE")) != NULL ) {
    ts = ts_open(tsdevice,0);
    if ((ts == NULL) || (ts_config(ts) != 0)){
      perror("ts_config");
      ts_available = false;            
    }    
  } else {
    ts_available = false;
  }
  printf("Touchscreen availability : %d\n", ts_available);
  /* FIXME force ts availability */
  ts_available = false;
    
  /* Try to open tomplayer inputs FIFO */
  input_fd = open(KEY_INPUT_FIFO,O_RDONLY|O_NONBLOCK);
  printf("FIFO availability : %d\n", input_fd);   

  if (input_fd > 0){
    /* Purge FIFO events */
    while (read(input_fd, &key, sizeof(key)) > 0);
    /* Initialize selected control */
    skin = state_get_current_skin();      
    if (skin->first_selection >= 0) {
        selected_ctrl_idx = skin->first_selection;        
    } else {
        selected_ctrl_idx = get_ctrl(0, +1);
    }    
  }
  if (ts_available){
    /* Purge touchscreen events */
    do {
      /* To interrupt blocking read */
      alarm(1);
      if (ts_read(ts, &samp, 1) < 0) {
        break;
      }
    } while (samp.pressure == 0);
  } else {    
    /* No touchscreen available */
    if (input_fd > 0){
      int flags;
      ts_samp = 0;      
      /* FIFO read is made blocking */      
      flags = fcntl(input_fd, F_GETFL);
      flags &= (~O_NONBLOCK);
      fcntl(input_fd, F_SETFL, flags);      
    } else {
      /* No inputs available */
      fprintf(stderr, "no inputs available...\n");
      return;
    }
  }
  
  /* Main events loop */
  while (is_mplayer_finished == false) {    
    /* To interrupt blocking read */
    setitimer(ITIMER_REAL, &timer_value, NULL);      
    if (ts_available){
      ts_samp = ts_read(ts, &samp, 1);    
    }
    if (ts_samp < 1){
      if (input_fd > 0){
        if (read(input_fd, &key, sizeof(key)) > 0) {      
          handle_key(key);          
        } else {
            if (errno != EINTR) /*&& (errno != EAGAIN))*/{
                fprintf(stderr, "spurious wakeup : %d \n", errno);                
                usleep(TIMER_PERIOD_US);
            }
        }
      }
    } else {
      handle_ts(&samp);
    }
    /* Update info from GPS */
    gps_update() ;
  }
}
