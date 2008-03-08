 /***************************************************************************
 *  Enable to turn off/on screen for power consumption savings
 *
 * 
 *  Copyright  2008  Stéphan Rafin
 *  Email* 
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

#include <barcelona/Barc_pwm.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "pwm.h"

static  int previous_setting =  PWM_BACKLIGHT_MAX;

/** Turn OFF the screen and save the current state to be able to restore
 */
int pwm_off(void) {
	int fd;
	int res = 0;
	
	
	fd = open("/dev/" PWM_DEVNAME, O_RDWR);
	if (fd < 0){
	    perror("Error while trying to open PWM module : ");
	    return -1;
	} 	
	if (ioctl (fd, IOR_BACKLIGHT_CURRENT, &previous_setting) != 0){
	    perror("Error while trying to get current backlight value : ");
	    res = -1;
	    goto out_pwm_off;		        
	}	
	if (ioctl (fd, IOW_BACKLIGHT_OFF, NULL) != 0){
	    perror("Error while turning OFF the screen : ");
	    res = -1;
	}

out_pwm_off:
	close(fd);
	return res;	
}


/** Restore screen to its previous state
 * 
 */
int pwm_resume(void) {
	int fd;
	int res = 0;
	
	fd = open("/dev/" PWM_DEVNAME, O_RDWR);
	if (fd < 0){
	    perror("Error while trying to open PWM module : ");
	    return -1;
	}
	if (ioctl (fd, IOW_BACKLIGHT_ON, &previous_setting) != 0){
	    perror("Error while trying to turn on backlight : ");
	    res = -1;
	    goto out_pwm_resume;		        
	}
	if (ioctl (fd, IOW_BACKLIGHT_UPDATE , &previous_setting) != 0){
	    perror("Error while turning to restore previous setting : ");
	    res = -1;
	}

out_pwm_resume:
	close(fd);
	return res;		
}
