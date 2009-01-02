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


    for( i = 0; i < MAX_SKIN_CONTROLS; i++ ){
		sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_TYPE_CONTROL );
		j = iniparser_getint(ini, section_control, -1);
        if( j < 0  ){
           PRINTDF( "Warning : no section  <%s>\n", section_control );
           break;
        } else {
	   skin_conf->controls[i].type = j;
        }

        /* Read filename to load
         *
         * Wolf  : suppress dynamic allocation is not such a good idea : it wastes about 80Ko
         * But now,  with mulitple config during the same session, a conf cleanup is needed (in unload_skin ?  Todo later)...
         */
	sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CTRL_BITMAP_FILENAME );
	s = iniparser_getstring(ini, section_control, NULL);
	if( s != NULL ) strcpy( skin_conf->controls[i].bitmap_filename, s );


	sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL );
	skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
        if( skin_conf->controls[i].cmd < 0  ){
			sprintf( section_control, SECTION_CONTROL_FMT_STR, i, KEY_CMD_CONTROL2 );
			skin_conf->controls[i].cmd = iniparser_getint(ini, section_control, -1);
		}

        if (skin_conf->controls[i].cmd == SKIN_CMD_BATTERY_STATUS){
        	PRINTDF("Battery conf index :%i - filename : %s\n",i,skin_conf->controls[i].bitmap_filename);
        	skin_conf->bat_index = i;
        }

        /* Fill table cmd -> skin index */
        if ((skin_conf->controls[i].cmd >= 0) &&
        	(skin_conf->controls[i].cmd < SKIN_CMD_MAX_NB)){
        	skin_conf->cmd2idx[skin_conf->controls[i].cmd] = i;
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

				if ((skin_conf->controls[i].type == PROGRESS_SKIN_CONTROL_X) ||
					(skin_conf->controls[i].type == PROGRESS_SKIN_CONTROL_Y)){
					skin_conf->progress_bar_index = i;
				}
                break;
            default:
                fprintf( stderr, "Type not defined correctly for %s\n", section_control );
                ret = false;
                goto error;
        }
    }

    skin_conf->nb = i;

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

bool config_set_diapo_conf(int enable, const char * folder, int delay){
   config.diapo_enabled = enable;
   config.diapo.delay = delay;
   if (config.diapo.file_path){
      free(config.diapo.file_path);
      config.diapo.file_path = strdup(folder);
      if (config.diapo.file_path  == NULL){
        return false;
      }
   }
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
