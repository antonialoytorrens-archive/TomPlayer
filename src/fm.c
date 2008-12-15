/**
 * \file fm.c
 * \author Stephan Rafin
 *
 * This module implements FM transmitter related functions
 *
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
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "linux/fmtransmitter.h"
#include "fm.h"

#define FM_DEVICE_FILENAME "/dev/fmt"
#define FM_FD_NOT_OPENED -1
#define FM_FD_ERROR -2

#ifdef NATIVE
/* Stubs for native compilation */
bool fm_set_state(unsigned int state){
  return true;
}
bool fm_set_freq(unsigned int freq){
  return true;
}
bool fm_set_stereo(unsigned int stereo){
  return true;
}
bool fm_set_power(unsigned char power){
  return true;
}
#else
static int test_device(){
  static int fd = FM_FD_NOT_OPENED;

  if (fd == FM_FD_NOT_OPENED){
    fd = open(FM_DEVICE_FILENAME,O_RDWR);
    if (fd < 0){
      perror("Error while trying to FM device ");
      fd = FM_FD_ERROR ;
    }
  }
  return fd;
}

/** Enable or disable the FM transmitter 
*
* \param[in] state 0 to disable and 1 to enable
* 
* \retval true  OK
* \retval false KO
*
*/
bool fm_set_state(unsigned int state){
  int fd ; 

  fd = test_device();
  if ( fd < 0 ){
    return false;
  }
  if (ioctl(fd, IOW_ENABLE , state) != 0){
    perror("Error while trying to change FM transmitter state ");
    return false;
  }
  return true;
}


/** Set FM transmitter frequency
*
* \param[in] freq Frequency to set in Hz (Should be in range [87000000 108000000] )
* 
* \retval true  OK
* \retval false KO
*
*/
bool fm_set_freq(unsigned int freq){
  int fd ; 
   
  fd = test_device();
  if ( fd < 0 ){
    return false;
  }
  /* Do not test return as return is not 0 when OK ... */
  ioctl(fd, IOW_SET_FM_FREQUENCY , freq);
  return true;
}



/** Set transmitter frequency stereo mode
*
* \param[in] stereo 0 for mono - 1 for stereo
* 
* \retval true  OK
* \retval false KO
*
*/
bool fm_set_stereo(unsigned int stereo){
  int fd ; 
   
  fd = test_device();
  if ( fd < 0 ){
    return false;
  }
  if (ioctl(fd, IOW_FMTRX_SET_MONO_STEREO , stereo) != 0){
    perror("Error while trying to set stereo on FM transmitter ");
    return false;
  }
  return true;
}



/** Set FM transmitter power
*
* \param[in] power power (should be in range [88 - 120])
* 
* \retval true  OK
* \retval false KO
*
*/
bool fm_set_power(unsigned char power){
  int fd ; 

  fd = test_device();
  if ( fd < 0 ){
    return false;
  }
  /* Do not test return as return is not 0 when OK ... */
  if (ioctl(fd, IOW_FMTRX_SET_POWER , power) != 0);
    
  return true;
}

#endif
