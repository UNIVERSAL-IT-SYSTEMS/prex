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

#ifndef _H8300_CONTEXT_H
#define _H8300_CONTEXT_H

/**
 * H8300 register reference:
 *
 *  Name    Number	Role
 *
 *  er0             argument 1 / integer result / scratch register
 *  er1             argument 2 / integer result / scratch register
 *  er2             argument 3 / integer result / scratch register
 *  er3             scratch register
 *  er4             register variable
 *  er5             register variable
 *  er6             register variable
 *  er7             stack pointer
 *  pc              program counter
 *  ccr             condition code register
 */

/*
 * Common register frame for trap/interrupt.
 * This cpu state are saved into top of the kernel stack in
 * trap/interrupt entries. Since the arguments of system calls are
 * passed via registers, the system call library is completely
 * dependent on this register format.
 */
struct cpu_regs {
	uint32_t er0;	/*  +0 (00) */
	uint32_t er1;	/*  +4 (04) */
	uint32_t er2;	/*  +8 (08) */
	uint32_t er3;	/* +12 (0C) */
	uint32_t er4;	/* +16 (10) */
	uint32_t er5;	/* +20 (14) */
	uint32_t er6;	/* +24 (18) */
	uint32_t sp;	/* +28 (1C) */
	uint8_t exr;    /* +32 (20) */
	uint8_t __padding1[3];
	uint8_t ccr;	/* +36 (24) */
	uint8_t __padding[3];
	uint32_t pc;	/* +40 (28) */
};

/*
 * Kernel mode context for context switching.
 */
struct kern_regs {
	uint32_t er4;
	uint32_t er5;
	uint32_t er6;
	uint32_t sp;
	uint32_t pc;
	uint32_t kstack;         /* sp used on user->kernel mode switch */
	uint8_t ccr;
};

/*
 * Processor context
 */
struct context {
	struct kern_regs kregs; /* kernel mode registers */
	struct cpu_regs	*uregs; /* user mode registers */
	struct cpu_regs	*saved_regs; /* saved user mode registers */
};

typedef struct context *context_t;	/* context id */

#endif /* !_H8300_ARCH_H */
