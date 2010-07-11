/*
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

#ifndef _H8300S_SYSTRAP_H
#define _H8300S_SYSTRAP_H

#if defined(__cybiko__)

#include <machine/asm.h>

/* On the Cybiko, system traps are normal subroutine calls to 0x100008
 * (CONFIG_LOADER_TEXT + 8; CONFIG_LOADER_TEXT isn't available here, alas).
 */

.macro syscall0 name, id
	proc _\name
		stm er4-er6, @-sp
		mov #\id, r4l
		jsr 0x100008
		ldm @sp+, er4-er6
		rts
.endm

.macro syscall4 name, id
	proc _\name
		stm er4-er6, @-sp
		mov @(4*4, sp), er3              ; parameter 4
		mov #\id, r4l
		jsr 0x100008
		ldm @sp+, er4-er6
		rts
.endm

#define SYSCALL0(name) syscall0 name, SYS_##name
#define SYSCALL1(name) syscall0 name, SYS_##name
#define SYSCALL2(name) syscall0 name, SYS_##name
#define SYSCALL3(name) syscall0 name, SYS_##name
#define SYSCALL4(name) syscall4 name, SYS_##name

#else
#error Architecture not supported
#endif

#endif
