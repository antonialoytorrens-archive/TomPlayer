/* include/barcelona/Barc_gps.h
 *
 * Public interface for the GPS driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_GPS_H
#define __INCLUDE_BARCELONA_BARC_GPS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GPS_DEVNAME					"gps"
#define GPS_MAJOR					243

#define GPS_DRIVER_MAGIC			'U'
#define IOW_GPS_ON					_IO(GPS_DRIVER_MAGIC, 1)
#define IOW_GPS_OFF					_IO(GPS_DRIVER_MAGIC, 2)
#define IOW_GPS_RESET				_IO(GPS_DRIVER_MAGIC, 3)
#define IOW_GPS_UPDATE_ON			_IO(GPS_DRIVER_MAGIC, 4)
#define IOW_GPS_UPDATE_OFF			_IO(GPS_DRIVER_MAGIC, 5)
#define IOR_GET_TIME_STAMP			_IO(GPS_DRIVER_MAGIC, 6)
#define IOW_GPS_PROGRAM_MODE	_IO(GPS_DRIVER_MAGIC, 7)
#define IOW_GPS_NORMAL_MODE		_IO(GPS_DRIVER_MAGIC, 8)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_GPS_H */

/* EOF */
