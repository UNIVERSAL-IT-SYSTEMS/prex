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

/*
 * Interrupt Priority Level
 *
 * Each interrupt has its logical priority level, with 0 being
 * the lowest priority. While some ISR is running, all lower
 * priority interrupts are masked off.
 */
volatile int irq_level;

/* The mapping describing which bits of the ICR register correspond to which
 * interrupt number. It just so happens that no ICR bit 0 is used, so we use
 * 0 to identify an interrupt with no ICR bit.
 *
 * The top nibble indicates which ICR register, the bottom is the bit number.
 */
struct icr_descriptor
{
	unsigned reg : 4;
	unsigned bit : 4;
} __attribute__ ((packed));

static const struct icr_descriptor icr_table[] =
{
	[INTERRUPT_IRQ0] = {0, 7},
	[INTERRUPT_IRQ1] = {0, 6},
	[INTERRUPT_IRQ2] = {0, 5},
	[INTERRUPT_IRQ3] = {0, 5},
	[INTERRUPT_IRQ4] = {0, 4},
	[INTERRUPT_IRQ5] = {0, 4},
	[INTERRUPT_IRQ6] = {0, 3},
	[INTERRUPT_IRQ7] = {0, 3},
	[INTERRUPT_SWDTEND] = {0, 2},
	[INTERRUPT_WOVI] = {0, 1},
	[INTERRUPT_ADI] = {1, 6},
	[INTERRUPT_TGI0A] = {1, 5},
	[INTERRUPT_TGI0B] = {1, 5},
	[INTERRUPT_TGI0C] = {1, 5},
	[INTERRUPT_TGI0D] = {1, 5},
	[INTERRUPT_TGI0V] = {1, 5},
	[INTERRUPT_TGI1A] = {1, 4},
	[INTERRUPT_TGI1B] = {1, 4},
	[INTERRUPT_TGI1V] = {1, 4},
	[INTERRUPT_TGI1U] = {1, 4},
	[INTERRUPT_TGI2A] = {1, 3},
	[INTERRUPT_TGI2B] = {1, 3},
	[INTERRUPT_TGI2V] = {1, 3},
	[INTERRUPT_TGI2U] = {1, 3},
	[INTERRUPT_CMIA0] = {2, 7},
	[INTERRUPT_CMIB0] = {2, 7},
	[INTERRUPT_OVI0] = {2, 7},
	[INTERRUPT_CMIA1] = {2, 6},
	[INTERRUPT_CMIB1] = {2, 6},
	[INTERRUPT_OVI1] = {2, 6},
	[INTERRUPT_ERI0] = {2, 4},
	[INTERRUPT_RXI0] = {2, 4},
	[INTERRUPT_TXI0] = {2, 4},
	[INTERRUPT_TEI0] = {2, 4},
	[INTERRUPT_ERI1] = {2, 3},
	[INTERRUPT_RXI1] = {2, 3},
	[INTERRUPT_TXI1] = {2, 3},
	[INTERRUPT_TEI1] = {2, 3},
	[INTERRUPT_ERI2] = {2, 2},
	[INTERRUPT_RXI2] = {2, 2},
	[INTERRUPT_TXI2] = {2, 2},
	[INTERRUPT_TEI2] = {2, 2}
};

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
 * Interrupt mapping table
 */
static uint8_t ipl_table[NIRQS];		/* vector -> level */
static uint8_t enable_table[NIPLS][3];	/* level -> mask */

/*
 * Enable appropriate interrupts for the current level
 */
static void update_icr(void)
{
	uint8_t* p = enable_table[irq_level];
	REG8(ICU_ICRA) = p[0];
	REG8(ICU_ICRB) = p[1];
	REG8(ICU_ICRC) = p[2];
}

/*
 * Unmask interrupt in ICU for specified irq.
 * The interrupt mask table is also updated.
 * Assumes CPU interrupt is disabled in caller.
 */
void interrupt_unmask(int vector, int level)
{
	const struct icr_descriptor* p = &icr_table[vector];
	uint8_t reg = p->reg;
	uint8_t bit = p->bit;

	if (!bit)
		return;

	uint8_t mask = 1 << bit;

	/* Save level mapping */
	ipl_table[vector] = level;

	/*
	 * Unmask the target interrupt for all
	 * lower interrupt levels.
	 */
	for (uint8_t i = 0; i < level; i++)
		enable_table[i][reg] |= mask;
	update_icr();
}

/*
 * Mask interrupt in ICU for specified irq.
 * Interrupt must be disabled when this routine is called.
 */
void interrupt_mask(int vector)
{
	const struct icr_descriptor* p = &icr_table[vector];
	uint8_t reg = p->reg;
	uint8_t bit = p->bit;

	if (!bit)
		return;

	uint8_t mask = ~(1 << bit);

	int s = splhigh();
	{
		uint8_t level = ipl_table[vector];
		for (uint8_t i = 0; i < level; i++)
			enable_table[i][reg] &= mask;
		ipl_table[vector] = IPL_NONE;
		update_icr();
	}
	splx(s);
}

/*
 * Setup interrupt mode.
 * Select whether an interrupt trigger is edge or level.
 */
void
interrupt_setup(int vector, int mode)
{
	REG16(ICU_ISCR) = REG16(ICU_ISCR)
			        & ~(3 << (vector*2))
	                | (mode << (vector*2));
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

	/* Adjust interrupt level */

	old_ipl = irq_level;
	new_ipl = ipl_table[vector];
	if (new_ipl > old_ipl)		/* Ignore spurious interrupt */
		irq_level = new_ipl;
	update_icr();

	/* Dispatch interrupt */

	splon();
	irq_handler(vector);
	sploff();

	/* Restore interrupt level */

	irq_level = old_ipl;
	update_icr();
}

/*
 * Initialize interrupt controllers.
 * All interrupts will be masked off.
 */
void
interrupt_init(void)
{
	int i;

	irq_level = IPL_NONE;

	for (i = 0; i < NIRQS; i++)
		ipl_table[i] = IPL_NONE;

	for (i = 0; i < NIPLS; i++)
		enable_table[i][0] = 0;

	REG8(ICU_SYSCR) = 0x01; /* interrupts controlled with I bit */
	REG8(ICU_IER) = 0; /* disable all hardware interrupts */
}
