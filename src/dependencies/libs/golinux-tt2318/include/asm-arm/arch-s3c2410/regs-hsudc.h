#ifndef _ASM_ARCH_REGS_HS_HSUDC_H_
#define _ASM_ARCH_REGS_HS_HSUDC_H_

/* USB2.0 Device Controller register */
#define S3C_HSUDCREG(x) ((x) + S3C_VA_HSUDC)

/* Non-Indexed Registers */
#define S3C_HSUDC_INDEX_REG		S3C_HSUDCREG(0x00) /* Index register */
#define S3C_HSUDC_EP_INT_REG		S3C_HSUDCREG(0x04) /* EP Interrupt pending and clear */
#define S3C_HSUDC_EP_INT_EN_REG		S3C_HSUDCREG(0x08) /* EP Interrupt enable */
#define S3C_HSUDC_FUNC_ADDR_REG		S3C_HSUDCREG(0x0c) /* Function address */
#define S3C_HSUDC_FRAME_NUM_REG		S3C_HSUDCREG(0x10) /* Frame number */
#define S3C_HSUDC_EP_DIR_REG		S3C_HSUDCREG(0x14) /* Endpoint direction */
#define S3C_HSUDC_TEST_REG		S3C_HSUDCREG(0x18) /* Test register */
#define S3C_HSUDC_SYS_STATUS_REG	S3C_HSUDCREG(0x1c) /* System status */
#define S3C_HSUDC_SYS_CON_REG		S3C_HSUDCREG(0x20) /* System control */
#define S3C_HSUDC_EP0_STATUS_REG	S3C_HSUDCREG(0x24) /* Endpoint 0 status */
#define S3C_HSUDC_EP0_CON_REG		S3C_HSUDCREG(0x28) /* Endpoint 0 control */
#define S3C_HSUDC_EP0_FIFO_REG		S3C_HSUDCREG(0x60) /* Endpoint 0 Buffer */
#define S3C_HSUDC_EP1_FIFO_REG		S3C_HSUDCREG(0x64) /* Endpoint 1 Buffer */
#define S3C_HSUDC_EP2_FIFO_REG		S3C_HSUDCREG(0x68) /* Endpoint 2 Buffer */
#define S3C_HSUDC_EP3_FIFO_REG		S3C_HSUDCREG(0x6c) /* Endpoint 3 Buffer */
#define S3C_HSUDC_EP4_FIFO_REG		S3C_HSUDCREG(0x70) /* Endpoint 4 Buffer */
#define S3C_HSUDC_EP5_FIFO_REG		S3C_HSUDCREG(0x74) /* Endpoint 5 Buffer */
#define S3C_HSUDC_EP6_FIFO_REG		S3C_HSUDCREG(0x78) /* Endpoint 6 Buffer */
#define S3C_HSUDC_EP7_FIFO_REG		S3C_HSUDCREG(0x7c) /* Endpoint 7 Buffer */
#define S3C_HSUDC_EP8_FIFO_REG		S3C_HSUDCREG(0x80) /* Endpoint 8 Buffer */
#define S3C_HSUDC_FIFO_CON_REG		S3C_HSUDCREG(0x100) /* Burst FIFO-DMA Control */
#define S3C_HSUDC_FIFO_STATUS_REG	S3C_HSUDCREG(0x104) /* Burst FIFO Status */

/* Indexed Registers */	
#define S3C_HSUDC_EP_STATUS_REG			S3C_HSUDCREG(0x2c) /* Endpoints status */
#define S3C_HSUDC_EP_CON_REG			S3C_HSUDCREG(0x30) /* Endpoints control */
#define S3C_HSUDC_BYTE_READ_CNT_REG		S3C_HSUDCREG(0x34) /* Byte read count */
#define S3C_HSUDC_BYTE_WRITE_CNT_REG		S3C_HSUDCREG(0x38) /* Byte write count */
#define S3C_HSUDC_MAXP_REG			S3C_HSUDCREG(0x3c) /* Max packet size */
#define S3C_HSUDC_DMA_CON_REG			S3C_HSUDCREG(0x40) /* DMA control */
#define S3C_HSUDC_DMA_CNT_REG			S3C_HSUDCREG(0x44) /* DMA count */
#define S3C_HSUDC_DMA_FIFO_CNT_REG		S3C_HSUDCREG(0x48) /* DMA FIFO count */
#define S3C_HSUDC_DMA_TOTAL_CNT1_REG		S3C_HSUDCREG(0x4c) /* DMA Total Transfer count1 */
#define S3C_HSUDC_DMA_TOTAL_CNT2_REG		S3C_HSUDCREG(0x50) /* DMA Total Transfer count2 */
#define S3C_HSUDC_DMA_IF_CON_REG		S3C_HSUDCREG(0x84) /* DMA interface Control */
#define S3C_HSUDC_DMA_MEM_BASE_ADDR_REG		S3C_HSUDCREG(0x88) /* Mem Base Addr */
#define S3C_HSUDC_DMA_MEM_CURRENT_ADDR_REG	S3C_HSUDCREG(0x8c) /* Mem current Addr */

/* EP interrupt register Bits */
#define S3C_HSUDC_INT_EP3		(1<<3) // R/C
#define S3C_HSUDC_INT_EP2		(1<<2) // R/C
#define S3C_HSUDC_INT_EP1		(1<<1) // R/C
#define S3C_HSUDC_INT_EP0		(1<<0) // R/C

/* System status register Bits */
#define S3C_HSUDC_INT_CHECK		(0xff8f)
#define S3C_HSUDC_INT_ERR		(0xff80) // R/C
#define S3C_HSUDC_INT_VBUSOFF		(1<<9) // R/C
#define S3C_HSUDC_INT_VBUSON		(1<<8) // R/C
#define S3C_HSUDC_INT_HSP		(1<<4) // R
#define S3C_HSUDC_INT_SDE		(1<<3) // R/C
#define S3C_HSUDC_INT_RESUME		(1<<2) // R/C
#define S3C_HSUDC_INT_SUSPEND		(1<<1) // R/C
#define S3C_HSUDC_INT_RESET		(1<<0) // R/C

/* system control register Bits */
#define S3C_HSUDC_DTZIEN_EN		(1<<14)
#define S3C_HSUDC_RRD_EN		(1<<5)
#define S3C_HSUDC_SUS_EN		(1<<1)
#define S3C_HSUDC_RST_EN		(1<<0)

/* EP0 status register Bits */
#define S3C_HSUDC_EP0_LWO		(1<<6)
#define S3C_HSUDC_EP0_STALL		(1<<4)
#define S3C_HSUDC_EP0_TX_SUCCESS	(1<<1)
#define S3C_HSUDC_EP0_RX_SUCCESS	(1<<0)

/* EP status register Bits */
#define S3C_HSUDC_EP_DOM		(1<<7)
#define S3C_HSUDC_EP_FIFO_FLUSH		(1<<6)
#define S3C_HSUDC_EP_STALL		(1<<5)
#define S3C_HSUDC_EP_LWO		(1<<4)
#define S3C_HSUDC_EP_PSIF_TWO		(2<<2)
#define S3C_HSUDC_EP_TX_SUCCESS		(1<<1)
#define S3C_HSUDC_EP_RX_SUCCESS		(1<<0)

#endif /* _ASM_ARCH_REGS_HS_HSUDC_H_ */
