/* include/barcelona/Barc_sd.h
 *
 * Public interface for the SD driver. 
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_SD_H
#define __INCLUDE_BARCELONA_BARC_SD_H

#ifndef COMPILING_SD_CARD_GO
#include <linux/types.h>
#include <linux/ioctl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SD_DEVNAME				"sdcard"
#define SD_MAJOR				250

#define SDCARD_ID_LEN				16

/* For IOR_SD_STATUS ioctl: */
#define SD_CARD_INSERTED		0x01
#define SD_CARD_REMOVED			0x02
#define SD_CARD_MASK			0x04

/* SD card driver ioctls */
#define SD_DRIVER_MAGIC			'S'
#define GET_SD_SERIAL_NUMBER	_IOR(SD_DRIVER_MAGIC, 0, unsigned char)
#define SDHOT_REG				_IOW(SD_DRIVER_MAGIC, 200, pid_t)
#define SDHOT_UNREG				_IOR(SD_DRIVER_MAGIC, 201, pid_t)
#define IOR_SD_STATUS			_IOR(SD_DRIVER_MAGIC, 202, unsigned)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_SD_H */

/* EOF */
