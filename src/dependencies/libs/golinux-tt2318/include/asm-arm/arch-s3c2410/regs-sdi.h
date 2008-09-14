/* linux/include/asm/arch-s3c2410/regs-sdi.h
 *
 * Copyright (c) 2004 Simtec Electronics <linux@simtec.co.uk>
 *		      http://www.simtec.co.uk/products/SWLINUX/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * S3C2410 MMC/SDIO register definitions
 *
 *  Changelog:
 *    18-Aug-2004 Ben Dooks      Created initial file
 *    29-Nov-2004 Koen Martens   Added some missing defines, fixed duplicates
 *    29-Nov-2004 Ben Dooks	 Updated Koen's patch
*/

#ifndef __ASM_ARM_REGS_SDI
#define __ASM_ARM_REGS_SDI "regs-sdi.h"

#include <asm/arch/regs-dyn.h>

/*
 * MMC/SD/SDIO Registers
 */
#define S3C_SDICON              (0x00)
#define S3C_SDIPRE              (0x04)
#define S3C_SDICARG             (0x08)
#define S3C_SDICCON             (0x0C)
#define S3C_SDICSTA             (0x10)
#define S3C_SDIRSP0             (0x14)
#define S3C_SDIRSP1             (0x18)
#define S3C_SDIRSP2             (0x1C)
#define S3C_SDIRSP3             (0x20)
#define S3C_SDITIMER            (0x24)
#define S3C_SDIBSIZE            (0x28)
#define S3C_SDIDCON             (0x2C)
#define S3C_SDIDCNT             (0x30)
#define S3C_SDIDSTA             (0x34)
#define S3C_SDIFSTA             (0x38)

/* WARNING: Samsung screwup! 
 *
 * On S3C2410 SDIIMSK & SDIDAT have their offsets
 * swapped compared to all newer SoC's.
 */
#define S3C_SDIIMSK		s3c24xx_sdiimsk
#define S3C_SDIDAT		s3c24xx_sdidat

#define S3C2440_SDIIMSK         (0x3c)
#define S3C2440_SDIDAT          (0x40)

#define S3C2410_SDIIMSK		(0x40)
#define S3C2410_SDIDAT		(0x3c)

#define S3C_SDICON_RESET        (1<<8)
#define S3C_SDICON_CLKTYPE_MMC  (1<<5)
#define S3C_SDICON_BYTEORDER    (1<<4)
#define S3C_SDICON_SDIOIRQ      (1<<3)
#define S3C_SDICON_RWAITEN      (1<<2)
#define S3C_SDICON_CLKENABLE    (1<<0)

#define S3C_SDICCON_ABORT       (1<<12)
#define S3C_SDICCON_WITHDATA    (1<<11)
#define S3C_SDICCON_LONGRSP     (1<<10)
#define S3C_SDICCON_WAITRSP     (1<<9)
#define S3C_SDICCON_CMDSTART    (1<<8)
#define S3C_SDICCON_SENDERHOST  (1<<6)
#define S3C_SDICCON_INDEX       (0x3f)

#define S3C_SDICSTA_CRCFAIL     (1<<12)
#define S3C_SDICSTA_CMDSENT     (1<<11)
#define S3C_SDICSTA_CMDTOUT     (1<<10)
#define S3C_SDICSTA_RSPFIN      (1<<9)
#define S3C_SDICSTA_XFERING     (1<<8)
#define S3C_SDICSTA_INDEX       (0xff)

#define S3C_SDIDCON_BURST4      (1<<24)
#define S3C_SDIDCON_WORDTX      (2<<22)
#define S3C_SDIDCON_IRQPERIOD   (1<<21)
#define S3C_SDIDCON_TXAFTERRESP (1<<20)
#define S3C_SDIDCON_RXAFTERCMD  (1<<19)
#define S3C_SDIDCON_BACMD       (1<<18)
#define S3C_SDIDCON_BLOCKMODE   (1<<17)
#define S3C_SDIDCON_WIDEBUS     (1<<16)
#define S3C_SDIDCON_DMAEN       (1<<15)
#define S3C_SDIDCON_DSTART      (1<<14)
#define S3C_SDIDCON_DATMODE     (3<<12)
#define S3C_SDIDCON_BLKNUM      (0x7ff)

/* constants for S3C_SDIDCON_DATMODE */
#define S3C_SDIDCON_XFER_READY  (0<<12)
#define S3C_SDIDCON_XFER_CHK    (1<<12)
#define S3C_SDIDCON_XFER_RX     (2<<12)
#define S3C_SDIDCON_XFER_TX     (3<<12)

#define S3C_SDIDCON_BLKNUM_MASK (0xfff)

#define S3C_SDIDSTA_NOBUSY      (1<<11)
#define S3C_SDIDSTA_RDWAITREQ   (1<<10)
#define S3C_SDIDSTA_IRQDETECT   (1<<9)
#define S3C_SDIDSTA_FIFOFAIL	(1<<8)
#define S3C_SDIDSTA_CRCFAIL     (1<<7)
#define S3C_SDIDSTA_RXCRCFAIL   (1<<6)
#define S3C_SDIDSTA_DATATIMEOUT (1<<5)
#define S3C_SDIDSTA_XFERFINISH  (1<<4)
#define S3C_SDIDSTA_BUSYFINISH  (1<<3)
#define S3C_SDIDSTA_SBITERR     (1<<2)  /* reserved on 2412 */
#define S3C_SDIDSTA_TXDATAON    (1<<1)
#define S3C_SDIDSTA_RXDATAON    (1<<0)

#define S3C_SDIFSTA_FRST        (1<<16)
#define S3C_SDIFSTA_FIFOFAIL    (1<<14)
#define S3C_SDIFSTA_TFDET       (1<<13)
#define S3C_SDIFSTA_RFDET       (1<<12)
#define S3C_SDIFSTA_TXHALF      (1<<11)
#define S3C_SDIFSTA_TXEMPTY     (1<<10)
#define S3C_SDIFSTA_RFLAST      (1<<9)
#define S3C_SDIFSTA_RFFULL      (1<<8)
#define S3C_SDIFSTA_RFHALF      (1<<7)
#define S3C_SDIFSTA_COUNTMASK   (0x7f)

#define S3C_SDIIMSK_NOBUSY      (1<<18)
#define S3C_SDIIMSK_RESPONSECRC (1<<17)
#define S3C_SDIIMSK_CMDSENT     (1<<16)
#define S3C_SDIIMSK_CMDTIMEOUT  (1<<15)
#define S3C_SDIIMSK_RESPONSEND  (1<<14)
#define S3C_SDIIMSK_READWAIT    (1<<13)
#define S3C_SDIIMSK_SDIOIRQ     (1<<12)
#define S3C_SDIIMSK_FIFOFAIL    (1<<11)
#define S3C_SDIIMSK_CRCSTATUS   (1<<10)
#define S3C_SDIIMSK_DATACRC     (1<<9)
#define S3C_SDIIMSK_DATATIMEOUT (1<<8)
#define S3C_SDIIMSK_DATAFINISH  (1<<7)
#define S3C_SDIIMSK_BUSYFINISH  (1<<6)
#define S3C_SDIIMSK_SBITERR     (1<<5)  /* reserved 2412 */
#define S3C_SDIIMSK_TXFIFOHALF  (1<<4)
#define S3C_SDIIMSK_TXFIFOEMPTY (1<<3)
#define S3C_SDIIMSK_RXFIFOLAST  (1<<2)
#define S3C_SDIIMSK_RXFIFOFULL  (1<<1)
#define S3C_SDIIMSK_RXFIFOHALF  (1<<0)

#endif /* __ASM_ARM_REGS_SDI */
