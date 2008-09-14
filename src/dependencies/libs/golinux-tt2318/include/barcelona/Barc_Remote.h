/* include/barcelona/Barc_Remote.h
 *
 * Public interface for the remote control driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_REMOTE_H
#define __INCLUDE_BARCELONA_BARC_REMOTE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define REMOTE_DEVNAME					"remote"
#define REMOTE_MAJOR					122

typedef struct {
	unsigned id;
	unsigned code;
	unsigned batteryStatus;
} REMOTE_EVENT;

/* For REMOTE_EVENT::batteryStatus */
#define REMOTE_BATTERY_LOW				0
#define REMOTE_BATTERY_OK				1

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_REMOTE_H */

/* EOF */
