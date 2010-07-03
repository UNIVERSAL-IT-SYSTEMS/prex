/*-
 * Copyright (c) 2009 David Given
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
 * cpufunc.S - H8300 specific CPU functions
 */

#include <conf/config.h>
#include <machine/asm.h>
#include <cpu.h>

proc _cpu_idle
	sleep
	rts

/* Enable all interrupts. */
proc _splon
	andc.b #IRQ_MASK, ccr
	rts

/* Disable all interrupts. */
proc _sploff
	orc #IRQ_FLAGS, ccr
	rts

/*
 * Switch register context.
 * er0 = previous kern_regs
 * er1 = next kern_regs
 * Interrupts must be disabled by caller.
 *
 * syntax - void cpu_switch(kern_regs *prev, kern_regs *next)
 *
 * struct kern_regs {
 *      uint32_t er4;        0
 *      uint32_t er5;        4
 *      uint32_t er6;        8
 *      uint32_t sp;         12
 *      uint32_t pc;         16
 *      uint32_t kstack;     20
 *      uint8_t ccr;         24
 * };
 *
 * Note: GCC uses r3 as a scratch register
 */
proc _cpu_switch
	/* Save old context. */
	mov.l @sp+, er3
	mov.l er4, @(0, er0)
	mov.l er5, @(4, er0)
	mov.l er6, @(8, er0)
	mov.l sp, @(12, er0)
	mov.l er3, @(16, er0)

	mov @current_kernel_stack, er3
	mov er3, @(20, er0)

	stc ccr, r3l
	mov.b r3l, @(24, er0)

	/* Load new context. */
	orc #IRQ_FLAGS, ccr                // disable interrupts
	mov.l @(0, er1), er4
	mov.l @(4, er1), er5
	mov.l @(8, er1), er6
	mov.l @(12, er1), sp
	mov.l @(16, er1), er2

	mov @(20, er1), er3
	mov er3, @current_kernel_stack
	bset #7, @current_kernel_stack     // mark us as being in kernel mode

	mov.b @(24, er1), r3l
	ldc r3l, ccr
	jmp @er2

.global current_kernel_stack
.lcomm current_kernel_stack, 4
