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

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <barcelona/Barc_Battery.h> 

#include "power.h"

#define BAT_DEV_NAME "/dev/battery"
#define INPUT_DEV_NAME "/dev/input/event0"
#define DEVICE_NOT_OPENED -2
static int bat_fd = DEVICE_NOT_OPENED;
static int input_fd = DEVICE_NOT_OPENED;

static uint16_t power_step[]= {3900, 3800, 3700}; 

/** Check whether the power button has been pushed and return battery informations  
 * 
 * \param states[out] the output struct with requested informations
 * 
 */
void power_get_states(struct power_states * states){
	int i;
	BATTERY_STATUS bat_status;
	struct input_event event;
	
	/* Get battery state */
	states->battery_state = POWER_BAT_UNKNOWN;
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
				 states->battery_state = POWER_BAT_PLUG;
			 } else {
				 states->battery_state = POWER_BAT_100;
				 i = 0;
				 while ((power_step[i] > bat_status.u16BatteryVoltage) && 
						 (states->battery_state < POWER_BAT_WARN))
				 {
					 i++;
				 }				 
			 }
		 } else {
			 perror ("Error while trying to get battery status ");
		 }
	}
	
	/* Check whether power button has been pushed */
	states->is_power_off_pushed = false;
	if (input_fd == DEVICE_NOT_OPENED){
			/* If device not yet opened, then try to */
			input_fd = open(INPUT_DEV_NAME, O_RDWR | O_NONBLOCK );
			if (input_fd < 0){
			    perror("Error while trying to open input device : ");
			    return ;
			} 	
	} 
	if (input_fd >= 0){			
		while(read(input_fd, &event, sizeof(event)) > 0) {			
			/* Power button has been released */
			if ((event.type == EV_KEY) &&
				(event.code == KEY_POWER) && 
				(event.value == 0)){
				states->is_power_off_pushed = true;
			}
		}
	}

	return;
}
