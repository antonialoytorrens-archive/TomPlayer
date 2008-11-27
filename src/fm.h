/**
 * \file fm.h
 * \author Wolfgar 
 * \brief This module provides functions related to FM transmitter
 * 
 * $URL$
 * $Rev$
 * $Author:$
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


#ifndef __FM_H__
#define __FM_H__ 

#include <stdbool.h>

bool fm_set_state(unsigned int state);
bool fm_set_freq(unsigned int freq);
bool fm_set_stereo(unsigned int stereo);
bool fm_set_power(unsigned char power);

#endif
