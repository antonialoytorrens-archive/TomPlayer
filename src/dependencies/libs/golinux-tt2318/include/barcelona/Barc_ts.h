/* include/barcelona/Barc_ts.h
 *
 * Public interface for the touchscreen driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_TS_H
#define __INCLUDE_BARCELONA_BARC_TS_H

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TS_DEVNAME					"ts"
#define TS_MAJOR					254

typedef struct {
	short pressure;
	short x;
	short y;
	short pad;
} TS_EVENT;

typedef struct {
	long An;		/* A = An/Divider */
	long Bn;		/* B = Bn/Divider */
	long Cn;		/* C = Cn/Divider */
	long Dn;		/* D = Dn/Divider */
	long En;		/* E = En/Divider */
	long Fn;		/* F = Fn/Divider */
	long Divider;
	int xMin;		// Stored on FLASH address 0x10c
	int xMax;		// Stored on FLASH address 0x110
	int yMin;		// Stored on FLASH address 0x104
	int yMax;		// Stored on FLASH address 0x108
} MATRIX;

#define TS_DRIVER_MAGIC		'f'
#define TS_GET_CAL		_IOR(TS_DRIVER_MAGIC, 10, MATRIX)
#define TS_SET_CAL		_IOW(TS_DRIVER_MAGIC, 11, MATRIX)
#define TS_SET_RAW_ON		_IO(TS_DRIVER_MAGIC, 14)
#define TS_SET_RAW_OFF		_IO(TS_DRIVER_MAGIC, 15)
#define TS_ENABLE		_IO(TS_DRIVER_MAGIC, 16)
#define TS_DISABLE		_IO(TS_DRIVER_MAGIC, 17)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_TS_H */

/* EOF */
