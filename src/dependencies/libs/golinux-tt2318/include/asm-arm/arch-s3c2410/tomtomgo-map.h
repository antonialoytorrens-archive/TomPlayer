/* include/asm-arm/arch-s3c2410/tomtomgo-map.h
 *
 * Memory map defitions for TomTom GO.
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* You need to include arch/map.h before this file. */

#ifndef __ASM_ARCH_TOMTOMGOMAP_H
#define __ASM_ARCH_TOMTOMGOMAP_H

/* IDE interface on newer models. */

#define TOMTOMGO_VA_IDE		S3C2410_ADDR(0x01300000)
#define TOMTOMGO_PA_IDE		S3C2410_CS3
#define TOMTOMGO_SZ_IDE		SZ_1M



/* External UART definition on Murcia and Newcastle models  */

#define TOMTOMGO_PA_MURCIA_EXTERNAL_UART      S3C2410_CS5
#define TOMTOMGO_PA_NEWCASTLE_EXTERNAL_UART1  S3C2410_CS3
#define TOMTOMGO_PA_NEWCASTLE_EXTERNAL_UART2  S3C2410_CS4
#define TOMTOMGO_SZ_EXTERNAL_UART		          SZ_1M

#endif /* __ASM_ARCH_TOMTOMGOMAP_H */

/* EOF */
