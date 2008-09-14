/* linux/include/asm/arch-s3c2410/tomtomgo-ide.h
 *
 * S3C2410 Barcelona IDE register definitions
 *
 * Copyright (C) 2004,2005 TomTom BV <http://www.tomtom.com/>
 * Author: Koen Martens <kmartens@sonologic.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Changelog:
 *    29-Nov-2004 Koen Martens      Created initial file
*/

#ifndef __ASM_ARM_TOMTOMGO_IDE
#define __ASM_ARM_TOMTOMGO_IDE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TOMTOMGO_IDEREG(x)       ((x) + TOMTOMGO_VA_IDE)

#define TOMTOMGO_IDEDATAREAD     TOMTOMGO_IDEREG(((0*4)+0))
#define TOMTOMGO_IDEDATAWRITE    TOMTOMGO_IDEREG(((0*4)+0))
#define TOMTOMGO_IDEERROR        TOMTOMGO_IDEREG(((1*4)+0))
#define TOMTOMGO_IDEFEATURES     TOMTOMGO_IDEREG(((1*4)+0))
#define TOMTOMGO_IDESECTORCOUNT  TOMTOMGO_IDEREG(((2*4)+0))
#define TOMTOMGO_IDESECTORNUMBER TOMTOMGO_IDEREG(((3*4)+0))
#define TOMTOMGO_IDECYLINDERLOW  TOMTOMGO_IDEREG(((4*4)+0))
#define TOMTOMGO_IDECYLINDERHIGH TOMTOMGO_IDEREG(((5*4)+0))
#define TOMTOMGO_IDEDEVICEHEAD   TOMTOMGO_IDEREG(((6*4)+0))
#define TOMTOMGO_IDESTATUS       TOMTOMGO_IDEREG(((7*4)+0))
#define TOMTOMGO_IDECOMMAND      TOMTOMGO_IDEREG(((7*4)+0))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ASM_ARM_TOMTOMGO_IDE */

/* EOF */
