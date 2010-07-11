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
 * clock.c - clock driver
 */

#include <kernel.h>
#include <timer.h>
#include <irq.h>
#include <cpufunc.h>
#include <sys/ipl.h>
#include <cpu.h>
#include "platform.h"

#define REG8(x) (*(volatile uint8_t*)(x))
#define REG16(x) (*(volatile uint16_t*)(x))

/* The clock rate per second - 1Mhz */
#define CLOCK_RATE	1000000L

/* The initial counter value */
#define TIMER_COUNT	(CLOCK_RATE / HZ)

/* Timer 1 registers */
#define TMR_LOAD	(*(volatile uint32_t *)(TIMER_BASE + 0x100))
#define TMR_VAL		(*(volatile uint32_t *)(TIMER_BASE + 0x104))
#define TMR_CTRL	(*(volatile uint32_t *)(TIMER_BASE + 0x108))
#define TMR_CLR		(*(volatile uint32_t *)(TIMER_BASE + 0x10c))

/* Timer control register */
#define TCTRL_DISABLE	0x00
#define TCTRL_ENABLE	0x80
#define TCTRL_PERIODIC	0x40
#define TCTRL_INTEN	0x20
#define TCTRL_SCALE256	0x08
#define TCTRL_SCALE16	0x04
#define TCTRL_32BIT	0x02
#define TCTRL_ONESHOT	0x01

/*
 * Clock interrupt service routine.
 */
static int clock_isr(void *arg)
{
	splhigh();
	timer_handler();
	REG8(TCSR0) &= ~TCSR_OVF;
	REG8(TCNT0) = 255 - ((SYSTEMCLOCK / CONFIG_HZ) / 8192);
	spl0();
	return INT_DONE;
}

/*
 * Initialize clock H/W chip.
 * Setup clock tick rate and install clock ISR.
 */
void clock_init(void)
{
	int x;

	/* Install ISR. */

	x = splhigh();
	REG8(TCR0) = 0;
	splx(x);

	irq_attach(INTERRUPT_OVI0, IPL_CLOCK, 0, &clock_isr,
			IST_NONE, NULL);

	/* Initialise 8-bit timer 0. */

	x = splhigh();
	REG16(MSTPCR) &= ~MSTPCR_MSTP12;
	REG8(TCR0) = TCR_OVIE | TCR_CKS_I8192;
	REG8(TCNT0) = 255 - ((SYSTEMCLOCK / CONFIG_HZ) / 8192);
	splx(x);

	DPRINTF(("Clock rate: %d ticks/sec\n", CONFIG_HZ));
}
