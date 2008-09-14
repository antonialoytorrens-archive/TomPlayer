/* linux/include/asm-arm/arch-s3c2410/system.h
 *
 * (c) 2003 Simtec Electronics
 *  Ben Dooks <ben@simtec.co.uk>
 *
 * S3C2410 - System function defines and includes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Changelog:
 *  12-May-2003 BJD  Created file
 *  14-May-2003 BJD  Removed idle to aid debugging
 *  12-Jun-2003 BJD  Added reset via watchdog
 *  04-Sep-2003 BJD  Moved to v2.6
 *  28-Oct-2004 BJD  Added over-ride for idle, and fixed reset panic()
 */

#ifndef __ASM_ARCH_S3C2410_SYSTEM_H
#define __ASM_ARCH_S3C2410_SYSTEM_H __FILE__

extern void arch_idle(void);
extern void arch_reset(char mode);

#endif /* __ASM_ARCH_S3C2410_SYSTEM_H */

/* EOF */
