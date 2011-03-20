/**
 * \file resume.c
 * \author wolfgar
 * \brief Resume file handling
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



#include <stdbool.h>
#include <stdio.h>
#include <dictionary.h>
#include <iniparser.h>
#include <linux/limits.h>

#include "resume.h"
#include "debug.h"

/* Correct iniparser bug in naming function iniparser_setstring/iniparser_set */
extern int iniparser_set(dictionary * ini, char * entry, char * val);
#define iniparser_setstring iniparser_set

#define RESUME_FILENAME "./conf/resume.ini"

#define RESUME_VIDEO_SECTION_KEY "RESUME VIDEO"
#define RESUME_AUDIO_SECTION_KEY "RESUME AUDIO"
#define RESUME_VIDEO_SETTINGS_SECTION_KEY "VIDEO SETTINGS"
#define RESUME_AUDIO_SETTINGS_SECTION_KEY "AUDIO SETTINGS"
#define RESUME_FILENAME_KEY "file"
#define RESUME_POS_KEY "pos"
#define RESUME_CONTRAST_KEY "contrast"
#define RESUME_BRIGHTNESS_KEY "brightness"
#define RESUME_VOLUME_KEY "volume"
#define RESUME_AUDIO_DELAY_KEY "audio_delay"


#define RESUME_PLAYLIST_FILENAME(x) (x == MODE_AUDIO)?"./conf/sav_apl.m3u":"./conf/sav_vpl.m3u"
#define RESUME_SECTION_KEY_GET(x) (x == MODE_AUDIO)?RESUME_AUDIO_SECTION_KEY:RESUME_VIDEO_SECTION_KEY

/**
 * Reinit the resume file
 *
 * \return 0  on success, -1 on failure
 */
int resume_file_init(enum engine_mode mode){  
	dictionary * ini ;
  char ini_path[100];
	FILE * fp;
	int ret = 0;  
  
	ini = iniparser_load(RESUME_FILENAME);
	if( ini == NULL ){
		PRINTD( "resume file doesn't exist\n" );
		ini = dictionary_new( 0 );
	}

	fp = fopen( RESUME_FILENAME, "w+" );
	if( fp == NULL ){
		PRINTDF( "Unable to create resume file <%s>\n", RESUME_FILENAME );
		ret = -1;
		goto error;
	}  
	iniparser_setstring( ini, RESUME_SECTION_KEY_GET(mode), NULL );
  snprintf(ini_path, sizeof(ini_path), "%s:%s", RESUME_SECTION_KEY_GET(mode), RESUME_FILENAME_KEY);
  ini_path[sizeof(ini_path)-1] = 0;
	iniparser_setstring( ini, ini_path, RESUME_PLAYLIST_FILENAME(mode));
  snprintf(ini_path, sizeof(ini_path), "%s:%s", RESUME_SECTION_KEY_GET(mode), RESUME_POS_KEY);
  ini_path[sizeof(ini_path)-1] = 0;
	iniparser_setstring( ini, ini_path, "-1" );

	iniparser_dump_ini( ini, fp );

	fclose( fp );

error:
	iniparser_freedict(ini);

	return ret;
}

/** Write position entry in resume file
 *
 * \param value The current position in seconds
 *
 * \return 0  on success, -1 on failure
 */
int resume_write_pos(enum engine_mode mode, int value){
  char buffer[100];
  char ini_path[100];
  dictionary * ini ;
  FILE * fp;
  int ret = 0;

  ini = iniparser_load(RESUME_FILENAME);
  if( ini == NULL ){
  	PRINTDF( "Unable to load resume file <%s>\n", RESUME_FILENAME );
  	return -1;
  }

  snprintf(buffer, sizeof(buffer),"%i", value);
  iniparser_setstring( ini, RESUME_SECTION_KEY_GET(mode), NULL );
  snprintf(ini_path, sizeof(ini_path), "%s:%s", RESUME_SECTION_KEY_GET(mode), RESUME_POS_KEY);
  ini_path[sizeof(ini_path)-1] = 0;
  iniparser_setstring( ini, ini_path, buffer );  
  fp = fopen( RESUME_FILENAME, "w+" );
  if( fp == NULL ){
  	PRINTDF( "Unable to create resume file <%s>\n", RESUME_FILENAME );
  	ret = -1;
  	goto error;
  }

  iniparser_dump_ini( ini, fp );
  fclose( fp );

error:
  iniparser_freedict(ini);
  return ret;
}


/** Get media file and position entry in resume file
 *
 * \param mode audio or video mode
 * \param filename Buffer where the media filename has to be stored
 * \param len Length of the buffer where thefilename has to be copied
 * \param pos Last position in seconds
 *
 * \return 0  on success, -1 on failure
 */
int resume_get_file_infos(enum engine_mode mode, char * filename, int len , int * pos){
  dictionary * ini ;
  char ini_path[100];
  char *s;
  int ret = 0;


  *pos = 0;
  if (len <= 0){
    return -1;
  }
  memset( filename, 0, len );

  ini = iniparser_load(RESUME_FILENAME);
  if( ini == NULL ){
  	PRINTDF( "Unable to load resume file <%s>\n", RESUME_FILENAME );
  	return -1;
  }

  snprintf(ini_path, sizeof(ini_path), "%s:%s", RESUME_SECTION_KEY_GET(mode), RESUME_FILENAME_KEY);
  ini_path[sizeof(ini_path)-1] = 0;
  s = iniparser_getstring(ini, ini_path, NULL);
  if( s != NULL ) strncpy( filename, s, len-1 );
  else{
  	ret = -1;
  	PRINTDF( "Error while getting media filename in : %s \n ", RESUME_FILENAME);
  	goto error;
  }
  
  snprintf(ini_path, sizeof(ini_path), "%s:%s", RESUME_SECTION_KEY_GET(mode), RESUME_POS_KEY);
  ini_path[sizeof(ini_path)-1] = 0;
  s = iniparser_getstring(ini, ini_path, NULL);
  if( s != NULL ){
  	if (sscanf(s ,"%i", pos) != 1){
  		ret = -1;
  		PRINTDF("Error while parsing position value : %s \n ", s);
  		goto error;
  	}
  }
  else{
  	ret = -1;
  	PRINTDF("Error while getting position value in resume file : %s \n ", RESUME_FILENAME);
  	goto error;
  }

error:
  iniparser_freedict(ini);
  return ret;
}



/** Read video settings from resume file
 *
 * \param settings read settings
 *
 * \return 0  on success, -1 on failure
 */
int resume_get_audio_settings(struct audio_settings * settings){
	dictionary * ini ;
	int res = 0;
	int i;

	ini = iniparser_load(RESUME_FILENAME);
	if( ini == NULL ){
		PRINTDF( "Warning : Unable to load resume file <%s>\n", RESUME_FILENAME );
		return -1;
	}

	i = iniparser_getint(ini, RESUME_AUDIO_SETTINGS_SECTION_KEY":"RESUME_VOLUME_KEY, -1);
	if( i < 0 ){
		PRINTD( "Warning : Unable to get volume from resume file\n");
		res = -1;
		goto out_audio_settings;
	}
	else settings->volume = i;


out_audio_settings:
	iniparser_freedict(ini);
    return res;
}

/** Get media file and position entry in resume file
 *
 * \param settings read settings
 *
 * \return 0  on success, -1 on failure
 */
int resume_get_video_settings(struct video_settings * settings) {
	dictionary * ini ;
	bool res = 0;
	int i;
	char *s;

	ini = iniparser_load(RESUME_FILENAME);
	if( ini == NULL ){
		PRINTDF( "Warning : Unable to load resume file <%s>\n", RESUME_FILENAME );
		return -1;
	}


	i = iniparser_getint(ini, RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_CONTRAST_KEY, -1);
	if( i < 0 ){
		res = -1;
		goto out_video_settings;
	}
	else settings->contrast = i;

	i = iniparser_getint(ini, RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_BRIGHTNESS_KEY, -1);
	if( i < 0 ){
		res = -1;
		goto out_video_settings;
	}
	else settings->brightness = i;

	i = iniparser_getint(ini, RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_VOLUME_KEY, -1);
	if( i < 0 ){
		res = -1;
		goto out_video_settings;
	}
	else settings->volume = i;



	s = iniparser_getstring(ini, RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_AUDIO_DELAY_KEY, NULL);
	if( s != NULL ){
		if (sscanf(s,"%f",&settings->audio_delay) != 1){
			res = -1;
			goto out_video_settings;
		}
	}
	else{
		res = -1;
		PRINTDF( "Error while getting in <%s> resume file\n ", RESUME_AUDIO_DELAY_KEY);
		goto out_video_settings;
	}

out_video_settings:
	iniparser_freedict(ini);
	return res;
}

/**
 * \fn int resume_set_audio_settings(const struct audio_settings * settings)
 * \brief Write audio settings to resume file
 *
 * \param settings read settings
 *
 * \return 0  on success, -1 on failure
 */
int resume_set_audio_settings(const struct audio_settings * settings){
	char buffer[256];
	dictionary * ini ;
	FILE * fp;
	int ret = 0;

	ini = iniparser_load(RESUME_FILENAME);
	if( ini == NULL ){
		PRINTDF( "Unable to load resume file <%s>\n", RESUME_FILENAME );
		return -1;
	}

	iniparser_setstring( ini, RESUME_AUDIO_SETTINGS_SECTION_KEY, NULL );
    snprintf(buffer,sizeof(buffer),"%i",settings->volume);
    iniparser_setstring( ini, RESUME_AUDIO_SETTINGS_SECTION_KEY":"RESUME_VOLUME_KEY, buffer );

    fp = fopen( RESUME_FILENAME, "w+" );
    if( fp == NULL ){
    	PRINTDF( "Unable to create resume file <%s>\n", RESUME_FILENAME );
    	ret = -1;
    	goto error;
    }

    iniparser_dump_ini( ini, fp );
    fclose( fp );

 error:
    iniparser_freedict(ini);
    return ret;



}


/** Write video settings to resume file
 *
 * \param settings read settings
 *
 * \return 0  on success, -1 on failure
 */
int resume_set_video_settings(const struct video_settings * settings) {
	char buffer[256];
	dictionary * ini ;
	FILE * fp;
	int ret = 0;

	ini = iniparser_load(RESUME_FILENAME);
	if( ini == NULL ){
		PRINTDF( "Unable to load resume file <%s>\n", RESUME_FILENAME );
		return false;
	}

	iniparser_setstring( ini, 	RESUME_VIDEO_SETTINGS_SECTION_KEY, NULL );
	snprintf(buffer,sizeof(buffer),"%i",settings->contrast);
	iniparser_setstring( ini, 	RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_CONTRAST_KEY, buffer );

    snprintf(buffer,sizeof(buffer),"%i",settings->brightness);
	iniparser_setstring( ini, 	RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_BRIGHTNESS_KEY, buffer );

	snprintf(buffer,sizeof(buffer),"%i",settings->volume);
	iniparser_setstring( ini, 	RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_VOLUME_KEY, buffer );

    snprintf(buffer,sizeof(buffer),"%f",settings->audio_delay);
	iniparser_setstring( ini, 	RESUME_VIDEO_SETTINGS_SECTION_KEY":"RESUME_AUDIO_DELAY_KEY, buffer );

	fp = fopen( RESUME_FILENAME, "w+" );
	if( fp == NULL ){
		PRINTDF( "Unable to create resume file <%s>\n", RESUME_FILENAME );
		ret = -1;
		goto error;
	}

	iniparser_dump_ini( ini, fp );
	fclose( fp );

error:
	iniparser_freedict(ini);
    return 0;
}


int resume_save_playslist(enum engine_mode mode, const char * current_filename){
  #define VOLATILE_PLAYLIST_FILENAME "/tmp/playlist.m3u"
  
  char buffer[PATH_MAX];
  FILE * in_pl = NULL;
  FILE * out_pl = NULL;
  bool found = false;

  in_pl = fopen(VOLATILE_PLAYLIST_FILENAME,"r");
  if (in_pl == NULL){
    return -1;
  }

  while (fgets(buffer, PATH_MAX-1,in_pl) != NULL){
    if (!found){
      if (strstr(buffer, current_filename) != NULL){
        out_pl = fopen(RESUME_PLAYLIST_FILENAME(mode),"w+");
        if (out_pl == NULL){
          break;
        }
        found = true;
        fwrite(buffer, strlen(buffer),1, out_pl);
      }
    } else {
      fwrite(buffer, strlen(buffer),1, out_pl);
    }
  }

  fclose(in_pl);
  if (out_pl)
    fclose(out_pl);
  if (found){
    return 0;
  } else {
    return -1;
  }
}
