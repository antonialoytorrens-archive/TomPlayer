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


#define PWM_DEFAULT_LIGHT (PWM_BACKLIGHT_MAX - 20)
static  int previous_setting =  PWM_DEFAULT_LIGHT;

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
	previous_setting = ioctl (fd, IOR_BACKLIGHT_CURRENT);
	if ( previous_setting < 0){
	    perror("Error while trying to get current backlight value : ");
	    previous_setting =  PWM_DEFAULT_LIGHT;
	    res = -1;
	    goto out_pwm_off;		        
	}	
	if (ioctl (fd, IOW_BACKLIGHT_OFF) != 0){
	    perror("Error while turning OFF the screen : ");
	    res = -1;
	}

out_pwm_off:
	close(fd);
	return res;	
}


/** Restore screen to its previous state
 * 
 * \Note Do Nothing if the screen is not OFF
 */
int pwm_resume(void) {
	int fd;
	int res = 0;
	int current_val;
	
	fd = open("/dev/" PWM_DEVNAME, O_RDWR);
	if (fd < 0){
	    perror("Error while trying to open PWM module : ");
	    return -1;
	}
	current_val = ioctl (fd, IOR_BACKLIGHT_CURRENT);
	if ( current_val < 0){
	    perror("Error while trying to get current backlight value : ");
	    res = -1;	   
	    goto out_pwm_resume;		  
	}
	if (current_val == PWM_BACKLIGHT_MIN) {
		/* Turn ON screen only if it is OFF */		
		if (ioctl (fd, IOW_BACKLIGHT_UPDATE , previous_setting) != 0){
		    perror("Error while turning to restore previous setting : ");
		    res = -1;
		}
	}

out_pwm_resume:
	close(fd);
	return res;		
}


/** Indicates wether the screen is OFF or not.
 * 
 * \note if an error occurs, the screen is supposed ON
 */ 
int pwm_is_on(void){
	int fd;
	int res = 1;
	int current_val;
	
	
	fd = open("/dev/" PWM_DEVNAME, O_RDWR);
	if (fd < 0){
	    perror("Error while trying to open PWM module : ");
	    return -1;
	} 	
	current_val = ioctl (fd, IOR_BACKLIGHT_CURRENT);
	if ( current_val < 0){
	    perror("Error while trying to get current backlight value : ");
	    res = -1;	   
	}		
	if (current_val == 0) {
		res = 0;
	} 	

	close(fd);
	return res;			
}
