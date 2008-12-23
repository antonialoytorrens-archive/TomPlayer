
/**
 * \file sound.c
 * \author wolfgar
 * \brief Sound related functions
 *
 *  Sound related functions
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


#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <barcelona/Barc_snd.h>

#include "sound.h"
#include "debug.h"

#define SOUND_DEV_NAME "/dev/sound"
#define DEVICE_NOT_OPENED -2
#define MODELID_PATH "/proc/barcelona/modelid"

static int check_fd(){
  static int snd_fd = DEVICE_NOT_OPENED;

  if (snd_fd == DEVICE_NOT_OPENED){          
          /* If device not yet opened, then try to */
          snd_fd = open(SOUND_DEV_NAME, O_RDWR);
          if (snd_fd < 0){
              perror("Error while trying to open sound device : ");
              return -1;
          }
  }
  return snd_fd;
}


/** Mute or Unmute external speaker 
*
* \param[in] state true to mute internal speaker, false to unmute.
*
* \return 0  on success, -1 on failure
*/
int snd_mute_external(bool state){
  int res;
  int snd_fd;

  snd_fd = check_fd();
  if (snd_fd<0){
    return -1;
  }

  if (state){
    res = ioctl (snd_fd,COOLSOUND_MUTE_EXTERNAL);
  } else {
    res = ioctl (snd_fd,COOLSOUND_UNMUTE_EXTERNAL);
  }
  if ( res != 0){
    perror("Error while trying to  mute/unmuet external  ");
    return -1;
  }
  return 0;


}


/** Mute or Unmute internal speaker 
*
* \param[in] state true to mute internal speaker, false to unmute.
*
* \return 0  on success, -1 on failure
*/
int snd_mute_internal(bool state){
  int res;
  int snd_fd;

  snd_fd = check_fd();
  if (snd_fd<0){
    return -1;
  }

  if (state){
    res = ioctl (snd_fd,COOLSOUND_MUTE_INTERNAL);
  } else {
    res = ioctl (snd_fd,COOLSOUND_UNMUTE_INTERNAL);
  }
  if ( res != 0){
    perror("Error while trying to  mute/unmuet internal  ");
    return -1;
  }
  return 0;


}

/** Check for headphones presence to turn off/on internal speaker accordingly
 *
 * \return 0  on success, -1 on failure
 */
int snd_check_headphone(void){
  int res = 0;
  unsigned int is_headphone = 0;
  int snd_fd;
  int test_fd;
  int modelid = 0;
  
  snd_fd = check_fd();
  if (snd_fd<0){
    return -1;
  }
  
  /* test whether headconnector exists !*/
  test_fd =  open(MODELID_PATH, O_RDWR);
  if (test_fd < 0 ){
    perror("Error while trying to open " MODELID_PATH);
    return -1;
  } else {
    char buffer[128];
    if (read(test_fd,buffer,sizeof(buffer)) <= 0){
      perror("Error while reading " MODELID_PATH);
      close (test_fd);
      return -1;          
    }
    modelid = strtol(buffer,NULL,10);    
  }
  close (test_fd);
  if (modelid == 24){
    PRINTDF("TT GO 740 Live - No headphoneconnector \n");
    return -1;
  }

  /* Test whether headphones are connected */
  res = ioctl (snd_fd, COOLSOUND_GET_HEADPHONE_CONNECTED, &is_headphone);
  if ( res != 0){
      perror("Error while trying to get headphones status : ");
      return -1;
  }
  
  
  if (is_headphone){
    /* Mute internal speaker when headphones are plugged */
    snd_mute_internal(true);	
  } else {
    /* Unmute internal speaker when headphones are not plugged */
    snd_mute_internal(false);		
  }
  return res;
}





