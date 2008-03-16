/***************************************************************************
 * Resume file handling
 *
 *  Mon Feb 27 2008
 *  Copyright  2008  Stéphan Rafin
 *  Email
 ****************************************************************************/
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
#include <minigui/common.h>
#include <minigui/minigui.h>

#include "resume.h"

#define RESUME_FILENAME "./resume.ini"

#define RESUME_SECTION_KEY "RESUME"
#define RESUME_VIDEO_SETTINGS_SECTION_KEY "VIDEO SETTINGS"
#define RESUME_AUDIO_SETTINGS_SECTION_KEY "AUDIO SETTINGS"

#define RESUME_FILENAME_KEY "file"
#define RESUME_POS_KEY "pos"
#define RESUME_CONTRAST_KEY "contrast"
#define RESUME_BRIGHTNESS_KEY "brightness"
#define RESUME_VOLUME_KEY "volume"
#define RESUME_AUDIO_DELAY_KEY "audio_delay"

/** Reinit the resume file 
*
* \param file The opened media filename
*
*\retval  0  OK
*\retval -1  KO
*/
int resume_file_init(char * file){    

  if (SetValueToEtcFile(RESUME_FILENAME,  RESUME_SECTION_KEY,  RESUME_FILENAME_KEY, file) != ETC_OK){
    fprintf(stderr,"Error while writing media file entry in resume file : %s \n ", RESUME_FILENAME);
    return -1;
  } else {
    if (SetValueToEtcFile(RESUME_FILENAME,  RESUME_SECTION_KEY,  RESUME_POS_KEY, "-1") != ETC_OK){
      fprintf(stderr,"Error while reinitializing position value in resume file : %s \n ", RESUME_FILENAME);
      return -1;
    }    
  }
  return 0;	
}


/** Write position entry in resume file 
*
* \param value The current position in seconds
*
*\retval  0  OK
*\retval -1  KO
*/
int resume_write_pos(int value){
  char buffer[100];

  snprintf(buffer, sizeof(buffer),"%i", value);
  if (SetValueToEtcFile(RESUME_FILENAME,  RESUME_SECTION_KEY,  RESUME_POS_KEY, buffer) != ETC_OK){
    fprintf(stderr,"Error while writing position value %i in resume file : %s \n ", value, RESUME_FILENAME);
    return -1;
  }    
  return 0;
}

/** Get media file and position entry in resume file 
*
*\param filename[out] Buffer where the media filename has to be stored
*\param len Length of the buffer where thefilename has to be copied 
*\param pos[out] Last position in seconds
*
* \retval 0  : OK
* \retval -1 :  An error occured 
*/
int resume_get_file_infos(char * filename, int len , int * pos){
  char buffer[100];  


  *pos = 0;
  if (len <= 0){
    return -1;
  }
  filename[0] = 0;

  if (GetValueFromEtcFile( RESUME_FILENAME, RESUME_SECTION_KEY, RESUME_POS_KEY, buffer, sizeof(buffer)) != ETC_OK){
    fprintf(stderr,"Error while getting position value in resume file : %s \n ", RESUME_FILENAME);
    return -1;
  }
  if (sscanf(buffer,"%i", pos) != 1){
    fprintf(stderr,"Error while parsing position value : %s \n ", buffer);
    return -1;
  }  
  if (GetValueFromEtcFile( RESUME_FILENAME, RESUME_SECTION_KEY, RESUME_FILENAME_KEY, filename, len) != ETC_OK){
    fprintf(stderr,"Error while getting media filename in : %s \n ", RESUME_FILENAME);
    return -1;
  }

  return 0;
}


/** Read video settings from resume file
 * \param settings[out] read settings 
 * 
 *\retval  0  OK
 *\retval -1  KO
 */
int resume_get_audio_settings(struct audio_settings * settings){
	GHANDLE gh_resume;
	int res = 0;
	 
	gh_resume = LoadEtcFile( RESUME_FILENAME );	
	if( gh_resume == ETC_FILENOTFOUND ){
       fprintf(stderr, "Warning : Unable to load resume file <%s>\n", RESUME_FILENAME );
       return -1;
	}		
    if (GetIntValueFromEtc( gh_resume, RESUME_AUDIO_SETTINGS_SECTION_KEY, RESUME_VOLUME_KEY, &settings->volume ) != ETC_OK ){
    	res = -1;
    	goto out_audio_settings;
    }	
     
out_audio_settings: 
    UnloadEtcFile( gh_resume);
    return res;
}


/** Read audio settings from resume file
 * \param settings[out] read settings 
 * 
 *\retval  0  OK
 *\retval -1  KO
 */
int resume_get_video_settings(struct video_settings * settings) {
	GHANDLE gh_resume;
	char buffer[256];
	int res = 0;
	 
	gh_resume = LoadEtcFile( RESUME_FILENAME );	
	if( gh_resume == ETC_FILENOTFOUND ){
       fprintf(stderr, "Warning : Unable to load resume file <%s>\n", RESUME_FILENAME );
       return -1;
	}

    if (GetIntValueFromEtc( gh_resume, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_CONTRAST_KEY, &settings->contrast ) != ETC_OK ){
    	res = -1;
    	goto out_video_settings;
    }
    if (GetIntValueFromEtc( gh_resume, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_BRIGHTNESS_KEY, &settings->brightness ) != ETC_OK ){
    	res = -1;
    	goto out_video_settings;
    }	
    if (GetIntValueFromEtc( gh_resume, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_VOLUME_KEY, &settings->volume ) != ETC_OK ){
    	res = -1;
    	goto out_video_settings;
    }    
        
    if (GetValueFromEtc( gh_resume, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_AUDIO_DELAY_KEY, buffer, sizeof(buffer) ) != ETC_OK ){
    	res = -1;
    	goto out_video_settings;
    }
    if (sscanf(buffer,"%f",&settings->audio_delay) != 1){
    	res = -1;
    }

    
out_video_settings: 
    UnloadEtcFile( gh_resume);
    return res;
}


/** Write audio settings to resume file
 * \param settings[in] settings to be written to the file
 * 
 *\retval  0  OK
 *\retval -1  KO
 */
int resume_set_audio_settings(const struct audio_settings * settings){	
	char buffer[256];
	
    snprintf(buffer,sizeof(buffer),"%i",settings->volume);
    if (SetValueToEtcFile( RESUME_FILENAME, RESUME_AUDIO_SETTINGS_SECTION_KEY, RESUME_VOLUME_KEY,buffer) != ETC_OK ){
    	return -1;
    }
    		        
    return 0;
	
}

/** Write video settings to resume file
 * \param settings[in] settings to be written to the file
 * 
 *\retval  0  OK
 *\retval -1  KO
 */
int resume_set_video_settings(const struct video_settings * settings) {
	
	char buffer[256];	
	
	snprintf(buffer,sizeof(buffer),"%i",settings->contrast);	 
    if (SetValueToEtcFile( RESUME_FILENAME, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_CONTRAST_KEY, buffer )  != ETC_OK ){
    	return -1;
    }
    snprintf(buffer,sizeof(buffer),"%i",settings->brightness);
    if (SetValueToEtcFile( RESUME_FILENAME, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_BRIGHTNESS_KEY, buffer ) != ETC_OK ){    	
    	return -1;
    }	
    snprintf(buffer,sizeof(buffer),"%i",settings->volume);
    if (SetValueToEtcFile( RESUME_FILENAME, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_VOLUME_KEY,buffer) != ETC_OK ){
    	return -1;
    }    
    snprintf(buffer,sizeof(buffer),"%f",settings->audio_delay);    
    if (SetValueToEtcFile( RESUME_FILENAME, RESUME_VIDEO_SETTINGS_SECTION_KEY, RESUME_AUDIO_DELAY_KEY, buffer ) != ETC_OK ){    	
    	return -1;
    }
    		        
    return 0;
}
