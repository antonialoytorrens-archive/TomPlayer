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
 * \file power.h
 * \author wolfgar
 * \brief power utilities functions
 */

#include <stdbool.h>

#ifndef POWER_H_
#define POWER_H_

/**
 * \enum E_POWER_LEVEL
 */
enum E_POWER_LEVEL {
      POWER_BAT_PLUG, /*!< TomTom has an alternative power source */
	  POWER_BAT_100,  /*!< Battery is full */
	  POWER_BAT_66,   /*!< 66% full */
	  POWER_BAT_33,	  /*!< 33% full */
	  POWER_BAT_WARN, /*<! Warning low battery */
	  POWER_BAT_UNKNOWN /*!<unknown state */
} ;

bool power_is_off_button_pushed(void);
enum E_POWER_LEVEL power_get_bat_state(void);

#endif /*POWER_H_*/
