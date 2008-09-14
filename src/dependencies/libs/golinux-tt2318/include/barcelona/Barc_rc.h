/* include/barcelona/Barc_rc.h
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

#ifndef __INCLUDE_BARCELONA_BARC_RC_H
#define __INCLUDE_BARCELONA_BARC_RC_H

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define RC_DEVNAME					"rc"
#define RC_MAJOR					122

typedef struct {
	unsigned int id;
	unsigned char batteryStatus;
	unsigned char keycode;
} RC_EVENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_RC_H */

/* EOF */
