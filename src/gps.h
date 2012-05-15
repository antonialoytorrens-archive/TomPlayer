/**
 * \file gps.h 
 * \brief  This module handles GPS to extract useful information from SiRF geodetic message 
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
#ifndef __GPS_H__
#define __GPS_H__

#include <time.h>

struct gps_data{
    int seq;    /**< sequence number (added by GPS module to detect refreshed data) */
    short int lat_deg;
    unsigned short int lat_mins;
    short int long_deg;
    unsigned short int long_mins;
    unsigned int alt_cm;
    unsigned int sat_id_list;
    unsigned int sat_nb;
    unsigned short int speed_kmh;
    struct tm time;
};

int gps_init (void);
int gps_update(void);
int gps_get_data(struct gps_data *);

#endif
