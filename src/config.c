/**
 * \file config.c
 * \author nullpointer
 * \brief This module implements configuration reading
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dictionary.h>
#include <iniparser.h>

#include "debug.h"
#include "config.h"
#include "zip_skin.h"
#include "engine.h"

#include "widescreen.h"


/* Definition of section in the config file */
#define SECTION_GENERAL             "general"
#define SECTION_VIDEO_SKIN          "video_skin"
#define SECTION_AUDIO_SKIN          "audio_skin"
#define SECTION_CONTROL_FMT_STR     "CONTROL_%d:%s"


#define KEY_SKIN_FILENAME           "filename"


#define KEY_FILTER_VIDEO_EXT        "filter_video"
#define KEY_FILTER_AUDIO_EXT        "filter_audio"
#define KEY_VIDEO_FILE_DIRECTORY    "video_dir"
#define KEY_AUDIO_FILE_DIRECTORY    "audio_dir"

#define KEY_SKIN_BMP                "image"
#define KEY_SKIN_CONF               "conf"

#define KEY_TEXT_X1                 "text_x1"
#define KEY_TEXT_Y1                 "text_y1"
#define KEY_TEXT_X2                 "text_x2"
#define KEY_TEXT_Y2                 "text_y2"

#define KEY_R_TRANSPARENCY          "r"
#define KEY_G_TRANSPARENCY          "g"
#define KEY_B_TRANSPARENCY          "b"


#define KEY_R_PROGRESSBAR           "pb_r"
#define KEY_G_PROGRESSBAR	    	"pb_g"
#define KEY_B_PROGRESSBAR           "pb_b"

#define KEY_SCREEN_SAVER_TO			"screen_saver_to"
#define KEY_EN_SCREEN_SAVER         "enable_screen_saver"
#define KEY_FM_TRANSMITTER          "fm_transmitter"
#define KEY_FM_TRANSMITTER1          "fm_transmitter1"
#define KEY_FM_TRANSMITTER2          "fm_transmitter2"
#define KEY_EN_FM_TRANSMITTER       "enable_fm_transmitter"
#define KEY_TYPE_CONTROL            "type"
#define KEY_CMD_CONTROL             "ctrl"
#define KEY_CMD_CONTROL2            "cmd"
#define KEY_EN_SMALL_TEXT           "enable_small_text"

#define KEY_CIRCULAR_CONTROL_X      "x"
#define KEY_CIRCULAR_CONTROL_Y      "y"
#define KEY_CIRCULAR_CONTROL_R      "r"

#define KEY_RECTANGULAR_CONTROL_X1  "x1"
#define KEY_RECTANGULAR_CONTROL_X2  "x2"
#define KEY_RECTANGULAR_CONTROL_Y1  "y1"
#define KEY_RECTANGULAR_CONTROL_Y2  "y2"

#define KEY_TEXT_COLOR "text_color"

#define KEY_CTRL_BITMAP_FILENAME "bitmap"

#define KEY_DIAPO_ENABLED   "diapo_enabled"
#define KEY_DIAPO_FILTER    "diapo_filter"
#define KEY_DIAPO_PATH      "diapo_path"
#define KEY_DIAPO_DELAY     "diapo_delay"

#define KEY_INTERNAL_SPEAKER "int_speaker"
#define KEY_VIDEO_PREVIEW "video_preview"

/**
 * \def SCREEN_SAVER_TO_S
 * \brief Timeout in seconds before turning OFF screen while playing audio
 */
#define SCREEN_SAVER_TO_S 6

/**
 * \def DEFAULT_FOLDER
 * \brief default folder is the specified one doesn't exist
 */
#define DEFAULT_FOLDER "/mnt"

struct tomplayer_config config;

/* Correct iniparser bug in naming function iniparser_setstring/iniparser_set */
extern int iniparser_set(dictionary * ini, char * entry, const char * val);
#define iniparser_setstring iniparser_set


struct rectangular_skin_control control_get_zone(const struct skin_control *ctrl){
    struct rectangular_skin_control zone;
    switch(ctrl->type){
        case CIRCULAR_SKIN_CONTROL :
            zone.x1 = ctrl->area.circular.x - ctrl->area.circular.r ;
            zone.y1 = ctrl->area.circular.y - ctrl->area.circular.r ;
            zone.x2 = ctrl->area.circular.x + ctrl->area.circular.r ;
            zone.y2 = ctrl->area.circular.y + ctrl->area.circular.r ;
            break;
        default :
            zone = ctrl->area.rectangular;
            break;
    }
    return zone;
}

static int control_compare(const struct skin_control *c1, const struct skin_control * c2){
    struct rectangular_skin_control zone1, zone2;
    
    zone1 = control_get_zone(c1);
    zone2 = control_get_zone(c2);    
    if ((zone1.y1  <= zone2.y2) &&
        (zone2.y1  <= zone1.y2)){
        /* If the two controls intersect horizontally the lefter is the lesser*/        
        return (zone1.x1 - zone2.x1);
    } else {
        /* Otherwise the upper the lesser */
        return (zone1.y1 - zone2.y1);        
    }
}

static void control_sort(struct skin_control *array, int length)  { 
    int i, j;  
    struct skin_control temp;
    int test; 

    for(i = length - 1; i > 0; i--){  
        test=0;  
        for(j = 0; j < i; j++){            
            if (control_compare(&array[j], &array[j+1]) > 0){
                temp = array[j];   
                array[j] = array[j+1];  
                array[j+1] = temp;  
                test=1;  
            }  
        }
        if(test==0) break; 
    }
} 

/** Show configuration parameters
 *
 * \param conf main configuration structure
 */
static void display_config( struct tomplayer_config * conf ){
    PRINTDF( "   - filter_video_ext : %s\n", conf->filter_video_ext );
    PRINTDF( "   - filter_audio_ext : %s\n", conf->filter_audio_ext );
    PRINTDF( "   - video_folder : %s\n", conf->video_folder );
    PRINTDF( "   - audio_folder : %s\n", conf->audio_folder );
    PRINTDF( "   - video_skin_filename : %s\n", conf->video_skin_filename );
    PRINTDF( "   - audio_skin_filename : %s\n", conf->audio_skin_filename );
    PRINTDF( "   - screen_saver_to : %d\n", conf->screen_saver_to );
}

/** Reset all value of the skin configuration structure
 *
 * \param conf skin configuration
 */
void reset_skin_conf (struct skin_config * conf){
	int i;
	/* FIXME Rajouter lib�ration dynamique */
	memset(conf, 0, sizeof(*conf));
	for( i = 0; i < SKIN_CMD_MAX_NB; i++ ){
	  conf->cmd2idx[i] = -1;
	}
}

/** Load a skin configuration
 *
 * \param filename fullpath to the skin configuration file
 * \param conf skin configuration structure
 *
 * \return true on succes, false on failure
 */
bool load_skin_config( char * filename, struct skin_config * skin_conf){
    dictionary * ini ;
    char section_control[PATH_MAX + 1];
    int i,j;
    char * s;
    bool ret = true;

    reset_skin_conf(skin_conf);
    skin_conf->progress_bar_index = -1;
    ini = iniparser_load(filename);
    if (ini == NULL) {
        PRINTDF( "Unable to load config file %s\n", filename);
        return false ;
    }
    i = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_COLOR, 0xFF0000);
    if( i < 0  ){
        PRINTD("No text color\n");
    } else{
        skin_conf->text_color = i;
        PRINTDF("Read txt color : 0x%x  \n",skin_conf->text_color);
    }
    skin_conf->text_x1 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_X1, 0);
    skin_conf->text_x2 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_X2, 0);
    skin_conf->text_y1 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_Y1, 0);
    skin_conf->text_y2 = iniparser_getint(ini, SECTION_GENERAL":"KEY_TEXT_Y2, 0);
    skin_conf->r = iniparser_getint(ini, SECTION_GENERAL":"KEY_R_TRANSPARENCY, 0);
    skin_conf->g = iniparser_getint(ini, SECTION_GENERAL":"KEY_G_TRANSPARENCY, 0);
    skin_conf->b = iniparser_getint(ini, SECTION_GENERAL":"KEY_B_TRANSPARENCY, 0);
    skin_conf->pb_r = iniparser_getint(ini, SECTION_GENERAL":"KEY_R_PROGRESSBAR, 0);
    skin_conf->pb_g = iniparser_getint(ini, SECTION_GENERAL":"KEY_G_PROGRESSBAR, 0);
    skin_conf->pb_b = iniparser_getint(ini, SECTION_GENERAL":"KEY_B_PROGRESSBAR, 0);
    s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_SKIN_BMP, NULL);
    if( s != NULL ) strcpy( skin_conf->bitmap_filename, s ); // FIXME replace by a strdup
    
    /* Parse controls description */
    for( i = 0; i < MAX_SKIN_CONTROLS; i++ ){
        sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TYPE_CONTROL );
        j = iniparser_getint(ini, section_control, -1);
        if( j < 0  ){
           PRINTDF( "Warning : no section  <%s>\n", section_control );
           break;
        } else {
            skin_conf->controls[i].type = j;
        }
        sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CTRL_BITMAP_FILENAME );
        s = iniparser_getstring(ini, section_control, NULL);
        if( s != NULL ) strcpy( skin_conf->controls[i].bitmap_filename, s );
        sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL );
        skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
        if( skin_conf->controls[i].cmd < 0  ){
            sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL2 );
            skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
        }    
        switch( skin_conf->controls[i].type ){
            case CIRCULAR_SKIN_CONTROL:
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_X );
                skin_conf->controls[i].area.circular.x= iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_Y );
                skin_conf->controls[i].area.circular.y = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CIRCULAR_CONTROL_R );
                skin_conf->controls[i].area.circular.r = iniparser_getint(ini, section_control, -1);
                break;
            case RECTANGULAR_SKIN_CONTROL:
            case PROGRESS_SKIN_CONTROL_X:
            case PROGRESS_SKIN_CONTROL_Y:
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_X1 );
                skin_conf->controls[i].area.rectangular.x1= iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_X2 );
                skin_conf->controls[i].area.rectangular.x2 = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_Y1 );
                skin_conf->controls[i].area.rectangular.y1 = iniparser_getint(ini, section_control, -1);
                sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_RECTANGULAR_CONTROL_Y2 );
                skin_conf->controls[i].area.rectangular.y2 = iniparser_getint(ini, section_control, -1);
                break;
            default:
                fprintf( stderr, "Type not defined correctly for %s\n", section_control );
                ret = false;
                goto error;
        }
    }
    /* Number of controls on the skin */
    skin_conf->nb = i;
    /* Sort the controls */
    control_sort(skin_conf->controls, skin_conf->nb);        
    /* Fill in the indexes fields */
    for (i = 0; i < skin_conf->nb; i++){
        if (skin_conf->controls[i].cmd == SKIN_CMD_BATTERY_STATUS){
            PRINTDF("Battery conf index :%i - filename : %s\n",i,skin_conf->controls[i].bitmap_filename);
            skin_conf->bat_index = i;
        }
        if ((skin_conf->controls[i].type == PROGRESS_SKIN_CONTROL_X) ||
            (skin_conf->controls[i].type == PROGRESS_SKIN_CONTROL_Y)){
            skin_conf->progress_bar_index = i;
        }
        /* Fill table cmd -> skin index */
        if ((skin_conf->controls[i].cmd >= 0) &&
            (skin_conf->controls[i].cmd < SKIN_CMD_MAX_NB)){
            skin_conf->cmd2idx[skin_conf->controls[i].cmd] = i;
        }        
    }
    
error:
    iniparser_freedict(ini);
    return ret;
}

/** Load the main configuration and skin configuration
 *
 * \param conf main configuration structure
 *
 * \return true on succes, false on failure
 */
bool load_config( struct tomplayer_config * conf ){
	dictionary * ini ;
    struct stat folder_stats;
	char *s;

	PRINTD( "Loading main configuration\n");

    memset( conf, 0, sizeof( struct tomplayer_config ) );

    ini = iniparser_load(CONFIG_FILE);
	if (ini == NULL) {
		PRINTDF( "Unable to load main configuration file <%s>\n", CONFIG_FILE );
		return false ;
	}
	
	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_VIDEO_FILE_DIRECTORY, NULL);
	if( s != NULL ) strcpy( conf->video_folder, s );
    if (stat(conf->video_folder, &folder_stats) != 0){
    	strcpy(conf->video_folder,DEFAULT_FOLDER);
    }

	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_AUDIO_FILE_DIRECTORY, NULL);
	if( s != NULL ) strcpy( conf->audio_folder, s );
    if (stat(conf->audio_folder, &folder_stats) != 0){
    	strcpy(conf->audio_folder,DEFAULT_FOLDER);
    }


	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_FILTER_VIDEO_EXT, NULL);
	if( s != NULL ) strcpy( conf->filter_video_ext, s );
	s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_FILTER_AUDIO_EXT, NULL);
	if( s != NULL ) strcpy( conf->filter_audio_ext, s );

	conf->screen_saver_to = iniparser_getint(ini, SECTION_GENERAL":"KEY_SCREEN_SAVER_TO, -1);
    if (conf->screen_saver_to < 0 ){
    	conf->screen_saver_to = SCREEN_SAVER_TO_S;
    }
    conf->enable_screen_saver = iniparser_getint(ini,SECTION_GENERAL":"KEY_EN_SCREEN_SAVER,-1);
    if (conf->enable_screen_saver  == -1){
      conf->enable_screen_saver  = (conf->screen_saver_to? 1:0);
    }

    conf->fm_transmitter = iniparser_getint(ini, SECTION_GENERAL":"KEY_FM_TRANSMITTER, 87000000);
    conf->fm_transmitter1 = iniparser_getint(ini, SECTION_GENERAL":"KEY_FM_TRANSMITTER1, 87000000);
    conf->fm_transmitter2 = iniparser_getint(ini, SECTION_GENERAL":"KEY_FM_TRANSMITTER2, 87000000);
	conf->int_speaker = iniparser_getint(ini, SECTION_GENERAL":"KEY_INTERNAL_SPEAKER, 0);
	conf->video_preview = iniparser_getint(ini, SECTION_GENERAL":"KEY_VIDEO_PREVIEW, 1);
		
    conf->enable_fm_transmitter = iniparser_getint(ini, SECTION_GENERAL":"KEY_EN_FM_TRANSMITTER,0);

    conf->diapo_enabled = iniparser_getint(ini, SECTION_GENERAL":"KEY_DIAPO_ENABLED, 0);
    if (conf->diapo_enabled){
      s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_DIAPO_FILTER, NULL);
      if( s != NULL ) {
        conf->diapo.filter = strdup(s);
      } else {
        conf->diapo.filter = strdup("^.*\\.(jpeg|png|gif)$");
      }
      s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_DIAPO_PATH, NULL);
      if( s != NULL ) {
        conf->diapo.file_path = strdup(s);
      } else {
        conf->diapo.file_path = strdup("/mnt/sdcard/photos");
      }
      conf->diapo.delay = iniparser_getint(ini, SECTION_GENERAL":"KEY_DIAPO_DELAY, 5);
    }

    s = iniparser_getstring(ini, SECTION_VIDEO_SKIN":"KEY_SKIN_FILENAME, NULL);
	if( s != NULL ) strcpy( conf->video_skin_filename, s );

	s = iniparser_getstring(ini, SECTION_AUDIO_SKIN":"KEY_SKIN_FILENAME, NULL);
	if( s != NULL ) strcpy(conf->audio_skin_filename, s );
    
    conf->enable_small_text = iniparser_getint(ini, SECTION_GENERAL":"KEY_EN_SMALL_TEXT, 0);   
    iniparser_freedict(ini);

	display_config( conf );    

    return true;
}

bool config_set_skin(enum config_type type, const char * filename){
  switch (type){
    case CONFIG_AUDIO:
      strncpy(config.audio_skin_filename,filename, sizeof(config.audio_skin_filename));
    break;
    case CONFIG_VIDEO:
      strncpy(config.video_skin_filename,filename, sizeof(config.audio_skin_filename));      
    break;
    default :
      return false;
  } 
  return true; 
}

bool config_set_default_folder(enum config_type type, const char * folder){
 switch (type){
    case CONFIG_AUDIO:
      strncpy(config.audio_folder ,folder, sizeof(config.audio_folder));
    break;
    case CONFIG_VIDEO:
      strncpy(config.video_folder ,folder, sizeof(config.video_folder));      
    break;
    default :
      return false;
  } 
  return true; 
}

bool config_set_screensaver_to(int delay){
  if (delay < 0){
    return false;
  }
  config.screen_saver_to = delay;
  return true;
}

bool config_toggle_screen_saver_state(void){
  if (config.enable_screen_saver){
    config.enable_screen_saver = 0;
  } else {
    config.enable_screen_saver = 1;
  }
  return true;
}


bool config_set_fm_frequency(int freq){
  if ((freq  < 87000000) || (freq > 108000000)){
    return false;
  } else {
    config.fm_transmitter = freq;
  }
  return true;
}

bool config_set_fm_frequency1(int freq){
  if ((freq  < 87000000) || (freq > 108000000)){
    return false;
  } else {
    config.fm_transmitter1 = freq;
  }
  return true;
}

bool config_set_fm_frequency2(int freq){
  if ((freq  < 87000000) || (freq > 108000000)){
    return false;
  } else {
    config.fm_transmitter2 = freq;
  }
  return true;
}

bool config_set_int_speaker(int mode){
  if ((mode  < 0) || (mode >= CONF_INT_SPEAKER_MAX)){
    return false;
  } else {
    config.int_speaker = mode;
  }
  return true;
}


bool config_toggle_fm_transmitter_state(void){
  if (config.enable_fm_transmitter){
    config.enable_fm_transmitter = 0;
  } else {
    config.enable_fm_transmitter = 1;
  }
  return true;
}

bool config_toggle_small_text_state(void){
  if (config.enable_small_text){
    config.enable_small_text = 0;
  } else {
    config.enable_small_text = 1;
  }
  return true;
}


bool config_toggle_enable_diapo(void){
  if (config.diapo_enabled){
    config.diapo_enabled = false;
  } else {
    config.diapo_enabled = true;
  }
  return true;
}

bool config_set_diapo_folder(const char *folder){
   if (config.diapo.file_path){
      free(config.diapo.file_path);
      config.diapo.file_path = strdup(folder);
      if (config.diapo.file_path  == NULL){
        return false;
      }
   }
    return true;
}

bool config_set_diapo_delay(int delay){
  config.diapo.delay = delay;
  return true;  
}

bool config_save(void){
    dictionary * ini ;    
    char buffer[32];    
    FILE * fp;
    struct tomplayer_config * conf = &config;
    int ret = true;

    PRINTD( "Saving main configuration\n");   
    ini = iniparser_load(CONFIG_FILE);
    if (ini == NULL) {
        PRINTDF( "Unable to load main configuration file <%s>\n", CONFIG_FILE );
        return false ;
    }
    
    fp = fopen( CONFIG_FILE, "w+" );
    if( fp == NULL ){
            PRINTD( "Unable to open main config file\n" );
            ret = false;
            goto error;
    }

    iniparser_setstring( ini, SECTION_GENERAL":"KEY_VIDEO_FILE_DIRECTORY, conf->video_folder);
    iniparser_setstring( ini, SECTION_GENERAL":"KEY_AUDIO_FILE_DIRECTORY, conf->audio_folder);
    iniparser_setstring( ini, SECTION_GENERAL":"KEY_FILTER_VIDEO_EXT, conf->filter_video_ext);
    iniparser_setstring( ini, SECTION_GENERAL":"KEY_FILTER_AUDIO_EXT, conf->filter_audio_ext);
    snprintf(buffer, sizeof(buffer),"%i",conf->screen_saver_to);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_SCREEN_SAVER_TO, buffer);
    snprintf(buffer, sizeof(buffer),"%i",conf->enable_screen_saver);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_EN_SCREEN_SAVER, buffer);
    snprintf(buffer, sizeof(buffer),"%i",conf->fm_transmitter);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_FM_TRANSMITTER, buffer);
    snprintf(buffer, sizeof(buffer),"%i",conf->enable_fm_transmitter);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_EN_FM_TRANSMITTER, buffer);
	snprintf(buffer, sizeof(buffer),"%i",conf->fm_transmitter1);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_FM_TRANSMITTER1, buffer);
	snprintf(buffer, sizeof(buffer),"%i",conf->fm_transmitter2);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_FM_TRANSMITTER2, buffer);
	snprintf(buffer, sizeof(buffer),"%i",conf->int_speaker);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_INTERNAL_SPEAKER, buffer);
	snprintf(buffer, sizeof(buffer),"%i",conf->video_preview);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_VIDEO_PREVIEW, buffer);			
    snprintf(buffer, sizeof(buffer),"%i",conf->diapo_enabled);
    iniparser_setstring(ini,  SECTION_GENERAL":"KEY_DIAPO_ENABLED, buffer);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_DIAPO_FILTER, conf->diapo.filter);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_DIAPO_PATH, conf->diapo.file_path);
    snprintf(buffer, sizeof(buffer),"%i",conf->diapo.delay);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_DIAPO_DELAY, buffer);
    iniparser_setstring(ini, SECTION_VIDEO_SKIN":"KEY_SKIN_FILENAME, conf->video_skin_filename);
    iniparser_setstring(ini, SECTION_AUDIO_SKIN":"KEY_SKIN_FILENAME, conf->audio_skin_filename);
    snprintf(buffer, sizeof(buffer),"%i",conf->enable_small_text);
    iniparser_setstring(ini, SECTION_GENERAL":"KEY_EN_SMALL_TEXT, buffer);

    iniparser_dump_ini( ini, fp );
    fclose( fp );    
    system("cp -f " CONFIG_FILE " ./conf/tomplayer.ini");    
    system( "unix2dos ./conf/tomplayer.ini" );

error:
    iniparser_freedict(ini);   
    return ret;
}

/* Release config */
void config_free(void){
  if (config.diapo.filter != NULL) {
    free(config.diapo.filter);
  } 
  if (config.diapo.file_path != NULL){
    free(config.diapo.file_path);
  }  
}

void config_reload(void){
  config_free();
  load_config( &config);
}
