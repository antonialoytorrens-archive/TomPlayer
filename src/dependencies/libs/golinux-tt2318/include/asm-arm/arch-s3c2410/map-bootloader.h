/* linux/include/asm-arm/arch-s3c2410/map-bootloader.h
 *
 * (c) 2003 Simtec Electronics
 *  Ben Dooks <ben@simtec.co.uk>
 *
 * S3C2410 - Memory map definitions for the TomTom bootloader
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Changelog:
 *  12-May-2003 BJD   Created file
 *  06-Jan-2003 BJD   Linux 2.6.0 version, moved bast specifics out
 *  10-Feb-2005 BJD   Added CAMIF definition from guillaume.gourat@nexvision.tv
 *  10-Mar-2005 LCVR  Added support to S3C2400, changed {VA,SZ} names
 *  21-Feb-2006 KM    Copied original file to map-bootloader.h for new TomTom GO
*/

#ifndef __ASM_ARCH_MAP_BOOTLOADER_H
#define __ASM_ARCH_MAP_BOOTLOADER_H

#include <asm/arch/map.h>

/*
 * We simply include the kernel-space map.h, and redefine the
 * virtual addresses to be the s3c2410 physical addresses
 */

#undef S3C2410_ADDR

#define S3C2410_ADDR(x)	  (x)

/* interrupt controller is the first thing we put in, to make
 * the assembly code for the irq detection easier
 */
#undef S3C24XX_VA_IRQ
#define S3C24XX_VA_IRQ	   S3C2410_PA_IRQ

/* memory controller registers */
#undef S3C24XX_VA_MEMCTRL
#define S3C24XX_VA_MEMCTRL S3C2410_PA_MEMCTRL

/* USB host controller */
#undef S3C24XX_VA_USBHOST
#define S3C24XX_VA_USBHOST S3C2410_PA_USBHOST

/* DMA controller */
#undef S3C24XX_VA_DMA
#define S3C24XX_VA_DMA	   S3C2410_PA_DMA

/* Clock and Power management */
#undef S3C24XX_VA_CLKPWR
#define S3C24XX_VA_CLKPWR  S3C2410_PA_CLKPWR

/* LCD controller */
#undef S3C24XX_VA_LCD
#define S3C24XX_VA_LCD	   S3C2410_PA_LCD

/* NAND flash controller */
#undef S3C24XX_VA_NAND
#define S3C24XX_VA_NAND	   S3C2410_PA_NAND

/* UARTs */
#undef S3C24XX_VA_UART
#define S3C24XX_VA_UART	   S3C2410_PA_UART

/* Timers */
#undef S3C24XX_VA_TIMER
#define S3C24XX_VA_TIMER   S3C2410_PA_TIMER

/* USB Device port */
#undef S3C24XX_VA_USBDEV
#define S3C24XX_VA_USBDEV  S3C2410_PA_USBDEV

/* Watchdog */
#undef S3C24XX_VA_WATCHDOG
#define S3C24XX_VA_WATCHDOG S3C2410_PA_WATCHDOG

/* IIC hardware controller */
#undef S3C24XX_VA_IIC
#define S3C24XX_VA_IIC	   S3C2410_PA_IIC

#undef VA_IIC_BASE
#define VA_IIC_BASE	   (S3C24XX_VA_IIC)

/* IIS controller */
#undef S3C24XX_VA_IIS
#define S3C24XX_VA_IIS	   S3C2410_PA_IIS

/* GPIO ports */
#undef S3C24XX_VA_GPIO
#define S3C24XX_VA_GPIO	   S3C2410_PA_GPIO

/* RTC */
#undef S3C24XX_VA_RTC
#define S3C24XX_VA_RTC	   S3C2410_PA_RTC

/* ADC */
#undef S3C24XX_VA_ADC
#define S3C24XX_VA_ADC	   S3C2410_PA_ADC

/* SPI */
#undef S3C24XX_VA_SPI
#define S3C24XX_VA_SPI	   S3C2410_PA_SPI

/* SDI */
#undef S3C24XX_VA_SDI
#define S3C24XX_VA_SDI	   S3C2410_PA_SDI

#endif /* __ASM_ARCH_MAP_BOOTLOADER_H */
