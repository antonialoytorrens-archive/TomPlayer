/* include/barcelona/timer.h
 *
 * Timer related definitions for TomTom GO.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __INCLUDE_BARCELONA_TIMER_H
#define __INCLUDE_BARCELONA_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Define prescalers and dividers for the timers we use. */
#define SYS_TIMER01_PRESCALER	((u32)0)	/* for Touch Screen and backlight */
#define SYS_TIMER234_PRESCALER	((u32)2)
#define SYS_TIMER0_MUX			((u32)0)	/* 1/2 */
#define SYS_TIMER0_DIVIDER		((u32)2)
#define SYS_TIMER1_MUX			((u32)0)	/* 1/2 */
#define SYS_TIMER1_DIVIDER		((u32)2)
#define SYS_TIMER4_MUX			((u32)0)	/* 1/2 */
#define SYS_TIMER4_DIVIDER		((u32)2)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCLUDE_BARCELONA_TIMER_H */

/* EOF */
