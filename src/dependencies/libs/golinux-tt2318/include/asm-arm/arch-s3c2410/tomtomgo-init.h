/* include/asm/arch/tomtomgo-init.h
 *
 * Special initialization for TomTom GO.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_TOMTOMGO_INIT_H
#define __ASM_ARCH_TOMTOMGO_INIT_H

static inline void tomtomgo_backlight_off(void)
{
	unsigned gpbdat = *((volatile unsigned *) 0x56000014);
	gpbdat &= ~(0x1 << 0);
	*((volatile unsigned *) 0x56000014) = gpbdat;
}

static inline void tomtomgo_backlight_on(void)
{
	unsigned gpbdat = *((volatile unsigned *) 0x56000014);
	gpbdat |= (0x1 << 0);
	*((volatile unsigned *) 0x56000014) = gpbdat;
}

void tomtomgo_init(void);

#endif /* __ASM_ARCH_TOMTOMGO_INIT_H */

/* EOF */
