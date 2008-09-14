/* linux/include/asm/arch-s3c2410/regs-irq.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *
 *  Changelog:
 *    19-06-2003     BJD     Created file
 *    12-03-2004     BJD     Updated include protection
 *    10-03-2005     LCVR    Changed S3C2410_VA to S3C24XX_VA
 *    17-02-2006     KM      Added S3C2412/S3C2413 defines
 */


#ifndef ___ASM_ARCH_REGS_IRQ_H
#define ___ASM_ARCH_REGS_IRQ_H "$Id: irq.h,v 1.3 2003/03/25 21:29:06 ben Exp $"

/* interrupt controller */

#define S3C2410_IRQREG(x)         ((x) + S3C24XX_VA_IRQ)
#define S3C2410_EINTREG(x)        ((x) + S3C24XX_VA_GPIO)

#define S3C2410_SRCPND	          S3C2410_IRQREG(0x000)
#define S3C2410_INTMOD	          S3C2410_IRQREG(0x004)
#define S3C2410_INTMSK	          S3C2410_IRQREG(0x008)
#define S3C2410_PRIORITY          S3C2410_IRQREG(0x00C)
#define S3C2410_INTPND	          S3C2410_IRQREG(0x010)
#define S3C2410_INTOFFSET         S3C2410_IRQREG(0x014)
#define S3C2410_SUBSRCPND         S3C2410_IRQREG(0x018)
#define S3C2410_INTSUBMSK         S3C2410_IRQREG(0x01C)

/* mask: 0=enable, 1=disable
 * 1 bit EINT, 4=EINT4, 23=EINT23
 * EINT0,1,2,3 are not handled here.
*/

#define S3C2410_EINTMASK          S3C2410_EINTREG(0x0A4)
#define S3C2410_EINTPEND          S3C2410_EINTREG(0X0A8)

#define S3C2410_SRCPND_EINT0      (1<<0)
#define S3C2410_SRCPND_EINT1      (1<<1)
#define S3C2410_SRCPND_EINT2      (1<<2)
#define S3C2410_SRCPND_EINT3      (1<<3)
#define S3C2410_SRCPND_EINT4_7    (1<<4)
#define S3C2410_SRCPND_EINT8_23   (1<<5)
#define S3C2410_SRCPND_RES_6      (1<<6)
#define S3C2410_SRCPND_nBATT_FLT  (1<<7)
#define S3C2410_SRCPND_TICK       (1<<8)
#define S3C2410_SRCPND_WDT        (1<<9)
#define S3C2410_SRCPND_TIMER0     (1<<10)
#define S3C2410_SRCPND_TIMER1     (1<<11)
#define S3C2410_SRCPND_TIMER2     (1<<12)
#define S3C2410_SRCPND_TIMER3     (1<<13)
#define S3C2410_SRCPND_TIMER4     (1<<14)
#define S3C2410_SRCPND_UART2      (1<<15)
#define S3C2410_SRCPND_LCD        (1<<16)
#define S3C2410_SRCPND_DMA0       (1<<17)
#define S3C2410_SRCPND_DMA1       (1<<18)
#define S3C2410_SRCPND_DMA2       (1<<19)
#define S3C2410_SRCPND_DMA3       (1<<20)
#define S3C2410_SRCPND_SDI        (1<<21)
#define S3C2410_SRCPND_SPI0       (1<<22)
#define S3C2410_SRCPND_UART1      (1<<23)
#define S3C2410_SRCPND_RES_24     (1<<24)
#define S3C2410_SRCPND_USBD       (1<<25)
#define S3C2410_SRCPND_USBH       (1<<26)
#define S3C2410_SRCPND_IIC        (1<<27)
#define S3C2410_SRCPND_UART0      (1<<28)
#define S3C2410_SRCPND_SPI1       (1<<29)
#define S3C2410_SRCPND_RTC        (1<<30)
#define S3C2410_SRCPND_ADC        (1<<31)

#define S3C2410_SUBSRCPND_RXD0    (1<<0)
#define S3C2410_SUBSRCPND_TXD0    (1<<1)
#define S3C2410_SUBSRCPND_ERR0    (1<<2)
#define S3C2410_SUBSRCPND_RXD1    (1<<3)
#define S3C2410_SUBSRCPND_TXD1    (1<<4)
#define S3C2410_SUBSRCPND_ERR1    (1<<5)
#define S3C2410_SUBSRCPND_RXD2    (1<<6)
#define S3C2410_SUBSRCPND_TXD2    (1<<7)
#define S3C2410_SUBSRCPND_ERR2    (1<<8)
#define S3C2410_SUBSRCPND_TC      (1<<9)
#define S3C2410_SUBSRCPND_ADC     (1<<10)

#ifdef CONFIG_CPU_S3C2440

#define S3C2440_SRCPND_EINT0      (1<<0)
#define S3C2440_SRCPND_EINT1      (1<<1)
#define S3C2440_SRCPND_EINT2      (1<<2)
#define S3C2440_SRCPND_EINT3      (1<<3)
#define S3C2440_SRCPND_EINT4_7    (1<<4)
#define S3C2440_SRCPND_EINT8_23   (1<<5)
#define S3C2440_SRCPND_INT_CAM    (1<<6)
#define S3C2440_SRCPND_nBATT_FLT  (1<<7)
#define S3C2440_SRCPND_TICK       (1<<8)
#define S3C2440_SRCPND_WDT_AC97   (1<<9)
#define S3C2440_SRCPND_TIMER0     (1<<10)
#define S3C2440_SRCPND_TIMER1     (1<<11)
#define S3C2440_SRCPND_TIMER2     (1<<12)
#define S3C2440_SRCPND_TIMER3     (1<<13)
#define S3C2440_SRCPND_TIMER4     (1<<14)
#define S3C2440_SRCPND_UART2      (1<<15)
#define S3C2440_SRCPND_LCD        (1<<16)
#define S3C2440_SRCPND_DMA0       (1<<17)
#define S3C2440_SRCPND_DMA1       (1<<18)
#define S3C2440_SRCPND_DMA2       (1<<19)
#define S3C2440_SRCPND_DMA3       (1<<20)
#define S3C2440_SRCPND_SDI        (1<<21)
#define S3C2440_SRCPND_SPI0       (1<<22)
#define S3C2440_SRCPND_UART1      (1<<23)
#define S3C2440_SRCPND_NFCON      (1<<24)
#define S3C2440_SRCPND_USBD       (1<<25)
#define S3C2440_SRCPND_USBH       (1<<26)
#define S3C2440_SRCPND_IIC        (1<<27)
#define S3C2440_SRCPND_UART0      (1<<28)
#define S3C2440_SRCPND_SPI1       (1<<29)
#define S3C2440_SRCPND_RTC        (1<<30)
#define S3C2440_SRCPND_ADC        (1<<31)

#define S3C2440_SUBSRCPND_RXD0    (1<<0)
#define S3C2440_SUBSRCPND_TXD0    (1<<1)
#define S3C2440_SUBSRCPND_ERR0    (1<<2)
#define S3C2440_SUBSRCPND_RXD1    (1<<3)
#define S3C2440_SUBSRCPND_TXD1    (1<<4)
#define S3C2440_SUBSRCPND_ERR1    (1<<5)
#define S3C2440_SUBSRCPND_RXD2    (1<<6)
#define S3C2440_SUBSRCPND_TXD2    (1<<7)
#define S3C2440_SUBSRCPND_ERR2    (1<<8)
#define S3C2440_SUBSRCPND_TC      (1<<9)
#define S3C2440_SUBSRCPND_ADC     (1<<10)
#define S3C2440_SUBSRCPND_CAM_C   (1<<11)
#define S3C2440_SUBSRCPND_CAM_P   (1<<12)
#define S3C2440_SUBSRCPND_WDT     (1<<13)
#define S3C2440_SUBSRCPND_AC97    (1<<14)

#endif /* CONFIG_CPU_S3C2440 */

#ifdef CONFIG_CPU_S3C2412

#define S3C2412_EINTMASK          S3C2410_EINTREG(0x0B4)
#define S3C2412_EINTPEND          S3C2410_EINTREG(0X0B8)

#define S3C2412_SRCPND_EINT0      (1<<0)
#define S3C2412_SRCPND_EINT1      (1<<1)
#define S3C2412_SRCPND_EINT2      (1<<2)
#define S3C2412_SRCPND_EINT3      (1<<3)
#define S3C2412_SRCPND_EINT4_7    (1<<4)
#define S3C2412_SRCPND_EINT8_23   (1<<5)
#define S3C2412_SRCPND_CAMIF      (1<<6)
#define S3C2412_SRCPND_nBATT_FLT  (1<<7)
#define S3C2412_SRCPND_TICK       (1<<8)
#define S3C2412_SRCPND_WDT        (1<<9)
#define S3C2412_SRCPND_TIMER0     (1<<10)
#define S3C2412_SRCPND_TIMER1     (1<<11)
#define S3C2412_SRCPND_TIMER2     (1<<12)
#define S3C2412_SRCPND_TIMER3     (1<<13)
#define S3C2412_SRCPND_TIMER4     (1<<14)
#define S3C2412_SRCPND_UART2      (1<<15)
#define S3C2412_SRCPND_LCD        (1<<16)
#define S3C2412_SRCPND_DMA0       (1<<17)
#define S3C2412_SRCPND_DMA1       (1<<18)
#define S3C2412_SRCPND_DMA2       (1<<19)
#define S3C2412_SRCPND_DMA3       (1<<20)
#define S3C2412_SRCPND_SDI_CF     (1<<21)
#define S3C2412_SRCPND_SPI0       (1<<22)
#define S3C2412_SRCPND_UART1      (1<<23)
#define S3C2412_SRCPND_NFCON      (1<<24)
#define S3C2412_SRCPND_USBD       (1<<25)
#define S3C2412_SRCPND_USBH       (1<<26)
#define S3C2412_SRCPND_IIC        (1<<27)
#define S3C2412_SRCPND_UART0      (1<<28)
#define S3C2412_SRCPND_SPI1_SPITO (1<<29)
#define S3C2412_SRCPND_RTC        (1<<30)
#define S3C2412_SRCPND_ADC        (1<<31)

#define S3C2412_SUBSRCPND_RXD0    (1<<0)
#define S3C2412_SUBSRCPND_TXD0    (1<<1)
#define S3C2412_SUBSRCPND_ERR0    (1<<2)
#define S3C2412_SUBSRCPND_RXD1    (1<<3)
#define S3C2412_SUBSRCPND_TXD1    (1<<4)
#define S3C2412_SUBSRCPND_ERR1    (1<<5)
#define S3C2412_SUBSRCPND_RXD2    (1<<6)
#define S3C2412_SUBSRCPND_TXD2    (1<<7)
#define S3C2412_SUBSRCPND_ERR2    (1<<8)
#define S3C2412_SUBSRCPND_TC      (1<<9)
#define S3C2412_SUBSRCPND_ADC     (1<<10)
#define S3C2412_SUBSRCPND_SPI0_TO (1<<11)
#define S3C2412_SUBSRCPND_SPI1_TO (1<<12)
#define S3C2412_SUBSRCPND_SDI     (1<<13)
#define S3C2412_SUBSRCPND_CF      (1<<14)
#define S3C2412_SUBSRCPND_SPI1    (1<<15)

#endif /* CONFIG_CPU_S3C2412 */

#ifdef CONFIG_CPU_S3C2443

/* Some IRQs got shuffled for 2443 */

#define S3C2443_SRCPND_EINT0      (1<<0)
#define S3C2443_SRCPND_EINT1      (1<<1)
#define S3C2443_SRCPND_EINT2      (1<<2)
#define S3C2443_SRCPND_EINT3      (1<<3)
#define S3C2443_SRCPND_EINT4_7    (1<<4)
#define S3C2443_SRCPND_EINT8_23   (1<<5)
#define S3C2443_SRCPND_CAM        (1<<6)
#define S3C2443_SRCPND_nBATT_FLT  (1<<7)
#define S3C2443_SRCPND_TICK       (1<<8)
#define S3C2443_SRCPND_WDT_AC97   (1<<9)
#define S3C2443_SRCPND_TIMER0     (1<<10)
#define S3C2443_SRCPND_TIMER1     (1<<11)
#define S3C2443_SRCPND_TIMER2     (1<<12)
#define S3C2443_SRCPND_TIMER3     (1<<13)
#define S3C2443_SRCPND_TIMER4     (1<<14)
#define S3C2443_SRCPND_UART2      (1<<15)
#define S3C2443_SRCPND_LCD        (1<<16)
#define S3C2443_SRCPND_DMA        (1<<17)
#define S3C2443_SRCPND_UART3      (1<<18)
#define S3C2443_SRCPND_CFCON      (1<<19)
#define S3C2443_SRCPND_SDI_1      (1<<20)
#define S3C2443_SRCPND_SDI_0      (1<<21)
#define S3C2443_SRCPND_SPI0       (1<<22)
#define S3C2443_SRCPND_UART1      (1<<23)
#define S3C2443_SRCPND_NAND       (1<<24)
#define S3C2443_SRCPND_USBD       (1<<25)
#define S3C2443_SRCPND_USBH       (1<<26)
#define S3C2443_SRCPND_IIC        (1<<27)
#define S3C2443_SRCPND_UART0      (1<<28)
#define S3C2443_SRCPND_SPI1       (1<<29)
#define S3C2443_SRCPND_RTC        (1<<30)
#define S3C2443_SRCPND_ADC        (1<<31)

#define S3C2443_SUBSRCPND_RXD0    (1<<0)
#define S3C2443_SUBSRCPND_TXD0    (1<<1)
#define S3C2443_SUBSRCPND_ERR0    (1<<2)
#define S3C2443_SUBSRCPND_RXD1    (1<<3)
#define S3C2443_SUBSRCPND_TXD1    (1<<4)
#define S3C2443_SUBSRCPND_ERR1    (1<<5)
#define S3C2443_SUBSRCPND_RXD2    (1<<6)
#define S3C2443_SUBSRCPND_TXD2    (1<<7)
#define S3C2443_SUBSRCPND_ERR2    (1<<8)
#define S3C2443_SUBSRCPND_TC      (1<<9)
#define S3C2443_SUBSRCPND_ADC     (1<<10)
#define S3C2443_SUBSRCPND_CAM_C   (1<<11)
#define S3C2443_SUBSRCPND_CAM_P   (1<<12)
#define S3C2443_SUBSRCPND_LCD1    (1<<14)
#define S3C2443_SUBSRCPND_LCD2    (1<<15)
#define S3C2443_SUBSRCPND_LCD3    (1<<16)
#define S3C2443_SUBSRCPND_LCD4    (1<<17)
#define S3C2443_SUBSRCPND_DMA0    (1<<18)
#define S3C2443_SUBSRCPND_DMA1    (1<<19)
#define S3C2443_SUBSRCPND_DMA2    (1<<20)
#define S3C2443_SUBSRCPND_DMA3    (1<<21)
#define S3C2443_SUBSRCPND_DMA4    (1<<22)
#define S3C2443_SUBSRCPND_DMA5    (1<<23)
#define S3C2443_SUBSRCPND_RXD3    (1<<24)
#define S3C2443_SUBSRCPND_TXD3    (1<<25)
#define S3C2443_SUBSRCPND_ERR3    (1<<26)
#define S3C2443_SUBSRCPND_WDT     (1<<27)
#define S3C2443_SUBSRCPND_AC97    (1<<28)

#endif /* CONFIG_CPU_S3C2443 */

#endif /* ___ASM_ARCH_REGS_IRQ_H */
