/*-
 * Copyright (c) 2008, Kohsuke Ohtani
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * interrupt.c - interrupt handling routines
 */

#include <kernel.h>
#include <irq.h>
#include <locore.h>
#include <cpu.h>

#define REG8(x) (*(volatile uint8_t*)(x))
#define REG16(x) (*(volatile uint16_t*)(x))
#define REG32(x) (*(volatile uint32_t*)(x))

/* Low-level interrupt handler --- do not call directly! */
extern void interrupt_entry(void);

/* This mapping converts from Cybiko boot routine numbers to H8300S interrupt
 * vectors.
 */

static const u_char cybiko_to_h8300s[] =
{
	INTERRUPT_IRQ0, // 0
	INTERRUPT_IRQ1,
	INTERRUPT_IRQ2,
	INTERRUPT_IRQ3,
	INTERRUPT_IRQ4,
	INTERRUPT_IRQ5, // 5
	INTERRUPT_IRQ6,
	INTERRUPT_IRQ7,
	INTERRUPT_SWDTEND,
	INTERRUPT_WOVI,
	INTERRUPT_ADI, // 10
	INTERRUPT_TGI0A,
	INTERRUPT_TGI0B,
	INTERRUPT_TGI0C,
	INTERRUPT_TGI0D,
	INTERRUPT_TGI0V, // 15
	INTERRUPT_TGI1A,
	INTERRUPT_TGI1B,
	INTERRUPT_TGI1V,
	INTERRUPT_TGI1U,
	INTERRUPT_TGI2A, // 20
	INTERRUPT_TGI2B,
	INTERRUPT_TGI2V,
	INTERRUPT_TGI2U,
	INTERRUPT_CMIA0,
	INTERRUPT_CMIB0, // 25
	INTERRUPT_OVI0,
	INTERRUPT_CMIA1,
	INTERRUPT_CMIB1,
	INTERRUPT_OVI1,
	INTERRUPT_ERI0, // 30
	INTERRUPT_RXI0,
	INTERRUPT_TXI0,
	INTERRUPT_TEI0,
	INTERRUPT_ERI1,
	INTERRUPT_RXI1, // 35
	INTERRUPT_TXI1,
	INTERRUPT_TEI1,
	INTERRUPT_ERI2,
	INTERRUPT_RXI2,
	INTERRUPT_TXI2, // 40
	INTERRUPT_TEI2
};

#define NUMBER_OF_CYBIKO_INTERRUPTS (sizeof(cybiko_to_h8300s) / \
		sizeof(*cybiko_to_h8300s))

/*
 * Unmask interrupt in ICU for specified irq.
 * The interrupt mask table is also updated.
 * Assumes CPU interrupt is disabled in caller.
 */
void interrupt_unmask(int vector, int level)
{
}

/*
 * Mask interrupt in ICU for specified irq.
 * Interrupt must be disabled when this routine is called.
 */
void interrupt_mask(int vector)
{
}

/*
 * Setup interrupt mode.
 * Select whether an interrupt trigger is edge or level.
 */
void interrupt_setup(int vector, int mode)
{
	/* For IRQ0 to IRQ7 only, set edge/level sensitivity. */

	if ((vector >= INTERRUPT_IRQ0) && (vector <= INTERRUPT_IRQ7))
	{
		int irq = vector - INTERRUPT_IRQ0;
		REG16(ICU_ISCR) = REG16(ICU_ISCR)
						& ~(3 << (irq*2))
						| (mode << (irq*2));
	}

	/* We need to override the entry in the Cybiko boot ROM's vector table
	 * to point at our ISR, not the boot ROM's. Unfortunately this table is
	 * not arranged in any sensible order.
	 */

	uint32_t cybikoisr = REG32(vector * 4);
	if (cybikoisr && (REG32(cybikoisr + 8) == 0x01006b02))
	{
		uint32_t bootvector = REG16(cybikoisr + 12);
		bootvector |= 0x00ff0000;

		REG32(bootvector) = (uint32_t) &interrupt_entry;
	}
}

/*
 * Common interrupt handler.
 */
void interrupt_handler(int vector)
{
	int old_ipl, new_ipl;

	if ((vector < 0) || (vector >= NUMBER_OF_CYBIKO_INTERRUPTS))
		return;
	vector = cybiko_to_h8300s[vector];

	/* Dispatch interrupt.
	 *
	 * Note! Unlike most other platforms, this is done with interrupts *off*.
	 * As the H8300S has no global interrupt enable register we can't do IPLs.
	 */

	irq_handler(vector);
}

/*
 * Initialize interrupt controllers.
 * All interrupts will be masked off.
 */
void
interrupt_init(void)
{
	REG8(ICU_SYSCR) = 0x01; /* interrupts controlled with I bit */
	REG8(ICU_IER) = 0; /* disable all hardware interrupts */
}
