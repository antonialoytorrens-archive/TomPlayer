/**
 * \file engine.c 
 * \brief This module implements the core engine during playing a media
 *
 * This module implements the core engine during playing a media. 
 * \li It Implements the logic behind the actions required by the input events
 * \li It handles the screen saver triggering  
 * \li It takes cares of restoring and saving the current settings
 * \li It Drives mplayer through the slave interface available in play_int.h 
 * \li It triggers the update of skins through the interface skin_display.h 
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
#include <signal.h>

#include "gps.h"
#include "config.h"
#include "resume.h"
#include "widescreen.h"
#include "pwm.h"
#include "sound.h"
#include "debug.h"
#include "skin.h"
#include "power.h"
#include "font.h"
#include "diapo.h"
#include "play_int.h"
#include "event_inputs.h"
#include "draw.h"
#include "track.h"
#include "skin_display.h"
#include "fm.h"
#include "engine.h"

/* Update period in ms */
#define UPDATE_PERIOD_MS 250

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

/* Mutex to prevent animation thread to interact badly with standard update thread */
static pthread_mutex_t display_mutex = PTHREAD_MUTEX_INITIALIZER;



static void alarm_handler(int sig) { 
 return;
}

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
                skin_display_refresh(SKIN_DISPLAY_NEW_TRACK);
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
  char buffer_filename[200];
  
  while (playint_is_running()){
    if (playint_wait_output(UPDATE_PERIOD_MS) == 0){        
        playint_flush_stdout();
    }
    /* Quick path to exit the loop if mplayer is over */
    if (!playint_is_running()){
        break;
    }
    
    pthread_mutex_lock(&display_mutex);
    /* DO Not send periodic commands to mplayer while in pause because it unlocks the pause for a brief delay 
     * Anyway it does not make sense to test for a new track while paused... 
     */ 
    if (playint_is_paused() == false){
      if (playint_get_path(buffer_filename, sizeof(buffer_filename)) > 0){                   
        if (track_has_changed(buffer_filename)){
          /* Current filename has changed (new track)*/          
          settings_update();
          if (resume_pos != 0){
            playint_seek(resume_pos, PLAYINT_SEEK_ABS);
            resume_pos = 0;
          }
          /* Load new tags and update internal filename */
          track_update(buffer_filename);
          if (!screen_saver_is_running()){
            skin_display_refresh(SKIN_DISPLAY_NEW_TRACK);
          }
        }
      }
    }
    
    /* Quick path to exit the loop if mplayer is over */
    if (!playint_is_running()){
        pthread_mutex_unlock(&display_mutex);
        break;
    }
    
    /* Update info from GPS */
    gps_update() ;
    
    /* Periodic update of the skin controls if they are visible */
    if (((state.menu_showed == true) ||  
        ((state.current_mode == MODE_AUDIO) && (!screen_saver_is_running())))) {
        skin_display_refresh(SKIN_DISPLAY_PERIODIC);
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


static int init(const char * mode){    
    struct sigaction new_action;
    bool is_video;      
    
    /* Dont want to be killed by SIGPIPE */
    signal (SIGPIPE, SIG_IGN);    
    /*Handler on alarm signal */
    new_action.sa_handler=alarm_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags=0;
    sigaction(SIGALRM, &new_action, NULL);

    /* Read generic configuration  */
    if (config_init() == false){
        fprintf( stderr, "Error while loading config\n" );
        return -1;
    }
    
    /* Test current mode */
    is_video = false;
    if (mode != NULL){
    if (strncmp(mode, "VIDEO", 5) == 0)
      is_video = true;
    }
    
    /* Initialize GPS module */
    gps_init();
    
    /* Initialize DevIL. */
    ilInit();
    iluInit();
    /* Will prevent any loaded image from being flipped dependent on its format */
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
    
    /* Initialize font module */
    font_init(11); /* Default font size is hard coded */
        
    /* Initialize tomplayer status and skin module */            
    if (is_video){
      state.current_mode = MODE_VIDEO;
      skin_init(config_get_skin_filename(CONFIG_VIDEO), true);        
    } else {    
      state.current_mode = MODE_AUDIO;          
      skin_init(config_get_skin_filename(CONFIG_AUDIO), true);        
    }
    
    /* Initialize Screen saver */
    screen_saver_init();
    
    /* Initialize resume function */
    resume_file_init(state.current_mode);
    
    /* Initialize settings module*/
    settings_init();    
          
    /* Init interface with mplayer */
    playint_init();
  
    /* Initialize diaporama */
    if (config_get_diapo_activation()){
        if (diapo_init(config_get_diapo()) == false){
            fprintf(stderr,"Diaporama initialization failed ...\n");      
            config_toggle_enable_diapo();
        }
    }
    
    /* Turn ON screen if it is not */
    pwm_resume();
    
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
 
    return 0;
}

static void release(void){         
    /* Desactivate FM transmitter if needed */
    if (config_get_fm_activation()){
        fm_set_state(0);
        snd_mute_internal(false);
    }
  
    /* Free resources */    
    track_release();
    diapo_release();
    config_free();
    ilShutDown();
    font_release();    
    skin_release();
    return;
}

static void play(char * filename, int pos){
    pthread_t up_tid;    
    pthread_t player_tid;                
    pthread_t anim_tid;
    int resume_pos;
    
    if (pos > 5){
      resume_pos = pos - 5;
    } else {
      resume_pos = 0;
    }    
    
    /* Launch all threads */
    pthread_create(&up_tid, NULL, update_thread, (void *)resume_pos);
    pthread_create(&anim_tid, NULL, anim_thread, NULL);
    pthread_create(&player_tid, NULL, mplayer_thread, filename);             
    
    /* Handle input events */
    event_loop();
    
    /* Save settings to resume file */
    if (state.current_mode == MODE_VIDEO){
        resume_set_video_settings(&settings.video);
    } else {
        resume_set_audio_settings(&settings.audio);
    }
    /* Save audio playlist if quit has been required 
       (as opposed to an end of playlist exit) */
    if (state.quit_asked)
        resume_save_playslist(state.current_mode, track_get_current_filename());
    
    /* Wait for everyone termination */
    pthread_join(up_tid, NULL);
    pthread_join(anim_tid, NULL);        
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
    int x,y,w,h;
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

    /* copy the relevant skin background zone */
    ilBindImage(skin_get_background());
    /*if (state)*/
    ilCopyPixels(x, y, 0, w, h, 1,
                 IL_RGB, IL_UNSIGNED_BYTE, select_square);
    if (state){
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
    }     
    draw_RGB_buffer(select_square, x, y , w, h, false);
    free(select_square);
    return 0;
}

enum eng_mode eng_get_mode(void){
    return state.current_mode;
}

int main( int argc, char *argv[] ){ 
  
  if (argc != 4){
    return -1;
  }
  
  if (init(argv[3]) == 0){
    play(argv[1], atoi(argv[2]));     
  }
  
  release();
  
  return 0;
}
