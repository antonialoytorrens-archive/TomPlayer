/* include/barcelona/Barc_photosensor.h
 *
 * Public interface for the photosensor driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Jeroen Taverne <jeroen.taverne@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_PHOTOSENSOR_H
#define __INCLUDE_BARCELONA_BARC_PHOTOSESNOR_H

#ifndef __INCLUDE_BARCELONA_TYPES_H
#include <barcelona/types.h>
#endif /* __INCLUDE_BARCELONA_TYPES_H */

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PHOTOSENSOR_DEVNAME					"photosensor"
#define PHOTOSENSOR_MAJOR					123

typedef struct {
	UINT32 level;				/* photosensor level */
} PHOTOSENSOR_DATA;

#define PHOTOSENSOR_DRIVER_MAGIC			'A'

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_PHOTOSENSOR_H */

/* EOF */
