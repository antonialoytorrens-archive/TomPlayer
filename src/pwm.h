
 /***************************************************************************
 *  Enable to turn off/on screen for power consumption savings
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
 * \file pwm.h
 * \author wolfgar
 * \brief turn on/off screen power consumption saving
 */

#ifndef __TOMPLAYER_PWM_H__
#define __TOMPLAYER_PWM_H__

int pwm_off(void);
int pwm_resume(void);
int pwm_set_brightness(int val);
int pwm_get_brightness(int *val);

#endif
