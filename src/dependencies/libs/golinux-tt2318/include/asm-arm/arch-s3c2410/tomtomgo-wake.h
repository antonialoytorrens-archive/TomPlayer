/* include/asm-arm/arch-s3c2410/tomtomgo-wake.h
 *
 * Declarations of TomTom GO wakeup check functions.
 *
 * Copyright (C) 2005 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_TOMTOMGO_WAKE_H
#define __ASM_ARCH_TOMTOMGO_WAKE_H

#define S3C2412_INFORM0_PWRST 	0x0
#define S3C2412_INFORM0_NEUTRAL 0x1
#define S3C2412_INFORM0_OFFRST 	0x3
#define S3C2412_INFORM0_WDTRST 	0x5
#define S3C2412_INFORM0_SFTRST 	0x9
#define S3C2412_INFORM0_RST_MASK 0xf

#define GET_SLEEP_STATE(x) (x)
#define SET_SLEEP_STATE(x) (x)

#ifdef CONFIG_CPU_S3C2412
static inline void tomtomgo_reboot_2412(unsigned long inform0)
{
	extern void s3c2412_cpu_reboot(void);
	__raw_writel(inform0, S3C2412_INFORM0);
	s3c2412_cpu_reboot();
}
#endif /* CONFIG_CPU_S3C2412 */
#ifdef CONFIG_CPU_S3C2443
static inline void tomtomgo_reboot_2443(unsigned long inform0)
{
	extern void s3c2443_cpu_reboot(void);
	__raw_writel(inform0, S3C2443_INFORM0);
	s3c2443_cpu_reboot();
}
#endif /* CONFIG_CPU_S3C2443 */
void tomtomgo_before_sleep(int simulateWake,int linuxSuspend);
int tomtomgo_decide_wakeup(void);

void tomtom_hddtemp_setaddress(unsigned long address);
int tomtom_hddtemp_check(void);

#endif /* __ASM_ARCH_TOMTOMGO_WAKE_H */

/* EOF */
