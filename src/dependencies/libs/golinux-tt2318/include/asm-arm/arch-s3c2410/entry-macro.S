/*
 * include/asm-arm/arch-s3c2410/entry-macro.S
 *
 * Low-level IRQ helper macros for S3C2410-based platforms
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.

 * Modifications:
 *     10-Mar-2005 LCVR  Changed S3C2410_VA to S3C24XX_VA
 */


	.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp

		mov	\tmp, #S3C24XX_VA_IRQ
		ldr	\irqstat, [\tmp, #0x10]		@ INTPND

		teq	\irqstat, #0			@ quick test no IRQ
		beq	1001f				@ bail out, Z set

		@ work out interrupt number: calculate lowest bit set in \irqstat
		movs	\irqnr, \irqstat, lsl #16
		mov	\irqnr, #0
		addeq	\irqnr, \irqnr, #16
		moveq	\irqstat, \irqstat, lsr #16
		tst	\irqstat, #0xff
		addeq	\irqnr, \irqnr, #8
		moveq	\irqstat, \irqstat, lsr #8
		tst	\irqstat, #0xf
		addeq	\irqnr, \irqnr, #4
		moveq	\irqstat, \irqstat, lsr #4
		tst	\irqstat, #0x3
		addeq	\irqnr, \irqnr, #2
		moveq	\irqstat, \irqstat, lsr #2
		tst	\irqstat, #0x1
		addeq	\irqnr, \irqnr, #1

		teq	\irqnr, #4
		teqne	\irqnr, #5
		beq	1002f				@ external irq reg
		add	\irqnr, \irqnr, #IRQ_EINT0
		b	1001f

		@ we get here from no main or external interrupts pending
1002:
		ldr	\irqnr, s3c24xx_eintofs
		add	\tmp, \tmp, \irqnr
		ldr	\irqstat, [ \tmp, # 0xa8 ]	@ EXTINTPEND
		ldr	\irqnr, [ \tmp, # 0xa4 ]	@ EXTINTMASK

		bic	\irqstat, \irqstat, \irqnr	@ clear masked irqs

		mov	\irqnr, #IRQ_EINT4		@ start extint nos
		mov	\irqstat, \irqstat, lsr#4	@ ignore bottom 4 bits
10021:
		movs	\irqstat, \irqstat, lsr#1
		bcs	1004f
		add	\irqnr, \irqnr, #1
		cmp	\irqnr, #IRQ_EINT23
		ble	10021b

		@ found no interrupt, set Z flag and leave
		movs	\irqnr, #0

1004:		@ ensure Z flag clear in case our MOVS shifted out the last bit
		teq	\irqnr, #0
1001:
		@ exit irq routine
		.endm


		/* currently don't need an disable_fiq macro */

		.macro	disable_fiq
		.endm


		/* global used to distinguish between 2412/3 and others. */

		.globl	s3c24xx_eintofs
s3c24xx_eintofs:
		.word	S3C24XX_VA_GPIO - S3C24XX_VA_IRQ
