/* include/asm-arm/arch-s3c2410/tomtomgo-irq.h
 *
 * IRQ number definitions for TomTom GO.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* You need to include arch/irqs.h before this file. */

#ifndef __ASM_ARCH_TOMTOMGOIRQ_H
#define __ASM_ARCH_TOMTOMGOIRQ_H

/* IRQ numbers of TomTom GO peripherals */

#define TOMTOMGO_IRQ_ONOFF		IRQ_EINT0
#define TOMTOMGO_IRQ_GPS_PPS	IRQ_EINT8
#define TOMTOMGO_IRQ_RC			IRQ_EINT16
#define TOMTOMGO_IRQ_IDE		IRQ_EINT19

#endif /* __ASM_ARCH_TOMTOMGOIRQ_H */

/* EOF */
