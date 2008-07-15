 /***************************************************************************
 *  Enable to turn off/on screen for power consumption savings
 *
 * $URL$
 * $Rev$
 * $Author$
 * $Date$
 *
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
 * \file pwm.c
 * \author wolfgar
 * \brief turn on/off screen power consumption saving
 */

#include <barcelona/Barc_pwm.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pwm.h"

#define SYS_PATH_BRIGHTNESS "/sys/class/backlight/s3c/brightness"
#define SYS_PATH_POWER  "/sys/class/backlight/s3c/power"

#define PWM_DEFAULT_LIGHT (PWM_BACKLIGHT_MAX - 20)
static  int previous_setting =  PWM_DEFAULT_LIGHT;


/**
 * \fn int pwm_off(void)
 * \brief Turn OFF the screen and save the current state to be able to restore
 *
 * \return
 */
int pwm_off(void) {
	int fd, fd_pow;
	int res = 0;
	bool is_sys_needed = false;

	fd = open("/dev/" PWM_DEVNAME, O_RDWR);
	if (fd < 0){
	    perror("Error while trying to open PWM module ");
	    /* Try to open the sys entry instead of the pwn driver (for new TT kernel) */
	    fd = open(SYS_PATH_BRIGHTNESS , O_RDWR);
	    if (fd >= 0 ){
	    	is_sys_needed = true;
	    } else {
	    	return -1;
	    }
	}
	if (is_sys_needed == false){
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
	} else {
		char buffer[128];
		if (read(fd,buffer,sizeof(buffer)) <= 0){
			perror("Error while reading sys entry");
		    res = -1;
		    goto out_pwm_off;
		}
		previous_setting = strtol(buffer,NULL,10);
		if ((previous_setting <= 0) || (previous_setting > PWM_BACKLIGHT_MAX)){
			previous_setting = PWM_DEFAULT_LIGHT;
		}
		fd_pow = open(SYS_PATH_POWER , O_RDWR);
		if (fd_pow <0){
			perror("Error while turning OFF the screen  ");
			res = -1;
			goto out_pwm_off;
		} else{
			write(fd_pow,"1",1);
			close(fd_pow);
		}
	}

out_pwm_off:
	close(fd);
	return res;
}


/**
 * \fn int pwm_resume(void)
 * \brief Restore screen to its previous state
 *
 * \return
 */
int pwm_resume(void) {
	int fd, fd_pow;
	int res = 0;
	int current_val;
	bool is_sys_needed = false;

	fd = open("/dev/" PWM_DEVNAME, O_RDWR);
	if (fd < 0){
	    perror("Error while trying to open PWM module ");
	    /* Try to open the sys entry instead of the pwn driver (for new TT kernel) */
	    fd = open(SYS_PATH_BRIGHTNESS , O_RDWR);
  	    if (fd >= 0 ){
  	    	is_sys_needed = true;
  	    } else {
  	    	return -1;
  	    }
	}
	if (is_sys_needed == false){
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
	} else {
		fd_pow = open(SYS_PATH_POWER , O_RDWR);
		if (fd_pow <0){
			perror("Error while opening power sys  ");
			res = -1;
			goto out_pwm_resume;
		} else{
			char buffer[128];
			int power_state;

			if (read(fd_pow,buffer,sizeof(buffer)) <= 0){
				perror("Error while reading power sys entry");
			    res = -1;
			    goto out_pwm_resume;
			}
			power_state = strtol(buffer,NULL,10);
			/* Turn ON screen only if it is OFF */
			if (power_state == 1){
				write(fd_pow,"0",1);
				snprintf(buffer,sizeof(buffer),"%i",previous_setting);
				write(fd,buffer,sizeof(buffer));
			}
			close(fd_pow);
		}
	}

out_pwm_resume:
	close(fd);
	return res;
}


