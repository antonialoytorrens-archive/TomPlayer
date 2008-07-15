/***************************************************************************
 *
 *  Sound related functions
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
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

/**
 * \file sound.c
 * \author wolfgar
 * \brief Sound related functions
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

#define SOUND_DEV_NAME "/dev/sound"
#define DEVICE_NOT_OPENED -2
static int snd_fd = DEVICE_NOT_OPENED;


/**
 * \fn int snd_check_headphone(void)
 * \brief Check for headphones presence to turn off/on internal speaker accordingly
 *
 * \return 0  on success, -1 on failure
 */
int snd_check_headphone(void){
	int res = 0;
	unsigned int is_headphone = 0;

	if (snd_fd == DEVICE_NOT_OPENED){
		/* If device not yet opened, then try to */
		snd_fd = open(SOUND_DEV_NAME, O_RDWR);
		if (snd_fd < 0){
		    perror("Error while trying to open sound device : ");
		    return -1;
		}
	}

	/* Test whether headphones are connected */
	res = ioctl (snd_fd, COOLSOUND_GET_HEADPHONE_CONNECTED, &is_headphone);
	if ( res != 0){
	    perror("Error while trying to get headphones status : ");
	    return -1;
	}


	if (is_headphone){
		/* Mute internal speaker when headphones are plugged */
		res = ioctl (snd_fd,COOLSOUND_MUTE_INTERNAL);
		if ( res != 0){
			perror("Error while trying to  mute internal : ");
		   	return -1;
		}
	} else {
		/* Unmute internal speaker when headphones are not plugged */
		res = ioctl (snd_fd,COOLSOUND_UNMUTE_INTERNAL);
		if ( res != 0){
			perror("Error while trying to unmute internal : ");
		   	return -1;
		}
	}

	return res;
}




