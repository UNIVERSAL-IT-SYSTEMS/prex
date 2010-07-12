/*-
 * Copyright (c) 2005, Kohsuke Ohtani
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

#include <conf/config.h>
#include <machine/asm.h>
#include <machine/memory.h>
#include <machine/syspage.h>
#include "platform.h"
#include "cpu.h"

/* This is called by the boot loader to start the Prex kernel. */

proc _kernel_start
	mov #current_kernel_stack, er0
	mov sp, @er0                       // save kernel stack
	bset #7, @er0                      // top bit means we're in kernel mode
	bset #3, @0x61:8                   // turn green LED on

	jmp _main

/* This is called by user code to enter a system call.
 *
 * This has to do the following:
 *
 * - switch to kernel mode.
 * - save the user mode registers into the current context->uregs structure.
 * - perform the system call.
 * - switch back out of kernel mode and restore the user mode registers.
 *
 * Switching to kernel mode involes switching to the appropriate kernel stack
 * (stored in current_kernel_stack). The context->uregs structure is stored
 * just above the top of the kernel stack on the H8300S, so
 * @current_kernel_stack + x will access it.
 *
 * Luckily the system call ABI requires the user process to save er0-er6 so
 * we only need to update the program counter and stack pointer.
 *
 * On entry to a system call, the registers are set up like this:
 *
 *   er0-er3  parameters
 *   er4      syscall ID
 *   er5-er6  scratch
 *
 * We need to push them onto the stack in C order:
 *
 *   sp+04    syscall ID
 *   sp+00    er3
 */
proc _syscall_entry
	/* Switch to the kernel stack and save the registers. */

	orc #IRQ_FLAGS, ccr                // interrupts off

	mov @current_kernel_stack, er5     // fetch current thread's kernel stack

	mov @sp, er6                       // fetch user program counter
	mov er6, @(0x28, er5)              // save pc in structure
	adds #4, sp                        // account for not needing to return
	mov sp, @(0x1c, er5)               // save user stack

	mov er5, sp                        // switch stacks
	bset #7, @current_kernel_stack     // mark ourselves as in kernel mode
	andc #IRQ_MASK, ccr                // interrupts on

	/* Do system call. */

	extu r4                            // sign extend syscall number
	extu er4

	push er4                           // save parameters
	push er3
	jsr _syscall_handler               // call syscall handler
	add #8, sp

	/* Return from system call. */

	orc #IRQ_FLAGS, ccr                // interrupts off
	bclr #7, @current_kernel_stack     // not in kernel mode any more
	mov @(0x28, sp), er5               // fetch user PC
	mov @(0x1c, sp), sp                // restore user stack

	andc #IRQ_MASK, ccr                // interrupts on
	jmp @er5

/* Used to start threads --- nothing to do with system calls.
 *
 * On the stack is a cpu_regs structure:
 *
 *   sp+28   pc
 *   sp+24   ccr
 *   sp+20   exr
 *   sp+1c   sp
 *   sp+18   er6
 *   sp+14   er5
 *   sp+10   er4
 *   sp+0c   er3
 *   sp+08   er2
 *   sp+04   er1
 *   sp+00   er0
 */

proc _syscall_ret
	orc #IRQ_FLAGS, ccr                // interrupts off
	bclr #7, @current_kernel_stack     // not in kernel mode any more

	mov @(0x1c, sp), er0               // new stack pointer
	mov @(0x28, sp), er1               // new pc
	mov er1, @-er0                     // push new pc onto new stack
//	mov @(0x24, sp), er1               // new ccr
//	mov er1, @-er0                     // push new ccr onto new stack
//	mov @(0x20, sp), er1               // new exr
//	mov er1, @-er0                     // push new exr onto new stack

	mov @(0x00, sp), er0               // restore registers
	mov @(0x04, sp), er1
	mov @(0x08, sp), er2
	mov @(0x0c, sp), er3
	mov @(0x10, sp), er4
	mov @(0x14, sp), er5
	mov @(0x18, sp), er6
	mov @(0x1c, sp), sp

	sub #4, sp
	andc #IRQ_MASK, ccr
	rts
//	sub #12, sp
//	rte                                // atomically pop new ccr and pc

/* Entry point for kernel thread */
proc _kernel_thread_entry
	mov er5, er0        /* Set argument */
	jmp @er4            /* Jump to kernel code */

/* Entry point for interrupt handler.
 *
 * The Cybiko boot ROM won't let us add interrupt handlers directly, but
 * it does vector stuff through its own table, and we can override that.
 * On entry, registers have been saved and the stack looks like this:
 *
 *   sp + 24   pc
 *   sp + 20   exr & ccr
 *   sp + 1c   er0
 *   sp + 18   er1
 *   sp + 14   er2
 *   sp + 10   er3
 *   sp + 0c   er4
 *   sp + 08   er5
 *   sp + 04   er6
 *   sp + 00   return address into Cybiko boot ROM
 *
 * The only way to find out which interrupt was called is to decipher the
 * return address --- IRQ0's return address is 0x0000113c and each ROM
 * handler is 26 bytes apart, so it's not too hard.
 */
proc _interrupt_entry
	bset #3, @0x61:8                   // turn green LED on
	mov sp, er4                        // save stack pointer

	mov @sp, er1                       // get return address
	sub #0x113c, er1
	mov #26, r0l
	divxu r0l, r1                      // divide; quotient in bottom 8 bits
	sub er0, er0
	mov r1l, r0l                       // store interrupt number in er0

	mov #current_kernel_stack, er1
	btst #7, @er1                      // are we in kernel mode?
	bne 1f

    mov @er1, sp                       // no. change to kernel stack
    mov @(0x1c, er4), er2              // read er0 from boot loader...
    mov er2, @(0x00, sp)               // save it to context->uregs
    mov @(0x18, er4), er2              // er1
    mov er2, @(0x04, sp)
    mov @(0x14, er4), er2              // er2
    mov er2, @(0x08, sp)
    mov @(0x10, er4), er2              // er3
    mov er2, @(0x0c, sp)
    mov @(0x08, er4), er2              // er4
    mov er2, @(0x10, sp)
    mov @(0x04, er4), er2              // er5
    mov er2, @(0x14, sp)
    mov @(0x00, er4), er2              // er6
    mov er2, @(0x18, sp)
    mov @(0x24, er4), er2              // pc
    mov er2, @(0x28, sp)
	mov er4, @(0x1c, sp)               // sp

1:

	mov #irq_nesting, er1
	mov @er1, r2l                      // nest interrupt
	inc r2l
	mov r2l, @er1

	cmp #1, r2l                        // is this the first interrupt?
	bne nested_interrupt               // no; go to simplified handler

	jsr _interrupt_handler

	jsr _splon
	jsr _sched_yield                   // try to preempt

	btst #7, @current_kernel_stack     // are we in kernel mode?
	bne 1f
	jsr _exception_deliver             // if not, try to deliver exceptions
1:

interrupt_ret:
	jsr _sploff
	mov er4, sp                        // restore old stack

	mov #irq_nesting, er1
	mov @er1, r2l                      // unnest interrupt
	dec r2l
	mov r2l, @er1
	rts                                // return to Cybiko boot rom

nested_interrupt:
	jsr _interrupt_handler
	bra interrupt_ret

.lcomm irq_nesting, 1

/* Idle the CPU until the next interrupt.
 */

proc _cpu_idle
	bclr #3, @0x61:8 /* turn green LED off */
	sleep
	rts

proc __cybiko_putc
	stm.l er2-er3, @-er7
	jsr 0x2418
	ldm.l @er7+, er2-er3
	rts

proc _diag_puts
	stm.l er2-er3, @-er7
	jsr 0x2428
	ldm.l @er7+, er2-er3
	rts

proc _diag_init
	rts

proc __pause
	mov #0, r0l
1:
	cmp #0, r0l
	beq 1b
	rts
