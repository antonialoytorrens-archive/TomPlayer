/* linux/include/asm-arm/arch-bast/dma.h
 *
 * Copyright (C) 2003,2004 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Samsung S3C2410X DMA support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Changelog:
 *  ??-May-2003 BJD   Created file
 *  ??-Jun-2003 BJD   Added more dma functionality to go with arch
 *  10-Nov-2004 BJD   Added sys_device support
*/

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H __FILE__

#include <linux/config.h>
#include <linux/sysdev.h>
#include "hardware.h"


/*
 * This is the maximum DMA address(physical address) that can be DMAd to.
 *
 */
#define MAX_DMA_ADDRESS		0x20000000
#define MAX_DMA_TRANSFER_SIZE   0x100000 /* Data Unit is half word  */


/* according to the samsung port, we cannot use the regular
 * dma channels... we must therefore provide our own interface
 * for DMA, and allow our drivers to use that.
 */

#define MAX_DMA_CHANNELS	0


/* types */

typedef enum {
	S3C_DMA_IDLE,
	S3C_DMA_RUNNING,
	S3C_DMA_PAUSED
} s3c_dma_state_t;


/* s3c_dma_loadst_t
 *
 * This represents the state of the DMA engine, wrt to the loaded / running
 * transfers. Since we don't have any way of knowing exactly the state of
 * the DMA transfers, we need to know the state to make decisions on wether
 * we can
 *
 * S3C_DMA_NONE
 *
 * There are no buffers loaded (the channel should be inactive)
 *
 * S3C_DMA_1LOADED
 *
 * There is one buffer loaded, however it has not been confirmed to be
 * loaded by the DMA engine. This may be because the channel is not
 * yet running, or the DMA driver decided that it was too costly to
 * sit and wait for it to happen.
 *
 * S3C_DMA_1RUNNING
 *
 * The buffer has been confirmed running, and not finisged
 *
 * S3C_DMA_1LOADED_1RUNNING
 *
 * There is a buffer waiting to be loaded by the DMA engine, and one
 * currently running.
*/

typedef enum {
	S3C_DMALOAD_NONE,
	S3C_DMALOAD_1LOADED,
	S3C_DMALOAD_1RUNNING,
	S3C_DMALOAD_1LOADED_1RUNNING,
} s3c_dma_loadst_t;

typedef enum {
	S3C_RES_OK,
	S3C_RES_ERR,
	S3C_RES_ABORT
} s3c_dma_buffresult_t;


typedef enum s3c_dmasrc_e s3c_dmasrc_t;

enum s3c_dmasrc_e {
	S3C_DMASRC_HW,      /* source is memory */
	S3C_DMASRC_MEM      /* source is hardware */
};

/* enum s3c_chan_op_e
 *
 * operation codes passed to the DMA code by the user, and also used
 * to inform the current channel owner of any changes to the system state
*/

enum s3c_chan_op_e {
	S3C_DMAOP_START,
	S3C_DMAOP_STOP,
	S3C_DMAOP_PAUSE,
	S3C_DMAOP_RESUME,
	S3C_DMAOP_FLUSH,
	S3C_DMAOP_TIMEOUT,           /* internal signal to handler */
};

typedef enum s3c_chan_op_e s3c_chan_op_t;

/* flags */

#define S3C_DMAF_SLOW		(1<<0)   /* slow, so don't worry about
					    * waiting for reloads */
#define S3C_DMAF_AUTOSTART	(1<<1)   /* auto-start if buffer queued */
#define S3C_DMAF_LOOPING	(1<<2)   /* loop the first queued buffer */

/* dma buffer */

typedef struct s3c_dma_buf_s s3c_dma_buf_t;

struct s3c_dma_client {
	const char          *name;
};

typedef struct s3c_dma_client s3c_dma_client_t;

/* s3c_dma_buf_s
 *
 * internally used buffer structure to describe a queued or running
 * buffer.
*/

struct s3c_dma_buf_s {
	s3c_dma_buf_t   *next;
	int                  magic;        /* magic */
	int                  size;         /* buffer size in bytes */
	dma_addr_t           data;         /* start of DMA data */
	dma_addr_t           ptr;          /* where the DMA got to [1] */
	void                *id;           /* client's id */
	int                  chunks;	   /* used for looping DMA */
	int                  chsize;
};

/* [1] is this updated for both recv/send modes? */

typedef struct s3c_dma_chan_s s3c_dma_chan_t;

/* s3c_dma_cbfn_t
 *
 * buffer callback routine type
*/

typedef void (*s3c_dma_cbfn_t)(s3c_dma_chan_t *, void *buf, int size,
				   s3c_dma_buffresult_t result);

typedef int  (*s3c_dma_opfn_t)(s3c_dma_chan_t *,
				   s3c_chan_op_t );

typedef void (*s3c_dma_lcfn_t)(s3c_dma_chan_t *, int chunk);

struct s3c_dma_stats_s {
	unsigned long          loads;
	unsigned long          timeout_longest;
	unsigned long          timeout_shortest;
	unsigned long          timeout_avg;
	unsigned long          timeout_failed;
};

typedef struct s3c_dma_stats_s s3c_dma_stats_t;

/* struct s3c_dma_chan_s
 *
 * full state information for each DMA channel
*/

struct s3c_dma_chan_s {
	/* channel state flags and information */
	unsigned char          number;      /* number of this dma channel */
	unsigned char          in_use;      /* channel allocated */
	unsigned char          irq_claimed; /* irq claimed for channel */
	unsigned char          irq_enabled; /* irq enabled for channel */
	unsigned char          xfer_unit;   /* size of an transfer */

	/* channel state */

	s3c_dma_state_t    state;
	s3c_dma_loadst_t   load_state;
	s3c_dma_client_t  *client;

	/* channel configuration */
	s3c_dmasrc_t       source;
	unsigned long          dev_addr;
	int		       hwsrc;
	unsigned long          load_timeout;
	unsigned int           flags;        /* channel flags */

	/* channel's hardware position and configuration */
	void __iomem           *regs;        /* channels registers */
	void __iomem           *addr_reg;    /* data address register */
	unsigned int           irq;          /* channel irq */
	unsigned long	       client_dcon;  /* client's default value of DCON */
	unsigned long          dcon;         /* our default value of DCON */
	unsigned long          reqsel;       /* default value for DREQSEL */

	/* driver handles */
	s3c_dma_cbfn_t     callback_fn;  /* buffer done callback */
	s3c_dma_opfn_t     op_fn;        /* channel operation callback */
	s3c_dma_lcfn_t     loopchunk_fn; /* notification when a chunk of loop is done */

	/* stats gathering */
	s3c_dma_stats_t   *stats;
	s3c_dma_stats_t    stats_store;

	/* buffer list and information */
	s3c_dma_buf_t      *curr;        /* current dma buffer */
	s3c_dma_buf_t      *next;        /* next buffer to load */
	s3c_dma_buf_t      *end;         /* end of queue */
	
	/* chunk management for looping buffer */
	int			chunk;
	int			cchunk;

	/* system device */
	struct sys_device	dev;
};

/* note, we don't really use dma_device_t at the moment */
typedef unsigned long dma_device_t;

/* functions --------------------------------------------------------------- */

/* s3c_dma_request
 *
 * request a dma channel exclusivley
*/

extern int s3c_dma_request(dmach_t channel,
			       s3c_dma_client_t *, void *dev);


/* s3c_dma_ctrl
 *
 * change the state of the dma channel
*/

extern int s3c_dma_ctrl(dmach_t channel, s3c_chan_op_t op);

/* s3c_dma_setflags
 *
 * set the channel's flags to a given state
*/

extern int s3c_dma_setflags(dmach_t channel,
				unsigned int flags);

/* s3c_dma_free
 *
 * free the dma channel (will also abort any outstanding operations)
*/

extern int s3c_dma_free(dmach_t channel, s3c_dma_client_t *);

/* s3c_dma_enqueue
 *
 * place the given buffer onto the queue of operations for the channel.
 * The buffer must be allocated from dma coherent memory, or the Dcache/WB
 * drained before the buffer is given to the DMA system.
*/

extern int s3c_dma_enqueue(dmach_t channel, void *id,
			       dma_addr_t data, int size);

extern int s3c_dma_setlooping(dmach_t channel, dma_addr_t data, 
				int size, int chunks);

extern int s3c_dma_getpos(dmach_t channel, void **id, unsigned long *pos);

/* s3c_dma_config
 *
 * configure the dma channel
*/

extern int s3c_dma_config(dmach_t channel, int xferunit, int dcon, int reqsel );

/* s3c_dma_devconfig
 *
 * configure the device we're talking to
*/

extern int s3c_dma_devconfig(int channel, s3c_dmasrc_t source,
				 int hwcfg, unsigned long devaddr );

/* s3c_dma_getposition
 *
 * get the position that the dma transfer is currently at
*/

extern int s3c_dma_getposition(dmach_t channel,
				   dma_addr_t *src, dma_addr_t *dest);

extern int s3c_dma_set_opfn(dmach_t, s3c_dma_opfn_t rtn);
extern int s3c_dma_set_buffdone_fn(dmach_t, s3c_dma_cbfn_t rtn);
extern int s3c_dma_set_loopchunk_fn(dmach_t channel, s3c_dma_lcfn_t rtn);

/* debugging helpers */
extern int s3c_dma_debug_channel(dmach_t chnr);

/* DMA Register definitions */

#define S3C_DMA_DISRC		(0x00)
#define S3C_DMA_DISRCC		(0x04)
#define S3C_DMA_DIDST		(0x08)
#define S3C_DMA_DIDSTC		(0x0C)
#define S3C_DMA_DCON		(0x10)
#define S3C_DMA_DSTAT		(0x14)
#define S3C_DMA_DCSRC		(0x18)
#define S3C_DMA_DCDST		(0x1C)
#define S3C_DMA_DMASKTRIG	(0x20)
#ifdef CONFIG_CPU_S3C2412
#define S3C2412_DMA_REQSEL	(0x24)
#endif /* CONFIG_CPU_S3C2412 */

#define S3C_DISRCC_INC		(1<<0)
#define S3C_DISRCC_APB		(1<<1)

#define S3C_DMASKTRIG_STOP	(1<<2)
#define S3C_DMASKTRIG_ON	(1<<1)
#define S3C_DMASKTRIG_SWTRIG	(1<<0)

#define S3C_DCON_DEMAND		(0<<31)
#define S3C_DCON_HANDSHAKE	(1<<31)
#define S3C_DCON_SYNC_PCLK	(0<<30)
#define S3C_DCON_SYNC_HCLK	(1<<30)

#define S3C_DCON_INTREQ		(1<<29)

#define S3C_DCON_HWSRC_SHIFT	(24)
#define S3C_DCON_HWSRC_MASK	(0x7)

#define S3C_DCON_CH0_XDREQ0	(0<<24)
#define S3C_DCON_CH0_UART0	(1<<24)
#define S3C_DCON_CH0_SDI	(2<<24)
#define S3C_DCON_CH0_TIMER	(3<<24)
#define S3C_DCON_CH0_USBEP1	(4<<24)

#define S3C_DCON_CH1_XDREQ1	(0<<24)
#define S3C_DCON_CH1_UART1	(1<<24)
#define S3C_DCON_CH1_I2SSDI	(2<<24)
#define S3C_DCON_CH1_SPI	(3<<24)
#define S3C_DCON_CH1_USBEP2	(4<<24)

#define S3C_DCON_CH2_I2SSDO	(0<<24)
#define S3C_DCON_CH2_I2SSDI	(1<<24)
#define S3C_DCON_CH2_SDI	(2<<24)
#define S3C_DCON_CH2_TIMER	(3<<24)
#define S3C_DCON_CH2_USBEP3	(4<<24)

#define S3C_DCON_CH3_UART2	(0<<24)
#define S3C_DCON_CH3_SDI	(1<<24)
#define S3C_DCON_CH3_SPI	(2<<24)
#define S3C_DCON_CH3_TIMER	(3<<24)
#define S3C_DCON_CH3_USBEP4	(4<<24)

#define S3C_DCON_SRCSHIFT	(24)
#define S3C_DCON_SRCMASK	(7<<24)

#define S3C_DCON_BYTE		(0<<20)
#define S3C_DCON_HALFWORD	(1<<20)
#define S3C_DCON_WORD		(2<<20)

#define S3C_DCON_AUTORELOAD	(0<<22)
#define S3C_DCON_NORELOAD	(1<<22)
#define S3C_DCON_HWTRIG		(1<<23)

#define S3C_REQSEL_SPI_0_TX     (0<<1)
#define S3C_REQSEL_SPI_0_RX     (1<<1)
#define S3C_REQSEL_SPI_1_TX     (2<<1)
#define S3C_REQSEL_SPI_1_RX     (3<<1)
#define S3C_REQSEL_IIS_TX       (4<<1)
#define S3C_REQSEL_IIS_RX       (5<<1)
#define S3C_REQSEL_PWM_TIMER    (9<<1)
#define S3C_REQSEL_SDMMC        (10<<1)
#define S3C_REQSEL_USB_DEV_EP1  (13<<1)		//s3c2412
#define S3C_REQSEL_USB_DEV_EP2  (14<<1)		//s3c2412
#define S3C_REQSEL_USB_DEV_EP3  (15<<1)		//s3c2412
#define S3C_REQSEL_USB_DEV_EP4  (16<<1)		//s3c2412
#define S3C_REQSEL_nXDREQ0      (17<<1)
#define S3C_REQSEL_nXDREQ1      (18<<1)
#define S3C_REQSEL_UART_00      (19<<1)
#define S3C_REQSEL_UART_01      (20<<1)
#define S3C_REQSEL_UART_10      (21<<1)
#define S3C_REQSEL_UART_11      (22<<1)
#define S3C_REQSEL_UART_20      (23<<1)
#define S3C_REQSEL_UART_21      (24<<1)
#define S3C_REQSEL_UART_30      (25<<1)
#define S3C_REQSEL_UART_31      (26<<1)
#define S3C_REQSEL_PCM_OUT      (27<<1)
#define S3C_REQSEL_PCM_IN       (28<<1)
#define S3C_REQSEL_MIC_IN       (29<<1)

#define S3C_REQSEL_HWTRIG	(1<<0)
#define S3C_REQSEL_SRCSHIFT	(1)
#define S3C_REQSEL_SRCMASK	(31<<1)

#define S3C2440_DSTAT_CURRTC_MASK (0xFFFFF)

#ifdef CONFIG_CPU_S3C2440
#define S3C2440_DIDSTC_CHKINT	(1<<2)

#define S3C2440_DCON_CH0_I2SSDO	(5<<24)
#define S3C2440_DCON_CH0_PCMIN	(6<<24)

#define S3C2440_DCON_CH1_PCMOUT	(5<<24)
#define S3C2440_DCON_CH1_SDI	(6<<24)

#define S3C2440_DCON_CH2_PCMIN	(5<<24)
#define S3C2440_DCON_CH2_MICIN	(6<<24)

#define S3C2440_DCON_CH3_MICIN	(5<<24)
#define S3C2440_DCON_CH3_PCMOUT	(6<<24)
#endif /* CONFIG_CPU_S3C2412 */

#endif /* __ASM_ARCH_DMA_H */
