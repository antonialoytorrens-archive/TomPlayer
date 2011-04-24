/**
 * \file config.c
 * \brief This module implements configuration access 
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
#include "diapo.h"
#include "engine.h"
#include "widescreen.h"

/* Definition of section keywords */
#define SECTION_GENERAL             "general"
#define SECTION_VIDEO_SKIN          "video_skin"
#define SECTION_AUDIO_SKIN          "audio_skin"

/* Definition of keywords used in configuration file */
#define KEY_SKIN_FILENAME           "filename"
#define KEY_FILTER_VIDEO_EXT        "filter_video"
#define KEY_FILTER_AUDIO_EXT        "filter_audio"
#define KEY_VIDEO_FILE_DIRECTORY    "video_dir"
#define KEY_AUDIO_FILE_DIRECTORY    "audio_dir"
#define KEY_SKIN_CONF               "conf"
#define KEY_SCREEN_SAVER_TO         "screen_saver_to"
#define KEY_EN_SCREEN_SAVER         "enable_screen_saver"
#define KEY_FM_TRANSMITTER          "fm_transmitter"
#define KEY_FM_TRANSMITTER1         "fm_transmitter1"
#define KEY_FM_TRANSMITTER2         "fm_transmitter2"
#define KEY_EN_FM_TRANSMITTER       "enable_fm_transmitter"
#define KEY_EN_SMALL_TEXT           "enable_small_text"
#define KEY_DIAPO_ENABLED   "diapo_enabled"
#define KEY_DIAPO_FILTER    "diapo_filter"
#define KEY_DIAPO_PATH      "diapo_path"
#define KEY_DIAPO_DELAY     "diapo_delay"
#define KEY_INTERNAL_SPEAKER "int_speaker"
#define KEY_VIDEO_PREVIEW "video_preview"
#define KEY_AUTO_RESUME "auto_resume"

/* Default timeout in seconds before turning OFF screen while playing audio if screen saver is active */
#define SCREEN_SAVER_TO_S 6

/* Default folder for media if the specified one doesn't exist */
#define DEFAULT_FOLDER "/mnt"

/* Correct iniparser bug in naming function iniparser_setstring/iniparser_set */
extern int iniparser_set(dictionary * ini, char * entry, const char * val);
#define iniparser_setstring iniparser_set

/* Helper macro */
#define SET_STRING(target, source) do {if (source != NULL) target = strdup(s);\
                                       if (target == NULL) goto load_error;} while(0)

/* Main configuration structure */
struct tomplayer_config{
    char *filter_video_ext;             /*!<List of supported video file extension */
    char *filter_audio_ext;             /*!<List of supported audio file extension */
    char *video_folder;                 /*!<fullpath to the video folder */
    char *audio_folder;                 /*!<fullpath to the audio folder */
    char *video_skin_filename;          /*!<fullpath to the video skin archive */
    char *audio_skin_filename;          /*!<fullpath to the audio skin archive */   
    int screen_saver_to;                /*!<screensaver timeout */
    int enable_screen_saver;            /*!<Enable Screen saver */
    unsigned int fm_transmitter;        /*!<FM transmitter frequency in HZ  */
    unsigned int fm_transmitter1;       /*!<First FM transmitter frequency backup  */
    unsigned int fm_transmitter2;       /*!<Second FM transmitter frequency backup */ 
    int enable_fm_transmitter;          /*!<Enable FM transmitter*/ 
    int diapo_enabled;                  /*!<Enable Diaporama */     
    struct diapo_config diapo;          /*!<Diaporama Config */
    int enable_small_text;              /*!<Enable samll text in file selector*/
    enum config_int_speaker_type int_speaker; /*!<Internal speaker configuration*/
    int video_preview;                  /*!<Enable video preview*/    
    int auto_resume;                    /*!<Enable auto resume*/    
};

/* Current configuration object */ 
static struct tomplayer_config config;

/** Display configuration parameters
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


/** Load the main configuration and skin configuration
 *
 * \param conf main configuration structure
 *
 * \return true on succes, false on failure
 */
static bool load_config( struct tomplayer_config * conf ){
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
    SET_STRING(conf->video_folder, s);    
    if (stat(conf->video_folder, &folder_stats) != 0){
        free(conf->video_folder);
        SET_STRING(conf->video_folder,DEFAULT_FOLDER);
    }
    
    s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_AUDIO_FILE_DIRECTORY, NULL);
    SET_STRING(conf->audio_folder, s);    
    if (stat(conf->audio_folder, &folder_stats) != 0){      
        free(conf->audio_folder);
        SET_STRING(conf->audio_folder,DEFAULT_FOLDER);
    }

    s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_FILTER_VIDEO_EXT, NULL);
    SET_STRING(conf->filter_video_ext, s);        
    s = iniparser_getstring(ini, SECTION_GENERAL":"KEY_FILTER_AUDIO_EXT, NULL);
    SET_STRING(conf->filter_audio_ext, s);    
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
    conf->auto_resume = iniparser_getint(ini, SECTION_GENERAL":"KEY_AUTO_RESUME, 0);
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
      conf->diapo.type = conf->diapo_enabled;
    }

    s = iniparser_getstring(ini, SECTION_VIDEO_SKIN":"KEY_SKIN_FILENAME, NULL);
    SET_STRING(conf->video_skin_filename, s);        
    s = iniparser_getstring(ini, SECTION_AUDIO_SKIN":"KEY_SKIN_FILENAME, NULL);
    SET_STRING(conf->audio_skin_filename, s);
    
    conf->enable_small_text = iniparser_getint(ini, SECTION_GENERAL":"KEY_EN_SMALL_TEXT, 0);   
    iniparser_freedict(ini);
  
    display_config( conf );    
    return true;

load_error:
    config_free();
    return false;
}


/** Initialize configuration object by reading configuration file */
bool config_init(void){
    return load_config(&config);   
}


/* -- GET accessors -- */

bool config_get_screen_saver(void){
    return config.enable_screen_saver ;
}

int config_get_screen_saver_to(void){
    return config.screen_saver_to;
}

const struct diapo_config *config_get_diapo(void){
    return &config.diapo ;
}

bool config_get_diapo_activation(void){
    return config.diapo_enabled;
}

bool config_get_fm_activation(void){
    return config.enable_fm_transmitter;
}

bool config_get_small_text_activation(void){
    return config.enable_small_text;
}

bool config_get_auto_resume(void){
    return config.auto_resume;
}

const char *config_get_ext(enum config_type type){
    switch (type){
        case CONFIG_AUDIO: 
            return config.filter_audio_ext;
        case CONFIG_VIDEO:
            return config.filter_video_ext;
        default :
            return NULL;
    }            
}

bool config_get_video_preview(void){
    return config.video_preview;
}

enum config_int_speaker_type config_get_speaker(void){
    return config.int_speaker;
}

unsigned int config_get_fm(enum config_fm_type type){
    switch(type){
        case CONFIG_FM_DEFAULT:
            return config.fm_transmitter;
        case CONFIG_FM_SAV1:
            return config.fm_transmitter1;
        case CONFIG_FM_SAV2:
            return config.fm_transmitter2;
        default :
            return config.fm_transmitter;
    }
}

const char *config_get_folder(enum config_type type){
    switch (type){
        case CONFIG_AUDIO: 
            return config.audio_folder;
        case CONFIG_VIDEO:
            return config.video_folder;
        default :
            return NULL;
    }            
}

const char *config_get_skin_filename(enum config_type type){
    switch (type){
        case CONFIG_AUDIO: 
            return config.audio_skin_filename;
        case CONFIG_VIDEO:
            return config.video_skin_filename;
        default :
            return NULL;
    }            
}


/* -- SET accessors -- */

bool config_set_skin_filename(enum config_type type, const char * filename){
  switch (type){
    case CONFIG_AUDIO:
        free(config.audio_skin_filename);
        config.audio_skin_filename = strdup(filename);
        return (config.audio_skin_filename != NULL);      
    break;
    case CONFIG_VIDEO:
        free(config.video_skin_filename);
        config.video_skin_filename = strdup(filename);
        return (config.video_skin_filename != NULL);                    
    break;
    default :
      return false;
  }  
}

bool config_set_default_folder(enum config_type type, const char * folder){
 switch (type){
    case CONFIG_AUDIO:
        free(config.audio_folder);        
        config.audio_folder = strdup(folder);
        return (config.audio_folder != NULL);
    break;
    case CONFIG_VIDEO:
        free(config.video_folder);        
        config.video_folder = strdup(folder);
        return (config.video_folder != NULL);
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


bool config_set_fm_frequency(enum config_fm_type type, unsigned int freq){
    if ((freq  < 87000000) || (freq > 108000000))
        return false;        
    switch(type){
        case CONFIG_FM_DEFAULT:
            config.fm_transmitter = freq;
            break;            
        case CONFIG_FM_SAV1:
            config.fm_transmitter1 = freq;
            break;
        case CONFIG_FM_SAV2:
            config.fm_transmitter2 = freq;
            break;
        default :
            return false;
    }
    return true;    
}
    

bool config_set_int_speaker(enum config_int_speaker_type mode){
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

/** Write configuration file */
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
    system("cp -f " CONFIG_FILE " ./conf/tomplaye.ini");    
    system( "unix2dos ./conf/tomplaye.ini" );

error:
    iniparser_freedict(ini);   
    return ret;
}

/** Release configuration */
void config_free(void){  
  free(config.filter_video_ext);
  free(config.filter_audio_ext);
  free(config.video_folder);
  free(config.audio_folder);
  free(config.video_skin_filename);
  free(config.audio_skin_filename);
  free(config.diapo.filter);
  free(config.diapo.file_path);
  config.filter_video_ext = NULL;
  config.filter_audio_ext = NULL;
  config.video_folder = NULL;
  config.audio_folder = NULL; 
  config.video_skin_filename = NULL;
  config.audio_skin_filename = NULL;
  config.diapo.filter = NULL;
  config.diapo.file_path = NULL;
}

/** Relaod configuration */
void config_reload(void){
  config_free();
  load_config(&config);
}
