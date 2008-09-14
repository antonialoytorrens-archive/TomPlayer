/* include/barcelona/Barc_acc.h
 *
 * Public interface for the accelerometer driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_ACC_H
#define __INCLUDE_BARCELONA_BARC_ACC_H

#ifndef __INCLUDE_BARCELONA_TYPES_H
#include <barcelona/types.h>
#endif /* __INCLUDE_BARCELONA_TYPES_H */

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ACC_DEVNAME					"acc"
#define ACC_MAJOR					120

typedef struct {
	UINT32 u32Gyro;					/* gyro data */
	UINT32 u32xData;				/* x axis data */
	UINT32 u32yData;				/* y axis data */
	UINT32 u32zData;				/* z axis data */
	UINT32 u32sTimeStamp;			/* timestamp */
	UINT32 u32usTimeStamp;			/* microseconds */
	UINT32 u32Temperature;			/* temperature */
} ACCMETER_DATA;

#define ACC_DRIVER_MAGIC			'A'
#define IOR_FIFOFILLED_SIZE			_IOR(ACC_DRIVER_MAGIC, 0, unsigned)		/* UNSUPPORTED */
#define IOW_ACC_SAMPLINGRATE		_IOW(ACC_DRIVER_MAGIC, 1, unsigned)

/* For IOW_ACC_SAMPLINGRATE ioctl */
#define ACC_RATE_MIN				1
#define ACC_RATE_DEF				25
#define ACC_RATE_MAX				100

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_ACC_H */

/* EOF */
