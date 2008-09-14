/* include/barcelona/Barc_pwm.h
 *
 * Public interface for the pwm driver.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_BARC_PWM_H
#define __INCLUDE_BARCELONA_BARC_PWM_H

#include <linux/ioctl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PWM_DEVNAME					"pwm"
#define PWM_MAJOR					251

#define PWM_DRIVER_MAGIC			'T'
#define IOW_BACKLIGHT_OFF			_IO(PWM_DRIVER_MAGIC, 2)
#define IOW_BACKLIGHT_ON			_IO(PWM_DRIVER_MAGIC, 3)
#define IOW_BACKLIGHT_UPDATE		_IOW(PWM_DRIVER_MAGIC, 9, unsigned)
#define IOR_BACKLIGHT_CURRENT		_IOR(PWM_DRIVER_MAGIC, 11, unsigned)
#define IOR_LIGHTSENSOR_CURRENT		_IOR(PWM_DRIVER_MAGIC, 12, unsigned)

#define PWM_BACKLIGHT_MIN			0
#define PWM_BACKLIGHT_MAX			100

#ifdef __KERNEL__
void pwm_restore_level(void);
void pwm_disable(void);
#endif /* __KERNEL__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_BARC_PWM_H */

/* EOF */
