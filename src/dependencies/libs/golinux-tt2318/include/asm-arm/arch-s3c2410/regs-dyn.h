/* include/asm-arm/arch-s3c2410/regs-dyn.h
 *
 * S3C24xx dynamic register declarations.
 *
 * Copyright (C) 2006 TomTom BV <http://www.tomtom.com/>
 * Author: Dimitry Andric <dimitry.andric@tomtom.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_ARCH_REGS_DYN_H
#define __ASM_ARCH_REGS_DYN_H

#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <barcelona/gopins.h>

extern unsigned s3c24xx_sdidat;
extern unsigned s3c24xx_sdiimsk;

#ifdef CONFIG_CPU_S3C2412

typedef void __iomem *s3c24xx_reg_t;

extern s3c24xx_reg_t s3c24xx_misccr; 
extern s3c24xx_reg_t s3c24xx_dclkcon;
extern s3c24xx_reg_t s3c24xx_mslcon; 
extern s3c24xx_reg_t s3c24xx_extint0;
extern s3c24xx_reg_t s3c24xx_extint1;
extern s3c24xx_reg_t s3c24xx_extint2;
extern s3c24xx_reg_t s3c24xx_einflt0;
extern s3c24xx_reg_t s3c24xx_einflt1;
extern s3c24xx_reg_t s3c24xx_einflt2;
extern s3c24xx_reg_t s3c24xx_einflt3;
extern s3c24xx_reg_t s3c24xx_eintmask;
extern s3c24xx_reg_t s3c24xx_eintpend;
extern s3c24xx_reg_t s3c24xx_gstatus0;
extern s3c24xx_reg_t s3c24xx_gstatus1;
extern s3c24xx_reg_t s3c24xx_gstatus2;
extern s3c24xx_reg_t s3c24xx_gstatus3;
extern s3c24xx_reg_t s3c24xx_gstatus4;
extern s3c24xx_reg_t s3c24xx_gstatus5;

extern int s3c24xx_eintofs;

static inline int s3c24xx_is_2412(void) { return s3c24xx_gstatus5 != NULL; }

void s3c2412_setregs(void);

#else /* CONFIG_CPU_S3C2412 */

#define s3c24xx_misccr   S3C2410_MISCCR
#define s3c24xx_dclkcon  S3C2410_DCLKCON
#define s3c24xx_mslcon   S3C2410_MSLCON
#define s3c24xx_extint0  S3C2410_EXTINT0
#define s3c24xx_extint1  S3C2410_EXTINT1
#define s3c24xx_extint2  S3C2410_EXTINT2
#define s3c24xx_einflt0  S3C2410_EINFLT0
#define s3c24xx_einflt1  S3C2410_EINFLT1
#define s3c24xx_einflt2  S3C2410_EINFLT2
#define s3c24xx_einflt3  S3C2410_EINFLT3
#define s3c24xx_eintmask S3C2410_EINTMASK
#define s3c24xx_eintpend S3C2410_EINTPEND
#define s3c24xx_gstatus0 S3C2410_GSTATUS0
#define s3c24xx_gstatus1 S3C2410_GSTATUS1
#define s3c24xx_gstatus2 S3C2410_GSTATUS2
#define s3c24xx_gstatus3 S3C2410_GSTATUS3
#define s3c24xx_gstatus4 S3C2410_GSTATUS4

#endif /* CONFIG_CPU_S3C2412 */

#ifdef CONFIG_CPU_S3C2443
unsigned s3c2443_read_gpacon(void);
unsigned s3c2443_read_gpadat(void);
unsigned s3c2443_read_extint0(void);
unsigned s3c2443_read_extint1(void);
unsigned s3c2443_read_extint2(void);
unsigned s3c2443_read_einflt2(void);
unsigned s3c2443_read_einflt3(void);

static inline unsigned s3c24xx_read_gpacon(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_gpacon() : __raw_readl(S3C2410_GPACON);
}

static inline unsigned s3c24xx_read_gpadat(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_gpadat() : __raw_readl(S3C2410_GPADAT);
}

static inline unsigned s3c24xx_read_extint0(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_extint0() : __raw_readl(s3c24xx_extint0);
}

static inline unsigned s3c24xx_read_extint1(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_extint1() : __raw_readl(s3c24xx_extint1);
}

static inline unsigned s3c24xx_read_extint2(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_extint2() : __raw_readl(s3c24xx_extint2);
}

static inline unsigned s3c24xx_read_einflt2(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_einflt2() : __raw_readl(s3c24xx_einflt2);
}

static inline unsigned s3c24xx_read_einflt3(void)
{
	return IO_GetCpuType() == GOCPU_S3C2443 ? s3c2443_read_einflt3() : __raw_readl(s3c24xx_einflt3);
}

#else /* CONFIG_CPU_S3C2443 */
static inline unsigned s3c24xx_read_gpacon(void)
{
	return __raw_readl(S3C2410_GPACON);
}

static inline unsigned s3c24xx_read_gpadat(void)
{
	return __raw_readl(S3C2410_GPADAT);
}

static inline unsigned s3c24xx_read_extint0(void)
{
	return __raw_readl(s3c24xx_extint0);
}

static inline unsigned s3c24xx_read_extint1(void)
{
	return __raw_readl(s3c24xx_extint1);
}

static inline unsigned s3c24xx_read_extint2(void)
{
	return __raw_readl(s3c24xx_extint2);
}

static inline unsigned s3c24xx_read_einflt2(void)
{
	return __raw_readl(s3c24xx_einflt2);
}

static inline unsigned s3c24xx_read_einflt3(void)
{
	return __raw_readl(s3c24xx_einflt3);
}
#endif /* CONFIG_CPU_S3C2443 */

#endif /* __ASM_ARCH_REGS_DYN_H */

/* EOF */
