/* linux/include/asm/arch-s3c2410/regs-clock.h
 *
 * Copyright (c) 2003,2004 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 clock register definitions
 *
 *  Changelog:
 *    18-Aug-2004 Ben Dooks         Added 2440 definitions
 *    08-Aug-2004 Herbert Pötzl     Added CLKCON definitions
 *    19-06-2003  Ben Dooks         Created file
 *    12-03-2004  Ben Dooks         Updated include protection
 *    29-Sep-2004 Ben Dooks	    Fixed usage for assembly inclusion
 *    10-Feb-2005 Ben Dooks	    Fixed CAMDIVN address (Guillaume Gourat)
 *    10-Mar-2005 Lucas Villa Real  Changed S3C2410_VA to S3C24XX_VA
 *    28-Feb-2006 Koen Martens      Added s3c2412_get_pll
 */

#ifndef __ASM_ARM_REGS_CLOCK
#define __ASM_ARM_REGS_CLOCK "$Id: clock.h,v 1.4 2003/04/30 14:50:51 ben Exp $"

#define S3C2410_CLKREG(x) ((x) + S3C24XX_VA_CLKPWR)

#define S3C2410_PLLVAL(_m,_p,_s) ((_m) << 12 | ((_p) << 4) | ((_s)))

#define S3C2410_LOCKTIME    S3C2410_CLKREG(0x00)
#define S3C2410_MPLLCON	    S3C2410_CLKREG(0x04)
#define S3C2410_UPLLCON	    S3C2410_CLKREG(0x08)
#define S3C2410_CLKCON	    S3C2410_CLKREG(0x0C)
#define S3C2410_CLKSLOW	    S3C2410_CLKREG(0x10)
#define S3C2410_CLKDIVN	    S3C2410_CLKREG(0x14)

#define S3C2410_CLKCON_IDLE	     (1<<2)
#define S3C2410_CLKCON_POWER	     (1<<3)
#define S3C2410_CLKCON_NAND	     (1<<4)
#define S3C2410_CLKCON_LCDC	     (1<<5)
#define S3C2410_CLKCON_USBH	     (1<<6)
#define S3C2410_CLKCON_USBD	     (1<<7)
#define S3C2412_CLKCON_USBD	     (1<<16)
#define S3C2410_CLKCON_PWMT	     (1<<8)
#define S3C2410_CLKCON_SDI	     (1<<9)
#define S3C2410_CLKCON_UART0	     (1<<10)
#define S3C2410_CLKCON_UART1	     (1<<11)
#define S3C2410_CLKCON_UART2	     (1<<12)
#define S3C2410_CLKCON_GPIO	     (1<<13)
#define S3C2410_CLKCON_RTC	     (1<<14)
#define S3C2410_CLKCON_ADC	     (1<<15)
#define S3C2410_CLKCON_IIC	     (1<<16)
#define S3C2410_CLKCON_IIS	     (1<<17)
#define S3C2410_CLKCON_SPI	     (1<<18)

#define S3C2410_PLLCON_MDIVSHIFT     12
#define S3C2410_PLLCON_PDIVSHIFT     4
#define S3C2410_PLLCON_SDIVSHIFT     0
#define S3C2410_PLLCON_MDIVMASK	     ((1<<(1+(19-12)))-1)
#define S3C2410_PLLCON_PDIVMASK	     ((1<<5)-1)
#define S3C2410_PLLCON_SDIVMASK	     3

/* DCLKCON register addresses in gpio.h */

#define S3C2410_DCLKCON_DCLK0EN	     (1<<0)
#define S3C2410_DCLKCON_DCLK0_PCLK   (0<<1)
#define S3C2410_DCLKCON_DCLK0_UCLK   (1<<1)
#define S3C2410_DCLKCON_DCLK0_DIV(x) (((x) - 1 )<<4)
#define S3C2410_DCLKCON_DCLK0_CMP(x) (((x) - 1 )<<8)

#define S3C2410_DCLKCON_DCLK1EN	     (1<<16)
#define S3C2410_DCLKCON_DCLK1_PCLK   (0<<17)
#define S3C2410_DCLKCON_DCLK1_UCLK   (1<<17)
#define S3C2410_DCLKCON_DCLK1_DIV(x) (((x) - 1) <<20)

#define S3C2410_CLKDIVN_PDIVN	     (1<<0)
#define S3C2410_CLKDIVN_HDIVN	     (1<<1)

#define S3C2410_CLKSLOW_USB_CLK_DISABLE	(1 << 7)

#ifdef CONFIG_CPU_S3C2440

/* extra registers */
#define S3C2440_CAMDIVN	    S3C2410_CLKREG(0x18)

#define S3C2440_CLKCON_CAMERA        (1<<19)
#define S3C2440_CLKCON_AC97          (1<<20)

#define S3C2440_CLKDIVN_PDIVN	     (1<<0)
#define S3C2440_CLKDIVN_HDIVN_MASK   (3<<1)
#define S3C2440_CLKDIVN_HDIVN_1      (0<<1)
#define S3C2440_CLKDIVN_HDIVN_2      (1<<1)
#define S3C2440_CLKDIVN_HDIVN_4_8    (2<<1)
#define S3C2440_CLKDIVN_HDIVN_3_6    (3<<1)
#define S3C2440_CLKDIVN_UCLK         (1<<3)

#define S3C2440_CAMDIVN_CAMCLK_MASK  (0xf<<0)
#define S3C2440_CAMDIVN_CAMCLK_SEL   (1<<4)
#define S3C2440_CAMDIVN_HCLK3_HALF   (1<<8)
#define S3C2440_CAMDIVN_HCLK4_HALF   (1<<9)
#define S3C2440_CAMDIVN_DVSEN        (1<<12)

#endif /* CONFIG_CPU_S3C2440 */

#ifdef CONFIG_CPU_S3C2412

#define S3C2412_CLKCON_DMA0          (1<<0)
#define S3C2412_CLKCON_DMA1          (1<<1)
#define S3C2412_CLKCON_DMA2          (1<<2)
#define S3C2412_CLKCON_DMA3          (1<<3)
#define S3C2412_CLKCON_NAND          (1<<4)
#define S3C2412_CLKCON_LCDC          (1<<5)
#define S3C2412_CLKCON_USBH          (1<<6)
#define S3C2412_CLKCON_SDRAM         (1<<8)
#define S3C2412_CLKCON_HCLKx1_2      (1<<10)
#define S3C2412_CLKCON_USBD48M       (1<<11)
#define S3C2412_CLKCON_USBH48M       (1<<12)
#define S3C2412_CLKCON_IISCLK        (1<<13)
#define S3C2412_CLKCON_UARTCLK       (1<<14)
#define S3C2412_CLKCON_CAMCLK        (1<<15)
#define S3C2412_CLKCON_USBD          (1<<16)
#define S3C2412_CLKCON_PWMT          (1<<17)
#define S3C2412_CLKCON_SDI           (1<<18)
#define S3C2412_CLKCON_UART0         (1<<19)
#define S3C2412_CLKCON_UART1         (1<<20)
#define S3C2412_CLKCON_UART2         (1<<21)
#define S3C2412_CLKCON_GPIO          (1<<22)
#define S3C2412_CLKCON_RTC           (1<<23)
#define S3C2412_CLKCON_ADC           (1<<24)
#define S3C2412_CLKCON_IIC           (1<<25)
#define S3C2412_CLKCON_IIS           (1<<26)
#define S3C2412_CLKCON_SPI           (1<<27)
#define S3C2412_CLKCON_WDT           (1<<28)

#define S3C2412_CLKDIVN_HCLKDIV_MASK     (3<<0)
#define S3C2412_CLKDIVN_HCLKDIV_1_2      (0<<0)
#define S3C2412_CLKDIVN_HCLKDIV_2_4      (1<<0)
#define S3C2412_CLKDIVN_HCLKDIV_3_6      (2<<0)
#define S3C2412_CLKDIVN_HCLKDIV_4_8      (3<<0)
#define S3C2412_CLKDIVN_PCLKDIV          (1<<2)
#define S3C2412_CLKDIVN_ARMDIV           (1<<3)
#define S3C2412_CLKDIVN_DVSEN            (1<<4)
#define S3C2412_CLKDIVN_HALFHCLK         (1<<5)
#define S3C2412_CLKDIVN_USB48DIV         (1<<6)
#define S3C2412_CLKDIVN_UARTCLKDIV_MASK  (0xf<<8)
#define S3C2412_CLKDIVN_IISCLKDIV_MASK   (0xf<<12)
#define S3C2412_CLKDIVN_CAMCLKDIV_MASK   (0xf<<16)

#define S3C2412_OSCSET                   S3C2410_CLKREG(0x18)
#define S3C2412_CLKSRC                   S3C2410_CLKREG(0x1c)
#define S3C2412_PMCON                    S3C2410_CLKREG(0x20)
#define S3C2412_PWRCFG                   S3C2410_CLKREG(0x24)
#define S3C2412_ENDIAN                   S3C2410_CLKREG(0x2c)
#define S3C2412_SWRSTCON                 S3C2410_CLKREG(0x30)
#define S3C2412_RSTCON                   S3C2410_CLKREG(0x34)
#define S3C2412_INFORM0                  S3C2410_CLKREG(0x70)
#define S3C2412_INFORM1                  S3C2410_CLKREG(0x74)
#define S3C2412_INFORM2                  S3C2410_CLKREG(0x78)
#define S3C2412_INFORM3                  S3C2410_CLKREG(0x7c)

#define S3C2412_OSCSET_XTALWAIT_MASK     (0xffff<<0)

#define S3C2412_CLKSRC_EXTCLKDIV_MASK    (7<<0)
#define S3C2412_CLKSRC_SELEXTDIV         (1<<3)
#define S3C2412_CLKSRC_SELMPLL           (1<<4)
#define S3C2412_CLKSRC_SELUPLL           (1<<5)
#define S3C2412_CLKSRC_SELUART           (1<<8)
#define S3C2412_CLKSRC_SELIIS            (1<<9)
#define S3C2412_CLKSRC_SELUSB            (1<<10)
#define S3C2412_CLKSRC_SELCAM            (1<<11)
#define S3C2412_CLKSRC_SELUREF_MASK      (3<<12)
#define S3C2412_CLKSRC_SELUREF_OM4       (0<<12)
#define S3C2412_CLKSRC_SELUREF_EXTOSC    (2<<12)
#define S3C2412_CLKSRC_SELUREF_EXTCLK    (3<<12)
#define S3C2412_CLKSRC_SELEREF_MASK      (3<<14)
#define S3C2412_CLKSRC_SELEREF_OM4       (0<<12)
#define S3C2412_CLKSRC_SELEREF_EXTOSC    (2<<12)
#define S3C2412_CLKSRC_SELEREF_EXTCLK    (3<<12)

#define S3C2412_PMCON_MODESTOP           (1<<16)
#define S3C2412_PMCON_MODESLEEP_MASK     (0xffff<<0)
#define S3C2412_PMCON_GOTOBED            (0x2bed)

#define S3C2412_PWRCFG_BATF_CFG_MASK     (3<<0)
#define S3C2412_PWRCFG_BATF_CFG_IGN      (0<<0)
#define S3C2412_PWRCFG_BATF_CFG_SLEEP    (2<<0)
#define S3C2412_PWRCFG_STBYWFI_MASK      (3<<6)
#define S3C2412_PWRCFG_STBYWFI_IGN       (0<<6)
#define S3C2412_PWRCFG_STBYWFI_IDLE      (1<<6)
#define S3C2412_PWRCFG_STBYWFI_STOP      (2<<6)
#define S3C2412_PWRCFG_STBYWFI_SLEEP     (3<<6)
#define S3C2412_PWRCFG_RTC_CFG           (1<<8)
#define S3C2412_PWRCFG_NFRESET_CFG       (1<<9)
#define S3C2412_PWRCFG_ESLEEP_CFG        (1<<15)
#define S3C2412_PWRCFG_EINT_WAKE_MASK    (0xffff<<16)

#define S3C2412_SWRSTCON_SWRST           (0x533c2412)

#define S3C2412_RSTCON_PWRSETCNT_MASK    (0xff<<0)
#define S3C2412_RSTCON_RSTCNT_MASK       (0xff<<8)

#define S3C2412_ENDIAN_BIG               (1<<0)

#endif /* CONFIG_CPU_S3C2412 */

#ifdef CONFIG_CPU_S3C2443

/* changed and extra registers */
#define S3C2443_LOCKCON0			S3C2410_CLKREG(0x00)
#define S3C2443_LOCKCON1			S3C2410_CLKREG(0x04)
#define S3C2443_OSCSET				S3C2410_CLKREG(0x08)
#define S3C2443_MPLLCON				S3C2410_CLKREG(0x10)
#define S3C2443_EPLLCON				S3C2410_CLKREG(0x18)
#define S3C2443_CLKSRC				S3C2410_CLKREG(0x20)
#define S3C2443_CLKDIV0				S3C2410_CLKREG(0x24)
#define S3C2443_CLKDIV1				S3C2410_CLKREG(0x28)
#define S3C2443_HCLKCON				S3C2410_CLKREG(0x30)
#define S3C2443_PCLKCON				S3C2410_CLKREG(0x34)
#define S3C2443_SCLKCON				S3C2410_CLKREG(0x38)
#define S3C2443_PWRMODE				S3C2410_CLKREG(0x40)
#define S3C2443_SWRST				S3C2410_CLKREG(0x44)
#define S3C2443_BUSPRI0				S3C2410_CLKREG(0x50)
#define S3C2443_SYSID				S3C2410_CLKREG(0x5c)
#define S3C2443_PWRCFG				S3C2410_CLKREG(0x60)
#define S3C2443_RSTCON				S3C2410_CLKREG(0x64)
#define S3C2443_RSTSTAT				S3C2410_CLKREG(0x68)
#define S3C2443_WKUPSTAT			S3C2410_CLKREG(0x6c)
#define S3C2443_INFORM0				S3C2410_CLKREG(0x70)
#define S3C2443_INFORM1				S3C2410_CLKREG(0x74)
#define S3C2443_INFORM2				S3C2410_CLKREG(0x78)
#define S3C2443_INFORM3				S3C2410_CLKREG(0x7c)
#define S3C2443_PHYCTRL				S3C2410_CLKREG(0x80)
#define S3C2443_PHYPWR				S3C2410_CLKREG(0x84)
#define S3C2443_URSTCON				S3C2410_CLKREG(0x88)
#define S3C2443_UCLKCON				S3C2410_CLKREG(0x8c)

#define S3C2443_LOCKCON0_M_LTIME_MASK		0xffff
#define S3C2443_LOCKCON1_E_LTIME_MASK		0xffff
#define S3C2443_OSCSET_XTALWAIT_MASK		0xffff

#define S3C2443_MPLLCON_SDIV_SHIFT		0
#define S3C2443_MPLLCON_SDIV_MASK		0x3
#define S3C2443_MPLLCON_PDIV_SHIFT		8
#define S3C2443_MPLLCON_PDIV_MASK		0x3
#define S3C2443_MPLLCON_MDIV_SHIFT		16
#define S3C2443_MPLLCON_MDIV_MASK		0xff
#define S3C2443_MPLLCON_ONOFF			(1<<24)
#define S3C2443_MPLLCON_MPLLEN_STOP		(1<<25)

#define S3C2443_EPLLCON_SDIV_SHIFT		0
#define S3C2443_EPLLCON_SDIV_MASK		0x3
#define S3C2443_EPLLCON_PDIV_SHIFT		8
#define S3C2443_EPLLCON_PDIV_MASK		0x3f
#define S3C2443_EPLLCON_MDIV_SHIFT		16
#define S3C2443_EPLLCON_MDIV_MASK		0xff
#define S3C2443_EPLLCON_ONOFF			(1<<24)
#define S3C2443_EPLLCON_MPLLEN_STOP		(1<<25)

#define S3C2443_CLKSRC_SELEXTCLK		(1<<3)
#define S3C2443_CLKSRC_SELMPLL			(1<<4)
#define S3C2443_CLKSRC_SELEPLL                  (1<<6)
#define S3C2443_CLKSRC_SELESRC_MASK		(3<<7)
#define S3C2443_CLKSRC_SELESRC_MPLLREF		(0<<7)
#define S3C2443_CLKSRC_SELESRC_XTAL		(2<<7)
#define S3C2443_CLKSRC_SELESRC_EXTCLK		(3<<7)
#define S3C2443_CLKSRC_SELIIS_MASK		(3<<14)
#define S3C2443_CLKSRC_SELIIS_EPLLDIV		(0<<14)
#define S3C2443_CLKSRC_SELIIS_EXTCLK		(1<<14)
#define S3C2443_CLKSRC_SELIIS_EPLLREF		(2<<14)

#define S3C2443_CLKDIV0_HCLKDIV_SHIFT		0
#define S3C2443_CLKDIV0_HCLKDIV_MASK		0x3
#define S3C2443_CLKDIV0_PCLKDIV			(1<<2)
#define S3C2443_CLKDIV0_HALFHCLK		(1<<3)
#define S3C2443_CLKDIV0_PREDIV_SHIFT		4
#define S3C2443_CLKDIV0_PREDIV_MASK		0x3
#define S3C2443_CLKDIV0_EXTDIV_SHIFT		6
#define S3C2443_CLKDIV0_EXTDIV_MASK		0x7
#define S3C2443_CLKDIV0_ARMDIV_MASK		(15<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_1		(0<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_2		(8<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_3		(2<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_4		(9<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_6		(10<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_8		(11<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_12		(13<<9)
#define S3C2443_CLKDIV0_ARMDIV_1_16		(15<<9)
#define S3C2443_CLKDIV0_DVS			(1<<13)

#define S3C2443_CLKDIV1_USBHOSTDIV_SHIFT	4
#define S3C2443_CLKDIV1_USBHOSTDIV_MASK		0x3
#define S3C2443_CLKDIV1_HSMMCDIV_SHIFT		6
#define S3C2443_CLKDIV1_HSMMCDIV_MASK		0x3
#define S3C2443_CLKDIV1_UARTDIV_SHIFT		8
#define S3C2443_CLKDIV1_UARTDIV_MASK		0xf
#define S3C2443_CLKDIV1_IISDIV_SHIFT		12
#define S3C2443_CLKDIV1_IISDIV_MASK		0xf
#define S3C2443_CLKDIV1_SPIDIV_SHIFT		24
#define S3C2443_CLKDIV1_SPIDIV_MASK		0x3
#define S3C2443_CLKDIV1_CAMDIV_SHIFT		26
#define S3C2443_CLKDIV1_CAMDIV_MASK		0xf

#define S3C2443_HCLKCON_DMA0			(1<<0)
#define S3C2443_HCLKCON_DMA1			(1<<1)
#define S3C2443_HCLKCON_DMA2			(1<<2)
#define S3C2443_HCLKCON_DMA3			(1<<3)
#define S3C2443_HCLKCON_DMA4			(1<<4)
#define S3C2443_HCLKCON_DMA5			(1<<5)
#define S3C2443_HCLKCON_CAMIF   		(1<<8)
#define S3C2443_HCLKCON_DISPCON			(1<<9)
#define S3C2443_HCLKCON_LCDCON			(1<<10)
#define S3C2443_HCLKCON_USBHOST			(1<<11)
#define S3C2443_HCLKCON_USBDEV			(1<<12)
#define S3C2443_HCLKCON_HSMMC			(1<<16)
#define S3C2443_HCLKCON_CFC			(1<<17)
#define S3C2443_HCLKCON_SSMC			(1<<18)
#define S3C2443_HCLKCON_DRAMC			(1<<19)

#define S3C2443_PCLKCON_UART0			(1<<0)
#define S3C2443_PCLKCON_UART1			(1<<1)
#define S3C2443_PCLKCON_UART2			(1<<2)
#define S3C2443_PCLKCON_UART3			(1<<3)
#define S3C2443_PCLKCON_IIC			(1<<4)
#define S3C2443_PCLKCON_SDI			(1<<5)
#define S3C2443_PCLKCON_SPI_HS			(1<<6)
#define S3C2443_PCLKCON_TSADC			(1<<7)
#define S3C2443_PCLKCON_AC97			(1<<8)
#define S3C2443_PCLKCON_IIS			(1<<9)
#define S3C2443_PCLKCON_PWM			(1<<10)
#define S3C2443_PCLKCON_WDT			(1<<11)
#define S3C2443_PCLKCON_RTC			(1<<12)
#define S3C2443_PCLKCON_GPIO			(1<<13)
#define S3C2443_PCLKCON_SPI_0			(1<<14)
#define S3C2443_PCLKCON_SPI_1			(1<<15)

#define S3C2443_SCLKCON_USBHOST			(1<<1)
#define S3C2443_SCLKCON_UARTCLK			(1<<8)
#define S3C2443_SCLKCON_IISCLK			(1<<9)
#define S3C2443_SCLKCON_DISPCLK			(1<<10)
#define S3C2443_SCLKCON_CAMCLK			(1<<11)
#define S3C2443_SCLKCON_HSMMCCLK		(1<<12)
#define S3C2443_SCLKCON_HSMMCCLK_EXT		(1<<13)
#define S3C2443_SCLKCON_SPICLK			(1<<14)
#define S3C2443_SCLKCON_SSMCCLK			(1<<15)
#define S3C2443_SCLKCON_DDRCLK			(1<<16)

#define S3C2443_PWRMODE_IDLE			(1<<17)
#define S3C2443_PWRMODE_STOP			(1<<16)
#define S3C2443_PWRMODE_SLEEP_SHIFT		0
#define S3C2443_PWRMODE_SLEEP_MASK		0xffff
#define S3C2443_PWRMODE_SLEEP_2BED		0x2bed

#define S3C2443_PWRCFG_BATF_CFG_MASK		(3<<0)
#define S3C2443_PWRCFG_BATF_CFG_IGNORE		(0<<0)
#define S3C2443_PWRCFG_BATF_CFG_GENINT		(1<<0)
#define S3C2443_PWRCFG_BATF_CFG_SLEEP		(3<<0)
#define S3C2443_PWRCFG_OSC_EN_STOP		(1<<2)
#define S3C2443_PWRCFG_OSC_EN_SLEEP		(1<<3)
#define S3C2443_PWRCFG_nSW_PHY_OFF_USB		(1<<4)
#define S3C2443_PWRCFG_RTCTICK_CFG		(1<<7)
#define S3C2443_PWRCFG_RTC_CFG			(1<<8)
#define S3C2443_PWRCFG_NFRESET_CFG		(1<<9)
#define S3C2443_PWRCFG_SLEEP_CFG		(1<<15)

#define S3C2443_SWRST_RESTART			(0x533c2443)

#define S3C2443_RSTCON_PWRSETCNT_SHIFT		0
#define S3C2443_RSTCON_PWRSETCNT_MASK		0xff
#define S3C2443_RSTCON_RSTCNT_SHIFT		8
#define S3C2443_RSTCON_RSTCNT_MASK		0xff
#define S3C2443_RSTCON_PWROFF_SLEEP		(1<<16)

#define S3C2443_RSTSTAT_EXTRST			(1<<0)
#define S3C2443_RSTSTAT_WDTRST			(1<<2)
#define S3C2443_RSTSTAT_SLEEP			(1<<3)
#define S3C2443_RSTSTAT_ESLEEP			(1<<4)
#define S3C2443_RSTSTAT_SWRST			(1<<5)

#define S3C2443_WKUPSTAT_EINT			(1<<0)
#define S3C2443_WKUPSTAT_RTC			(1<<1)
#define S3C2443_WKUPSTAT_RTC_TICK		(1<<4)
#define S3C2443_WKUPSTAT_BATF			(1<<5)

#define S3C2443_SYSID_CHIPID_SHIFT		8
#define S3C2443_SYSID_CHIPID_MASK		0xffffff
#define S3C2443_SYSID_SREV_SHIFT		4
#define S3C2443_SYSID_SREV_MASK			0xf
#define S3C2443_SYSID_LREV_SHIFT		0
#define S3C2443_SYSID_LREV_MASK			0xf

#define S3C2443_PHYCTRL_DOWNSTREAM_PORT		(1<<0)
#define S3C2443_PHYCTRL_INT_PLL_SEL		(1<<1)
#define S3C2443_PHYCTRL_EXT_CLK			(1<<2)
#define S3C2443_PHYCTRL_CLK_SEL_MASK		(3<<3)
#define S3C2443_PHYCTRL_CLK_SEL_48MHZ		(0<<3)
#define S3C2443_PHYCTRL_CLK_SEL_12MHZ		(2<<3)
#define S3C2443_PHYCTRL_CLK_SEL_24MHZ		(3<<3)

#define S3C2443_PHYPWR_FORCE_SUSPEND		(1<<0)
#define S3C2443_PHYPWR_PLL_POWERDOWN		(1<<1)
#define S3C2443_PHYPWR_XO_ON			(1<<2)
#define S3C2443_PHYPWR_PLL_REF_CLK		(1<<3)
#define S3C2443_PHYPWR_ANALOG_POWERDOWN_MASK	(3<<4)
#define S3C2443_PHYPWR_ANALOG_POWERDOWN_DOWN	(1<<4)
#define S3C2443_PHYPWR_ANALOG_POWERDOWN_UP	(3<<4)
#define S3C2443_PHYPWR_COMMON_ON_N		(1<<31)

#define S3C2443_URSTCON_PHY_RESET		(1<<0)
#define S3C2443_URSTCON_HOST_RESET		(1<<1)
#define S3C2443_URSTCON_FUNC_RESET		(1<<2)


#define S3C2443_UCLKCON_TCLK_EN		(1<<0)
#define S3C2443_UCLKCON_HOST_CLK_EN		(1<<1)
#define S3C2443_UCLKCON_FUNC_CLK_EN		(1<<2)
/*S3C2443_UCLKCON_HOST_CLK_TEST Host CLK Test mode Enabled to
ensure correct operation this fields should be set to 1
(0 == Enabled 1 == Disable)(source s3c2443 manual rev 10 2-36*/
#define S3C2443_UCLKCON_HOST_CLK_TEST		(1<<4)
#define S3C2443_UCLKCON_DETECT_VBUS		(1<<31)

#endif /* CONFIG_CPU_S3C2443 */

#ifndef __ASSEMBLY__

#include <asm/div64.h>

static inline unsigned
s3c2410_get_pll(unsigned pllval, unsigned baseclk)
{
	unsigned mdiv, pdiv, sdiv;
	u64 num;
	u32 den;

	mdiv = pllval >> S3C2410_PLLCON_MDIVSHIFT;
	pdiv = pllval >> S3C2410_PLLCON_PDIVSHIFT;
	sdiv = pllval >> S3C2410_PLLCON_SDIVSHIFT;

	mdiv &= S3C2410_PLLCON_MDIVMASK;
	pdiv &= S3C2410_PLLCON_PDIVMASK;
	sdiv &= S3C2410_PLLCON_SDIVMASK;

	num = (mdiv + 8ULL) * baseclk;
	den = (pdiv + 2U) << sdiv;
	do_div(num, den);

	return num;
}

#ifdef CONFIG_CPU_S3C2412
static inline void
s3c2412_gen_stbywfi(unsigned cfg)
{
	unsigned reg;
	const int zero = 0;

	reg = __raw_readl(S3C2412_PWRCFG);
	reg &= ~S3C2412_PWRCFG_STBYWFI_MASK;
	reg |= cfg;
	__raw_writel(reg, S3C2412_PWRCFG);

	asm("mcr%? p15, 0, %0, c7, c0, 4" : : "r" (zero));
}
#endif /* CONFIG_CPU_S3C2412 */

#ifdef CONFIG_CPU_S3C2443
static inline unsigned
s3c2443_get_mpll(unsigned pllval, unsigned baseclk)
{
	unsigned mdiv, pdiv, sdiv;
	u64 num;
	u32 den;

	mdiv = pllval >> S3C2443_MPLLCON_MDIV_SHIFT;
	pdiv = pllval >> S3C2443_MPLLCON_PDIV_SHIFT;
	sdiv = pllval >> S3C2443_MPLLCON_SDIV_SHIFT;

	mdiv &= S3C2443_MPLLCON_MDIV_MASK;
	pdiv &= S3C2443_MPLLCON_PDIV_MASK;
	sdiv &= S3C2443_MPLLCON_SDIV_MASK;

	num = 2ULL * (mdiv + 8ULL) * baseclk;
	den = pdiv << sdiv;
	do_div(num, den);

	return num;
}

static inline unsigned
s3c2443_get_epll(unsigned pllval, unsigned baseclk)
{
	unsigned mdiv, pdiv, sdiv;
	u64 num;
	u32 den;

	mdiv = pllval >> S3C2443_EPLLCON_MDIV_SHIFT;
	pdiv = pllval >> S3C2443_EPLLCON_PDIV_SHIFT;
	sdiv = pllval >> S3C2443_EPLLCON_SDIV_SHIFT;

	mdiv &= S3C2443_EPLLCON_MDIV_MASK;
	pdiv &= S3C2443_EPLLCON_PDIV_MASK;
	sdiv &= S3C2443_EPLLCON_SDIV_MASK;

	num = (mdiv + 8ULL) * baseclk;
	den = (pdiv + 2U) << sdiv;
	do_div(num, den);

	return num;
}
#endif /* CONFIG_CPU_S3C2443 */

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ARM_REGS_CLOCK */
