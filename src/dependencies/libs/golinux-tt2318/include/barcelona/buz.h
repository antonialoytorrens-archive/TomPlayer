/* include/barcelona/buz.h
 *
 * Public interface for the buzzer driver.
 *
 * Copyright (C) 2006 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BUZ_H
#define __INCLUDE_BARCELONA_BUZ_H

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BUZ_DEVNAME			"buz"
#define BUZ_MAJOR			252

#define BUZ_DRIVER_MAGIC		'U'
#define IOW_BUZZER_OFF			_IO(BUZ_DRIVER_MAGIC, 2)
#define IOW_BUZZER_ON			_IO(BUZ_DRIVER_MAGIC, 3)
#define IOW_BUZ_SETFREQ			_IOW(BUZ_DRIVER_MAGIC, 9, signed long int)
#define IOW_BUZ_SETDUTYCYCLE		_IOW(BUZ_DRIVER_MAGIC, 10, signed long int)

#define BUZ_DEFFREQ_NOM			11000000
#define BUZ_FREQUENCY_MIN		336
#define BUZ_FREQUENCY_MAX		5500000
#define BUZ_FREQUENCY_DEFAULT		2730
#define BUZ_DEFAULT_DUTYCYCLE		50

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BUZ_H */

/* EOF */
