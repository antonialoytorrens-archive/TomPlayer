/**
 * \file engine.c
 * \author nullpointer & wolfgar 
 * \brief This module implements the core engine during playing a media
 *
 * This module implements the core engine during playing a media.
 * \li It displays and update the skins and their objects 
 * \li It Implements the logic behind the actions required by the input events
 * \li It Drives mplayer through the slave interface available in play_int.h 
 * \li It handles the screen saver triggering  
 * \li It takes cares of restoring and saving the current settings
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

#include <pthread.h>
#include <unistd.h>

#include "config.h"
#include "resume.h"
#include "widescreen.h"
#include "pwm.h"
#include "sound.h"
#include "power.h"
#include "debug.h"
#include "skin.h"
#include "font.h"
#include "diapo.h"
#include "play_int.h"
#include "event_inputs.h"
#include "draw.h"
#include "engine.h"

/* Update period in ms */
#define UPDATE_PERIOD_MS 250


static pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;
/* Current audio filename */
static char current_filename[200];

/* Engine state */
static struct{
    enum eng_mode current_mode;
    bool menu_showed;
    bool quit_asked;
}state = { .menu_showed = false, 
           .quit_asked = false
         };

/* Screen saver state */
static struct{
    bool is_running;
    time_t next_time;
    bool stop_asked;
}screen_saver_state;

/* current settings */
static struct {
  bool initialized;
  struct video_settings video;
  struct audio_settings audio;
}settings;

static void display_bat_state(bool force);

static void quit(){
  int pos;  
  
  pos = playint_get_file_position_seconds();
  if (pos > 0){
    resume_write_pos(state.current_mode, pos);
  }
  playint_quit();
  state.quit_asked = true ;
}

static void settings_init(){
    struct video_settings v_settings;
    struct audio_settings a_settings;

    if (state.current_mode == MODE_AUDIO){                
        if (resume_get_audio_settings(&a_settings) == 0){
            settings.audio = a_settings;
            settings.initialized = true;
        }
    } else {
        if (resume_get_video_settings(&v_settings) == 0){
            settings.video = v_settings;
            settings.initialized = true;
        } 
    }
}

/** 
 * \note mplayer has to run 
 */
static void settings_update(){
    struct video_settings v_settings;
    struct audio_settings a_settings;
    
    if (state.current_mode == MODE_AUDIO){
        if (settings.initialized){
            playint_set_audio_settings(&settings.audio);            
        } else {
            if (playint_get_audio_settings(&a_settings) == 0){
                settings.audio = a_settings;
                settings.initialized = true;
            }
        }
    } else {
        if (settings.initialized){
            playint_set_video_settings(&settings.video);
        } else {
            if (playint_get_video_settings(&v_settings) == 0){
                settings.video = v_settings;
                settings.initialized = true;
            }           
        }
    }
}



/** Draw text in track info zone of a skin */
static void draw_track_text(const char * text) {               
    const struct skin_config * skin_conf;
    int img_width, img_height;
    struct font_color  color ;
     
    skin_conf = skin_get_config();
    img_width = (skin_conf->text_x2 - skin_conf->text_x1) ;
    img_height = (skin_conf->text_y2 - skin_conf->text_y1);
    color.r = skin_conf->text_color>>16;
    color.g = (skin_conf->text_color&0xFF00)>>8 ;
    color.b = skin_conf->text_color&0xFF;
    draw_text(text, skin_conf->text_x1, skin_conf->text_y1, img_width, img_height, &color, 0);
}

static void display_current_filename( char * filename) {
  char artist[200];
  char title[200];
  char displayed_name[300];
  
  if (playint_get_title(title, sizeof(title)) > 0){
    if  (playint_get_artist(artist, sizeof(artist)) > 0) {
        snprintf(displayed_name, sizeof(displayed_name),"%s - %s", artist, title);
    } else {
        snprintf(displayed_name, sizeof(displayed_name),"%s", title);
    }
    displayed_name[sizeof(displayed_name)-1] = 0;
    draw_track_text(displayed_name);    
  } else {
    draw_track_text(filename);       
  }
}

/** Initialize screen saver */
static int screen_saver_init(){
    struct timespec tp;
      
    /* Timout in cycles before turning OFF screen while playing audio */
    if (config_get_screen_saver()){
        /* no CLOCK_MONOTONIC available, it is a shame... */
        clock_gettime(CLOCK_REALTIME, &tp);
        screen_saver_state.next_time = tp.tv_sec + config_get_screen_saver_to(); 
        PRINTDF("screen saver next time : %d\n", screen_saver_state.next_time);
    } else {
        screen_saver_state.next_time = 0;        
    }
    return 0;
}

static bool screen_saver_is_running(void){
    return screen_saver_state.is_running;
}

/** Reset the screen saver time out
 * \retval 1 Screen saver was running
 * \retval 0 Sacreen saver was not running
 */
static int screen_saver_reset_to(){   
    struct timespec tp;
    if (screen_saver_state.next_time != 0){
        if (state.current_mode != MODE_AUDIO){
            return 0;
        }
        if (screen_saver_state.is_running){
            screen_saver_state.stop_asked = true;               
            return 1;
        }
        clock_gettime(CLOCK_REALTIME, &tp);
        screen_saver_state.next_time = tp.tv_sec + config_get_screen_saver_to();
        PRINTDF("screen saver next time : %d\n", screen_saver_state.next_time);
    }
    return 0;    
}

/** Update the screen saver state */
static int screen_saver_update(void){
    struct timespec tp;
    if (state.current_mode != MODE_AUDIO){
        return 0;
    }
    if (screen_saver_state.next_time != 0){
        /* Check whether it is time to enter screen saver */
        clock_gettime(CLOCK_REALTIME, &tp);
        if ((tp.tv_sec >= screen_saver_state.next_time) &&   
            (!screen_saver_state.is_running)){
            PRINTDF("Entering screen scaver time %d - expected %d\n", tp.tv_sec, screen_saver_state.next_time);
            screen_saver_state.is_running = true;        
            if (config_get_diapo_activation()){
                diapo_resume();
            } else {
                pwm_off();
            }      
        }
        /* If problem with diapo then fall back on black screen*/
        if (screen_saver_state.is_running){
            if (config_get_diapo_activation()){
                if (diapo_get_error()){
                    /*config.diapo_enabled = false;          */
                    pwm_off();
                }
            }
        }    
        /* Check if it is time to exit screen saver 
        * (Done in this thread to avoid concurrent display update from events thread) */
        if (screen_saver_state.stop_asked){       
            if (config_get_diapo_activation()){
                diapo_stop();
                draw_img(skin_get_background());
                display_current_filename(current_filename);       
                display_bat_state(true);
                eng_select_ctrl(NULL, true);
            } else {
                pwm_resume();
            }           
            screen_saver_state.next_time = tp.tv_sec + config_get_screen_saver_to();
            PRINTDF("screen saver next time : %d\n", screen_saver_state.next_time);
            screen_saver_state.is_running = false;
            screen_saver_state.stop_asked = false;       
        }
    }
    return 0;
}


static void display_bat_state(bool force){
    static enum E_POWER_LEVEL previous_state = POWER_BAT_UNKNOWN;
    enum E_POWER_LEVEL state;
    struct skin_rectangular_shape  zone;
    const struct skin_control * bat = skin_get_ctrl(SKIN_CMD_BATTERY_STATUS);

    PRINTDF("Display bat \n");
    if (bat != NULL){
        state = power_get_bat_state();
        if ((state != previous_state) ||
            (force == true )){
            zone = skin_ctrl_get_zone(bat);
            PRINTDF("New battery state : %i \n", state);
            draw_cursor(skin_get_img(SKIN_CMD_BATTERY_STATUS), state,zone.x1, zone.y1);
            previous_state = state;
        }
    }
}


static int track_get_string(char * buffer, size_t len, int time, bool is_hour){        
    int curr_hour, curr_min, curr_sec;
    int ret;
    char sign[2] = {0,0};
    
    curr_sec = abs(time);    
    curr_hour = curr_sec / 3600;
    curr_min = curr_sec / 60;
    curr_sec = curr_sec % 60;
    if (time < 0)
        sign[0] = '-';
    if (is_hour){        
        ret = snprintf(buffer, len, "%s%02i:%02i:%02i", sign, curr_hour, curr_min, curr_sec);
    } else {
        ret = snprintf(buffer, len, "%s%02i:%02i", sign, curr_min, curr_sec);
    }
    buffer[len - 1] = 0;
    return ret;    
}

static void display_progress_bar(int current, int length)
{
    /* Previous coord of progress bar cursor */
    static struct{
        int x,y;
        int pos, length; 
    } pb_prev_val = {-1, -1, 0, 0};
    
    int i, x, y, height, width;
    unsigned char * buffer;
    int buffer_size;    
    int percent_pos;
    struct font_color color;
    char displayed_text[16];
    int text_height, text_width, orig; 
    struct skin_rectangular_shape pb_zone;
    const struct skin_control *pb = skin_get_pb();
    
    if (pb == NULL){
        /* No progress bar on skin nothing to do */
        return;
    }
    
    if ((pb_prev_val.pos == current) && 
        (pb_prev_val.length == length)){
        /* current position in seconds since last call have not been modified
         * It is useless to redraw...*/
        return;
    }
    pb_prev_val.pos = current;
    pb_prev_val.length = length;
    pb_zone = skin_ctrl_get_zone(pb);
    
    percent_pos = (current * 100) / length;
    /* Display current time in file at the beginig of progress bar */
    
    height = pb_zone.y2 - pb_zone.y1;
    color.r = 255;
    color.g = 255;
    color.b = 255; 
    if ((height - 4) > 0){
        /* Display current track time */
        track_get_string(displayed_text, sizeof(displayed_text), current, !(length < 3600));
        font_change_size(height-4);        
        font_get_size(displayed_text, &text_width, &text_height, &orig);
        font_restore_default_size();  
        draw_text(displayed_text, pb_zone.x1, pb_zone.y1 - text_height,
                          text_width, text_height, &color, height-4);
        /* Display remaining track time */                      
        current = current - length;
        track_get_string(displayed_text, sizeof(displayed_text), current, !(length < 3600));
        font_change_size(height-4);        
        font_get_size(displayed_text, &text_width, &text_height, &orig);
        font_restore_default_size();  
        draw_text(displayed_text,  pb_zone.x2 - text_width, pb_zone.y1 - text_height,
                          text_width, text_height, &color, height-4);        
    }
                       
    
    if (skin_get_pb_img() == 0) {
        /* No cursor bitmap : just fill progress bar */
        int step1;
        unsigned char col1r, col1g, col1b;
        
        width =  pb_zone.x2 - pb_zone.x1;
        if ((height== 0) || (width <= 0)){
            /*dont care if progress bar not visible */
            return;
        }
        buffer_size = height * width * 4;
        buffer = malloc( buffer_size );
        if (buffer == NULL){
            fprintf(stderr, "Allocation error\n");
            return;
        }    
        col1r = skin_get_config()->pb_r;
        col1g = skin_get_config()->pb_g;
        col1b = skin_get_config()->pb_b;
        step1 =  percent_pos * width / 100;

        i = 0;
        for( y = 0; y < height; y++ ){
            for( x = 0; x < width ; x++ ){
                if (x <= step1) {
                    buffer[i++] = (unsigned char )col1r;
                    buffer[i++] = (unsigned char )col1g;
                    buffer[i++] = (unsigned char )col1b;
                    buffer[i++] = 255;
                } else {
                    buffer[i++] = 0;
                    buffer[i++] = 0;
                    buffer[i++] = 0;
                    buffer[i++] = 0;
                }
            }
        }
        draw_RGB_buffer(buffer, pb_zone.x1, pb_zone.y1, width, height, true);
    } else {
        /* A cursor bitmap is available */
        int new_x;
        int erase_width;
        int erase_x;
                          
        /* Get cusor infos and compute new coordinate */
        ilBindImage(skin_get_pb_img());
        width  = ilGetInteger(IL_IMAGE_WIDTH);
        height = ilGetInteger(IL_IMAGE_HEIGHT);
        if (pb_prev_val.y == -1){
            PRINTDF("New progress bar coord : y1 : %i - y2 : %i - h : %i\n",
                    pb_zone.y1, pb_zone.y2, height);
            pb_prev_val.y = pb_zone.y1 + (pb_zone.y2 - pb_zone.y1) / 2  - height/2;                           
            if (pb_prev_val.y < 0){
                pb_prev_val.y = 0;
            }
            pb_prev_val.x = pb_zone.x1 - width / 2;
            if (pb_prev_val.x < 0){
                pb_prev_val.x = 0;
            }
        }
        new_x = pb_zone.x1 +
                percent_pos * (pb_zone.x2 - pb_zone.x1) / 100 - width / 2;

        /* Alloc buffer for RBGA conversion */
        buffer_size = width * height * 4;
        buffer = malloc( buffer_size );
        if (buffer == NULL){
            fprintf(stderr, "Allocation error\n");
            return;
        }

        /* Restore Background */
        ilBindImage(skin_get_background());
        erase_width = width;
        erase_x = pb_prev_val.x;        
        ilCopyPixels(erase_x, pb_prev_val.y, 0,
                     erase_width, height , 1,
                     IL_RGBA, IL_UNSIGNED_BYTE, buffer);
        /*PRINTDF("Progress bar - erase x : %i - prev y : %i - w : %i -h : %i - buffer : @%x \n",erase_x,prev_coords.y,erase_width,height,buffer);*/
        draw_RGB_buffer(buffer, erase_x, pb_prev_val.y, erase_width, height, true);

        /* Display cursor at its new position after overlaying cursor bitmap on skin background */
        draw_cursor(skin_get_pb_img(), 0, new_x, pb_prev_val.y);
        pb_prev_val.x = new_x;
    }
    free(buffer);
}

static void *anim_thread(void * param){

  int nb_frame,width,height;
  int current_num, x, y;
  unsigned char * buffer;
  ILuint img;
  struct skin_rectangular_shape zone;
  
  img = skin_get_img(SKIN_CMD_ANIM);

  
  if (( state.current_mode == MODE_AUDIO ) &&
      ( img != 0)){
    zone = skin_ctrl_get_zone(skin_get_ctrl(SKIN_CMD_ANIM));
    ilBindImage(img);
    nb_frame = ilGetInteger(IL_NUM_IMAGES);
    width  = ilGetInteger(IL_IMAGE_WIDTH);
    height = ilGetInteger(IL_IMAGE_HEIGHT);
    PRINTDF(" anim : frames : %i - WxH : %ix%i \n", nb_frame, width,height);
    x = zone.x1;
    y = zone.y1;
    current_num = 0;
    while (playint_is_running()){
			 /*PRINTD("New anim frame : %i \n", current_num);*/
			 pthread_mutex_lock(&display_mutex);
			 ilBindImage(img);
			 ilActiveImage(current_num);
			 ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	 		 buffer = ilGetData();
			 draw_RGB_buffer(buffer, x, y, width, height, false);
			 pthread_mutex_unlock(&display_mutex);
			 current_num++;
			 if (current_num >= nb_frame){
				 current_num=0;
			 }
			 usleep(100000);
		 }

    }
    return NULL;
}


/** Thread that updates peridocally OSD if needed
*
* \note it also flushes mplayer stdout
*/
static void * update_thread(void *val){
  int resume_pos = (int)val;
  int pos, file_length;  
  char buffer_filename[200];
  
  current_filename[0] = 0;
  while (playint_is_running()){
    if (playint_wait_output(UPDATE_PERIOD_MS) == 0){        
        playint_flush_stdout();
    }
    if (!playint_is_running()){
        break;
    }
    pthread_mutex_lock(&display_mutex);
    /* DO Not send periodic commands to mplayer while in pause because it unlocks the pause for a brief delay */ 
    if (playint_is_paused() == false){          
      if( playint_get_filename(buffer_filename, sizeof(buffer_filename)) > 0){                   
        if( strcmp(buffer_filename, current_filename ) ){
          /* Current filename has changed (new track)*/          
          settings_update();                      
          strcpy(current_filename, buffer_filename);                    
          file_length = playint_get_file_length();
          if (resume_pos != 0){
            playint_seek(resume_pos, PLAYINT_SEEK_ABS);
            resume_pos = 0;
          }
          if( state.current_mode == MODE_AUDIO ){
            // Display filename 
            if (!screen_saver_is_running()){
              display_current_filename(current_filename);
              display_bat_state(true);
            }            
          }
          if(state.current_mode == MODE_VIDEO){
            draw_img(skin_get_background());
            display_bat_state(true);            
          }
        }
      }
      
      if (!playint_is_running()){
        pthread_mutex_unlock(&display_mutex);
        break;
      }
    
      /* Update progress bar */
      if (((state.menu_showed == true) ||  
          ((state.current_mode == MODE_AUDIO) && (!screen_saver_is_running())))) {
        /* Update progress bar */
        pos = playint_get_file_position_seconds();
        if (file_length == 0) 
          file_length = playint_get_file_length();
        if ((pos >= 0) && (file_length > 0)) {
          display_progress_bar(pos, file_length);
        } else {
          fprintf(stderr, "error while trying to retrieve current position\n");
        }
        /* Display battery state */
        display_bat_state(false);
      }
    }
    pthread_mutex_unlock(&display_mutex);

    /* Handle screen saver */
    screen_saver_update();
   
    /* speakers config update */
    if (config_get_speaker() == CONF_INT_SPEAKER_AUTO) {
      if (!config_get_fm_activation()){
      /* No FM transmitter : Check for headphones presence to turn on/off internal speaker */
        snd_check_headphone();
      } else {
        /* FM transmitter : always mute */
        snd_mute_internal(true);
      }
    } else {
      if (config_get_speaker() == CONF_INT_SPEAKER_NO ){
        snd_mute_internal(true);
      } else { /* CONF_INT_SPEAKER_ALWAYS */
        snd_mute_internal(false);	
      }
    }
    /* FIXME test*/     
    /* Unmute external for eclipse AND CARMINAT and start test */  
    /* snd_mute_external(false);*/
    /* FIXME Test for tomtom START 
    snd_set_volume_db(30);*/
    
    /* Handle power button*/
    if (power_is_off_button_pushed() == true){
      quit();      
    }    
    
  } /* End main loop */

  /* Stop screen saver if active on exit */
  if (screen_saver_is_running()){   
    if (config_get_diapo_activation()){
      diapo_stop();   
    } else {    
      pwm_resume();
    }
  }
  pthread_exit(NULL);
}


static void *mplayer_thread(void *filename){
    playint_run((char *)filename);        
    printf("\nmplayer has exited\n"); 
    pthread_exit(NULL);
}


int eng_init(bool is_video){       
    /* Dont want to be killed by SIGPIPE */
    signal (SIGPIPE, SIG_IGN);
    
    /* Initialize DevIL. */
    ilInit();
    iluInit();
    /* Will prevent any loaded image from being flipped dependent on its format */
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
    
    /* Initialize font module */
    font_init(11); /* Default font size is hard coded */
        
    /* Initialize tomplayer status */            
    if (is_video)
      state.current_mode = MODE_VIDEO;
    else
      state.current_mode = MODE_AUDIO;          
    
    screen_saver_init();
    resume_file_init(state.current_mode);
    if (state.current_mode == MODE_VIDEO){
        skin_init(config_get_skin_filename(CONFIG_VIDEO), true);        
    } else {    
        skin_init(config_get_skin_filename(CONFIG_AUDIO), true);        
        draw_img(skin_get_background());
        draw_track_text( "Loading...");
    }
    settings_init();
    
    /* Turn ON screen if it is not */
    pwm_resume();
    return true;
}

int eng_release(void){
    ilShutDown();
    font_release();    
    skin_release();
    return true;
}

void eng_play(char * filename, int pos){
    pthread_t up_tid;    
    pthread_t player_tid;            
    pthread_t evt_tid;
    pthread_t anim_tid;
    int resume_pos;
    
    if (pos > 5){
      resume_pos = pos - 5;
    } else {
      resume_pos = 0;
    }    
    
    /* Init interface with mplayer */
    playint_init();
    
    /* Launch all threads */
    pthread_create(&up_tid, NULL, update_thread, (void *)resume_pos);
    pthread_create(&anim_tid, NULL, anim_thread, NULL);
    pthread_create(&player_tid, NULL, mplayer_thread, filename);     
    pthread_create(&evt_tid, NULL, event_loop, NULL);
    /* Wait for the end of mplayer thread */
    pthread_join(player_tid, NULL);    
    
    /* Save settings to resume file */
    if (state.current_mode == MODE_VIDEO){
        resume_set_video_settings(&settings.video);
    } else {
        resume_set_audio_settings(&settings.audio);
    }
    /* Save audio playlist if quit has been required (as opposed to an end of playlist exit)*/
    if (state.quit_asked)
        resume_save_playslist(state.current_mode, current_filename);
    
    /* Wait for everyone termination */
    pthread_join(up_tid, NULL);
    pthread_join(anim_tid, NULL);    
    pthread_join(evt_tid, NULL);    
}


/** This function requires tomplayer menu to be displayed 
 * \retval  0 The menu was already displayed
 * \retval  1 The screen saver was active
 * \retval  2 The video menu was hidden
 *
 * \note This function has to called on each input event 
 */
int eng_ask_menu(void){
    if (screen_saver_reset_to() > 0){
        /* Screen saver was active */
        return 1;
    }
    if( state.menu_showed == false && state.current_mode == MODE_VIDEO){
        state.menu_showed = true;
        playint_menu_show();   
        return 2;
    }
    return 0;
}

void eng_handle_cmd(int cmd, int p)
{
    struct video_settings v_settings;
    struct audio_settings a_settings;
    static bool update_settings = false;

    switch(cmd){
        case SKIN_CMD_PAUSE:
            playint_pause();
            break;
        case SKIN_CMD_STOP:
            quit();            
            break;
        case SKIN_CMD_MUTE:
            playint_mute();           
            break;
        case SKIN_CMD_VOL_PLUS:
            if (p == -1) 
                playint_vol(1, PLAYINT_VOL_REL);
            else
                playint_vol(p, PLAYINT_VOL_ABS);            
            update_settings = true;
        break;
        case SKIN_CMD_VOL_MOINS:
            if (p == -1) 
                playint_vol(-1, PLAYINT_VOL_REL);
            else
                playint_vol(p, PLAYINT_VOL_ABS);                            
            update_settings = true;
            break;
        case SKIN_CMD_LIGHT_PLUS:            
            playint_bright(5);
            update_settings = true;
            break;
        case SKIN_CMD_LIGHT_MOINS:
            playint_bright(-5);
            update_settings = true;
            break;
        case SKIN_CMD_DELAY_PLUS:
            playint_delay(0.1);            
            update_settings = true;
            break;
        case SKIN_CMD_DELAY_MOINS:
            playint_delay(-0.1);            
            update_settings = true;
            break;
        case SKIN_CMD_GAMMA_PLUS:
            playint_contrast(5);
            update_settings = true;
            break;
        case SKIN_CMD_GAMMA_MOINS:
            playint_contrast(-5);
            update_settings = true;
            break;
        case SKIN_CMD_FORWARD:
            if (p == -1)
                playint_seek(10, PLAYINT_SEEK_REL);
            else
                playint_seek(p, PLAYINT_SEEK_PERCENT);                            
            break;
        case SKIN_CMD_BACKWARD:
            if (p == -1) 
                playint_seek(-10, PLAYINT_SEEK_REL);
            else
                playint_seek(p, PLAYINT_SEEK_PERCENT);            
            break;
        case SKIN_CMD_PREVIOUS:
            playint_skip(-1);
            break;
        case SKIN_CMD_NEXT:
            playint_skip(1);
            break;
        case SKIN_CMD_BATTERY_STATUS:
        	break;
        case SKIN_CMD_EXIT_MENU:
        default:
            if (state.current_mode == MODE_VIDEO){
                state.menu_showed = false;
                playint_menu_hide();
            }
    }

    /* Update in-memory settings */
    if (update_settings == true) {
        if (!playint_is_paused()){
            /* We get no answer from mplayer while paused */            
            if (state.current_mode == MODE_VIDEO){
                if (playint_get_video_settings(&v_settings) == 0){
                    settings.video = v_settings;
                }
            } else {
                if (playint_get_audio_settings( &a_settings) == 0){
                    settings.audio = a_settings;
                }
            }
            update_settings = false ;
        }
    }
}

int eng_select_ctrl(const struct skin_control * ctrl, bool state){
    static const struct skin_control *selected_ctrl;
    int x,y,w,h/*,xc,yc*/;
    struct skin_rectangular_shape zone;
    unsigned char * select_square;
    
    if ((ctrl != NULL) && (state)){
        selected_ctrl = ctrl;   
        playint_osd(skin_cmd_2_txt(ctrl->cmd), 1500);        
    } else {
        if (!state){
            selected_ctrl = NULL; 
        } else {            
            if (selected_ctrl == NULL) 
                return -1;
            ctrl = selected_ctrl;
        }
    }
    
    zone = skin_ctrl_get_zone(ctrl);
    x = zone.x1;
    y = zone.y1;
    w = zone.x2 - zone.x1 ;
    h = zone.y2 - zone.y1 ;    
    select_square = malloc(3*w*h);        
    if (select_square == NULL){
        return -1;
    }          
    /*xc = 240-w/2;
    yc = 272/2-h/2;*/
    /* copy the relevant skin background zone */
    ilBindImage(skin_get_background());
    /*if (state)*/
    ilCopyPixels(x, y, 0, w, h, 1,
                 IL_RGB, IL_UNSIGNED_BYTE, select_square);
/*    else
    ilCopyPixels(xc, yc, 0, w, h, 1,
                 IL_RGB, IL_UNSIGNED_BYTE, select_square);*/
    if (state){
        #if 0
      if (ctrl->type == CIRCULAR_SKIN_CONTROL){
        int x, y, err ;                
        int xm, ym, r;
        r = (w / 2) - 2;
        xm = r;
        ym = r;
        x = -r;
        y = 0;
        err = 2-2*r;
        do {
          select_square[(xm-x+(ym+y)*w)*3]   = 255;
          select_square[(xm-x+(ym+y)*w)*3+1] = 0;
          select_square[(xm-x+(ym+y)*w)*3+2] = 0;  
          select_square[(xm-y+(ym-x)*w)*3]   = 255;
          select_square[(xm-y+(ym-x)*w)*3+1] = 0;
          select_square[(xm-y+(ym-x)*w)*3+2] = 0;  
          select_square[(xm+x+(ym-y)*w)*3]   = 255;
          select_square[(xm+x+(ym-y)*w)*3+1] = 0;
          select_square[(xm+x+(ym-y)*w)*3+2] = 0;  
          select_square[(xm+y+(ym+x)*w)*3]   = 255;
          select_square[(xm+y+(ym+x)*w)*3+1] = 0;
          select_square[(xm+y+(ym+x)*w)*3+2] = 0;  
          r = err;
          if (r >  x) err += ++x*2+1; /* e_xy+e_x > 0 */
          if (r <= y) err += ++y*2+1; /* e_xy+e_y < 0 */
        } while (x < 0);    
      } else {
          #endif
        int i,j;    
        /* Draw a square around the control */        
        for(j=0; j<h; j++){
            for (i=0; i<w; i++){
            if ((i==1) || (i==(w-1)) || (j==1) || (j==(h-1))){
                select_square[(i+j*w)*3]   = 255;
                select_square[(i+j*w)*3+1] = 0;
                select_square[(i+j*w)*3+2] = 0;
            }
            }
        }
      
      //}
    }     
    draw_RGB_buffer(select_square, x, y , w, h, false);
    free(select_square);
    return 0;
}

enum eng_mode eng_get_mode(void){
    return state.current_mode;
}
