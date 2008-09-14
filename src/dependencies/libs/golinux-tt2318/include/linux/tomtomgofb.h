/* include/linux/tomtomgofb.h
 *
 * Definitions for TomTom GO framebuffer devices.
 *
 * Copyright (C) 2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_LINUX_TOMTOMGOFB_H
#define __INCLUDE_LINUX_TOMTOMGOFB_H

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TOMTOMGOFBIO_TYPE		't'
#define TOMTOMGOFBIO_BLIT		_IO(TOMTOMGOFBIO_TYPE, 1)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_LINUX_TOMTOMGOFB_H */

/* EOF */
