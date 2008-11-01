/***************************************************************************
 *
 *Power related functions to :
 *    - Probe power Off button
 * 	  - Get Battery informations
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
 * \file power.c
 * \author wolfgar
 * \power utilities functions
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <barcelona/Barc_Battery.h>

#include "power.h"
#include "engine.h"

#define BAT_DEV_NAME "/dev/battery"
#define BUTTON_NAME "/proc/barcelona/onoff"
#define DEVICE_NOT_OPENED -2
#define BUTTON_PUSHED_MASK 1

static int bat_fd = DEVICE_NOT_OPENED;

static uint16_t power_step[]= {3900, 3800, 3700};

/**
 * \fn bool power_is_off_button_pushed(void)
 * \brief Check whether the power button has been pushed
 *
 * \return true if the power button is pushed, else false
 */
bool power_is_off_button_pushed(void){
	bool is_power_off_pushed;
	unsigned char buffer[128];
	unsigned int val;
	int input_fd = DEVICE_NOT_OPENED;

	/* Check whether power button has been pushed */
	is_power_off_pushed = false;

	/* Proc entry has to be open each time it is used ...*/
	input_fd = open(BUTTON_NAME, O_RDWR );
	if (input_fd < 0){
	    //perror("Error while trying to open button device : ");
	}

	if (input_fd >= 0){
		if (read(input_fd,buffer,sizeof(buffer)) <= 0){
				perror("Error while reading proc onoff entry");
		} else {
			val = strtol(buffer,NULL,10);
			if (val & BUTTON_PUSHED_MASK){
				is_power_off_pushed = true;
				write(input_fd,"0",1);
			}
		}
                close (input_fd);
	}
	
	return is_power_off_pushed;
}


/**
 * \fn enum E_POWER_LEVEL power_get_bat_state(void)
 * \brief Get current battery state
 *
 * \return the power level
 */
enum E_POWER_LEVEL power_get_bat_state(void){
	enum E_POWER_LEVEL state;
	BATTERY_STATUS bat_status;
	int i;

	/* Get battery state */
	state = POWER_BAT_UNKNOWN;
	if (bat_fd == DEVICE_NOT_OPENED){
		/* If device not yet opened, then try to */
		bat_fd = open(BAT_DEV_NAME, O_RDWR);
		if (bat_fd < 0){
		    perror("Error while trying to open battery device : ");
		}
	}
	if (bat_fd >= 0){
		 if (ioctl (bat_fd, IOR_BATTERY_STATUS, &bat_status) == 0){
			 if (bat_status.u8ChargeStatus != 0){
				 state = POWER_BAT_PLUG;
			 } else {
				 state = POWER_BAT_100;
				 i = 0;
				 while ((power_step[i] > bat_status.u16BatteryVoltage) &&
						 (state < POWER_BAT_WARN))
				 {
					 i++;
					 state++;
				 }
			 }
		 } else {
			 perror ("Error while trying to get battery status ");
		 }
	}

	return state;
}
