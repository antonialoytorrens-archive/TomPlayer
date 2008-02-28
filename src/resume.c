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

#define RESUME_FILENAME "./resume.ini"
#define RESUME_SECTION_KEY "RESUME"
#define RESUME_FILENAME_KEY "file"
#define RESUME_POS_KEY "pos"

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

/** Get position entry in resume file 
*
* \retval >=0 : The position in seconds
* \retval -1 :  An error occured 
*/
int resume_get_pos(void){
  char buffer[100];
  int ret;

  if (GetValueFromEtcFile( RESUME_FILENAME, RESUME_SECTION_KEY, RESUME_POS_KEY, buffer, sizeof(buffer)) != ETC_OK){
    fprintf(stderr,"Error while getting position value in resume file : %s \n ", RESUME_FILENAME);
    return -1;
  }
  if (sscanf(buffer,"%i", &ret) != 1){
    fprintf(stderr,"Error while parsing position value : %s \n ", buffer);
    return -1;
  }  
  return ret;
}
