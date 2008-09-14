/* linux/include/asm/arch-s3c2410/regs-iis.h
 *
 * Copyright (c) 2003 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 IIS register definition
 *
 *  Changelog:
 *    19-06-2003     BJD     Created file
 *    26-06-2003     BJD     Finished off definitions for register addresses
 *    12-03-2004     BJD     Updated include protection
 *    07-03-2005     BJD     Added FIFO size flags and S3C2440 MPLL
 *    05-04-2005     LCVR    Added IISFCON definitions for the S3C2400
 *    18-07-2005     DA      Change IISCON_MPLL to IISMOD_MPLL
 *                           Correct IISMOD_256FS and IISMOD_384FS
 *                           Add IISCON_PSCEN
 *    17-02-2006     KM      Added definitions for S3C2412/S3C2413
 */

#ifndef __ASM_ARCH_REGS_IIS_H
#define __ASM_ARCH_REGS_IIS_H

//#define S3C2410_IISCON	 (0x00)
#define S3C2410_IISCON	 (S3C24XX_VA_IIS + 0x00)

#define S3C2410_IISCON_LRINDEX	  (1<<8)
#define S3C2410_IISCON_TXFIFORDY  (1<<7)
#define S3C2410_IISCON_RXFIFORDY  (1<<6)
#define S3C2410_IISCON_TXDMAEN	  (1<<5)
#define S3C2410_IISCON_RXDMAEN	  (1<<4)
#define S3C2410_IISCON_TXIDLE	  (1<<3)
#define S3C2410_IISCON_RXIDLE	  (1<<2)
#define S3C2410_IISCON_PSCEN	  (1<<1)
#define S3C2410_IISCON_IISEN	  (1<<0)

//#define S3C2410_IISMOD	 (0x04)
#define S3C2410_IISMOD	 (S3C24XX_VA_IIS + 0x04)

#define S3C2440_IISMOD_MPLL	  (1<<9)
#define S3C2410_IISMOD_SLAVE	  (1<<8)
#define S3C2410_IISMOD_NOXFER	  (0<<6)
#define S3C2410_IISMOD_RXMODE	  (1<<6)
#define S3C2410_IISMOD_TXMODE	  (2<<6)
#define S3C2410_IISMOD_TXRXMODE	  (3<<6)
#define S3C2410_IISMOD_LR_LLOW	  (0<<5)
#define S3C2410_IISMOD_LR_RLOW	  (1<<5)
#define S3C2410_IISMOD_IIS	  (0<<4)
#define S3C2410_IISMOD_MSB	  (1<<4)
#define S3C2410_IISMOD_8BIT	  (0<<3)
#define S3C2410_IISMOD_16BIT	  (1<<3)
#define S3C2410_IISMOD_BITMASK	  (1<<3)
#define S3C2410_IISMOD_256FS	  (0<<2)
#define S3C2410_IISMOD_384FS	  (1<<2)
#define S3C2410_IISMOD_16FS	  (0<<0)
#define S3C2410_IISMOD_32FS	  (1<<0)
#define S3C2410_IISMOD_48FS	  (2<<0)

//#define S3C2410_IISPSR		(0x08)
#define S3C2410_IISPSR	 (S3C24XX_VA_IIS + 0x08)

#define S3C2410_IISPSR_INTMASK	(31<<5)
#define S3C2410_IISPSR_INTSHIFT	(5)
#define S3C2410_IISPSR_EXTMASK	(31<<0)
#define S3C2410_IISPSR_EXTSHFIT	(0)

//#define S3C2410_IISFCON  (0x0c)
#define S3C2410_IISFCON  (S3C24XX_VA_IIS + 0x0c)

#define S3C2410_IISFCON_TXDMA	  (1<<15)
#define S3C2410_IISFCON_RXDMA	  (1<<14)
#define S3C2410_IISFCON_TXENABLE  (1<<13)
#define S3C2410_IISFCON_RXENABLE  (1<<12)
#define S3C2410_IISFCON_TXMASK	  (0x3f << 6)
#define S3C2410_IISFCON_TXSHIFT	  (6)
#define S3C2410_IISFCON_RXMASK	  (0x3f)
#define S3C2410_IISFCON_RXSHIFT	  (0)

#define S3C2400_IISFCON_TXDMA     (1<<11)
#define S3C2400_IISFCON_RXDMA     (1<<10)
#define S3C2400_IISFCON_TXENABLE  (1<<9)
#define S3C2400_IISFCON_RXENABLE  (1<<8)
#define S3C2400_IISFCON_TXMASK	  (0x07 << 4)
#define S3C2400_IISFCON_TXSHIFT	  (4)
#define S3C2400_IISFCON_RXMASK	  (0x07)
#define S3C2400_IISFCON_RXSHIFT	  (0)

//#define S3C2410_IISFIFO  (0x10)
#define S3C2410_IISFIFO  (S3C24XX_VA_IIS + 0x10)



/*
 * S3C2412/13 has different IIS architecture,
 * this information should not be in this file
 * but it is because we (TomTom) do (does) not
 * wish to create a seperate kernel for just
 * S3C2412/13 architecture - KM
 */

// TODO: check whether S3C24XX_VA_IIS is correct for
//       S3C2412/S3C2413 (final documentation from 
//       Samsung is not ready yet) - KM

//#define S3C2412_IISFCON  (0x00)
#define S3C2412_IISCON	 (S3C24XX_VA_IIS + 0x00)

#define S3C2412_IISCON_LRI		(1<<11)
#define S3C2412_IISCON_FTXEMPT		(1<<10)
#define S3C2412_IISCON_FRXEMPT		(1<<9)
#define S3C2412_IISCON_FTXFULL		(1<<8)
#define S3C2412_IISCON_FRXFULL		(1<<7)
#define S3C2412_IISCON_TXDMAPAUSE	(1<<6)
#define S3C2412_IISCON_RXDMAPAUSE	(1<<5)
#define S3C2412_IISCON_TXCHPAUSE	(1<<4)
#define S3C2412_IISCON_RXCHPAUSE	(1<<3)
#define S3C2412_IISCON_TXDMACTIVE	(1<<2)
#define S3C2412_IISCON_RXDMACTIVE	(1<<1)
#define S3C2412_IISCON_I2SACTIVE	(1<<0)

//#define S3C2412_IISMOD   (0x04)
#define S3C2412_IISMOD	 (S3C24XX_VA_IIS + 0x04)

#define S3C2412_IISMOD_IMSMASK		(3<<10)
#define S3C2412_IISMOD_IMASTER		(0<<10)
#define S3C2412_IISMOD_EMASTER		(1<<10)
#define S3C2412_IISMOD_SLAVE		(2<<10)

#define S3C2412_IISMOD_TXRMASK		(3<<8)
#define S3C2412_IISMOD_XMIT		(0<<8)
#define S3C2412_IISMOD_RECV		(1<<8)
#define S3C2412_IISMOD_XMITRECV		(2<<8)

#define S3C2412_IISMOD_LOCLKL		(0<<7)
#define S3C2412_IISMOD_LOCLKR		(1<<7)

#define S3C2412_IISMOD_SDFMASK		(3<<5)
#define S3C2412_IISMOD_FMTI2S		(0<<5)
#define S3C2412_IISMOD_FMTMSB		(1<<5)
#define S3C2412_IISMOD_FMTLSB		(2<<5)

#define S3C2412_IISMOD_RFSMASK		(3<<3)
#define S3C2412_IISMOD_256FS		(0<<3)
#define S3C2412_IISMOD_512FS		(1<<3)
#define S3C2412_IISMOD_384FS		(2<<3)
#define S3C2412_IISMOD_768FS		(3<<3)

#define S3C2412_IISMOD_BFSMASK		(3<<1)
#define S3C2412_IISMOD_BFS32FS		(0<<1)
#define S3C2412_IISMOD_BFS48FS		(1<<1)
#define S3C2412_IISMOD_BFS16FS		(2<<1)

#define S3C2412_IISMOD_16BIT		(0<<0)
#define S3C2412_IISMOD_8BIT		(1<<0)


//#define S3C2412_IISFIC   (0x08)
#define S3C2412_IISFIC	 (S3C24XX_VA_IIS + 0x08)

#define S3C2412_IISFIC_TFLUSH		(1<<15)
#define S3C2412_IISFIC_FTXCNTMASK	(0x1f << 8)
#define S3C2412_IISFIC_RFLUSH		(1<<7)
#define S3C2412_IISFIC_FRXCNTMASK	(0x1f << 0)

//#define S3C2412_IISPSR   (0x0c)
#define S3C2412_IISPSR	 (S3C24XX_VA_IIS + 0x0c)

#define S3C2412_IISPSR_PSRAEN		(1<<15)
#define S3C2412_IISPSR_PSVALAMASK	(0x3f << 15)

//#define S3C2412_IISTXD   (0x10)
#define S3C2412_IISTXD	 (S3C24XX_VA_IIS + 0x10)


//#define S3C2412_IISRXD   (0x14)
#define S3C2412_IISRXD	 (S3C24XX_VA_IIS + 0x14)


#define S3C2412_DATA_L8MASK		(0xff << 16)
#define S3C2412_DATA_R8MASK		(0xff << 0)
#define S3C2412_DATA_L16MASK		(0xffff << 16)
#define S3C2412_DATA_R16MASK		(0xffff << 0)

#define S3C2443_IISMOD_CDCLKCON		(1<<12)

#endif /* __ASM_ARCH_REGS_IIS_H */
