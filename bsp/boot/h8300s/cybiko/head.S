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
#include <bsp/hal/h8300s/include/cpu.h>

.h8300s

.global boot_stack
.lcomm boot_stack, CONFIG_BOOTSTACK_SIZE

.global _start
.text
_start:
	.byte 0x12, 0x34, 0xab, 0xcd
	.long boot

#if 0
readbyte = 0x234a
writebyte = 0x2418

boot:

loop:
	jsr readbyte

	push er0
	shlr #2, r0l
	shlr #2, r0l
	jsr writenibble
	pop er0
	jsr writenibble

	mov #',', r0l
	jsr writebyte
	bra boot

writenibble:
	and #0xf, er0
	add #hextable, er0
	mov.b @er0, r0l
	jmp writebyte

hextable:
	.byte '0', '1', '2', '3', '4', '5', '6', '7'
	.byte '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'

#else
	jmp _syscall_entry

boot:
	orc #IRQ_FLAGS, ccr
	mov.l #(boot_stack + CONFIG_BOOTSTACK_SIZE), er7

	/* Copy R/W data from ROM to RAM. */
	mov.l #__data_in_rom_start, er1
	mov.l #__data_in_ram_start, er2
	mov.l #__data_in_ram_end, er3
1:
	mov.l @er1+, er0
	mov.l er0, @er2
	add #4, er2
	cmp er2, er3
	bne 1b

	/* Clear BSS. */
	mov.l #__bss_end, er1
	sub er0, er0
1:
	mov.l er0, @er3
	add #4, er3
	cmp er3, er1
	bne 1b

	jsr _startup
	jmp _kernel_start
#endif
