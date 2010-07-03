/*-
 * Copyright (c) 2010, David Given
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
 * pl011.c - ARM PrimeCell PL011 UART
 */

#include <driver.h>
#include <tty.h>
#include <serial.h>
#include <bsp/hal/h8300s/include/cpu.h>

#define REG8(x) (*(volatile uint8_t*)(x))
#define REG16(x) (*(volatile uint16_t*)(x))

#define IRQMASK (SCR_TIE | SCR_RIE)

/* Forward functions */
static int	h8300serial_init(struct driver *);
static void	h8300serial_xmt_char(struct serial_port *, char);
static char	h8300serial_rcv_char(struct serial_port *);
static void	h8300serial_set_poll(struct serial_port *, int);
static void	h8300serial_start(struct serial_port *);
static void	h8300serial_stop(struct serial_port *);


struct driver h8300serial_driver = {
	/* name */ "h8300serial",
	/* devops */ NULL,
	/* devsz */ 0,
	/* flags */ 0,
	/* probe */	NULL,
	/* init */ h8300serial_init,
	/* detach */ NULL,
};

static struct serial_ops h8300serial_ops = {
	/* xmt_char */ h8300serial_xmt_char,
	/* rcv_char */ h8300serial_rcv_char,
	/* set_poll */ h8300serial_set_poll,
	/* start */ h8300serial_start,
	/* stop */ h8300serial_stop,
};

static struct serial_port h8300serial_port;
static irq_t rxi;
static irq_t txi;

/* Polled transmit (for debug purposes only). */

static void h8300serial_xmt_char(struct serial_port *sp, char c)
{
	uint8_t ssr;

	/* Wait for Tx buffer empty. */

	for (;;)
	{
		ssr = REG8(SCI2 + SSR);
		if (ssr & SSR_TDRE)
			break;
	}

	/* Write byte to transmit register. */

	REG8(SCI2 + TDR) = c;
	REG8(SCI2 + SSR) = ssr & ~SSR_TDRE;
}

/* Polled receive (for debug purposes only). */

static char h8300serial_rcv_char(struct serial_port *sp)
{
	uint8_t ssr;
	char data;

	/* Wait for Rx buffer full. */

	for (;;)
	{
		ssr = REG8(SCI2 + SSR);
		if (ssr & SSR_RDRF)
			break;
	}

	/* Read byte. */

	data = REG8(SCI2 + RDR);
	REG8(SCI2 + SSR) = ssr & ~SSR_RDRF;

	return data;
}

static void h8300serial_set_poll(struct serial_port *sp, int on)
{
	int s = splhigh();
	int scr = REG8(SCI2 + SCR);

	if (on)
	{
		/* Disable interrupt for polling mode. */
		scr &= ~IRQMASK;
	}
	else
	{
		/* Enable interrupt again */
		scr |= IRQMASK;
	}

	REG8(SCI2 + SCR) = scr;
	splx(s);
}

static int rxi_isr(void* arg)
{
	struct serial_port* sp = arg;
	int data = -1;
	uint8_t ssr;
	int s;

	s = splhigh();
	ssr = REG8(SCI2 + SSR);
	if (ssr & SSR_RDRF)
	{
		/* Only read data if data is available to read --- this provides
		 * some bullet-proofing against stray interrupts.
		 */
		data = REG8(SCI2 + RDR);
		REG8(SCI2 + SSR) = ssr & ~SSR_RDRF;
	}
	splx(s);

	if (data != -1)
		serial_rcv_char(sp, data);
	return 0;
}

static int txi_isr(void* arg)
{
	struct serial_port* sp = arg;

	serial_xmt_done(sp);
	return 0;
}

static void h8300serial_start(struct serial_port *sp)
{
	rxi = irq_attach(INTERRUPT_RXI2, IPL_COMM, 0, rxi_isr,
			     IST_NONE, sp);

	txi = irq_attach(INTERRUPT_TXI2, IPL_COMM, 0, txi_isr,
			     IST_NONE, sp);

	REG8(SCI2 + SCR) |= IRQMASK;
}

static void h8300serial_stop(struct serial_port *sp)
{
	REG8(SCI2 + SCR) &= ~IRQMASK;
}

static int h8300serial_init(struct driver *self)
{
	serial_attach(&h8300serial_ops, &h8300serial_port);
	return 0;
}
