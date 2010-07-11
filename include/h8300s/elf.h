/*
 * Copyright (c) 2007, Kohsuke Ohtani
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

#ifndef _H8300_ELF_H
#define _H8300_ELF_H

#define R_H8_NONE       0
#define R_H8_DIR32      1
#define R_H8_DIR32_28   2
#define R_H8_DIR32_24   3
#define R_H8_DIR32_16   4
#define R_H8_DIR32U     6
#define R_H8_DIR32U_28  7
#define R_H8_DIR32U_24  8
#define R_H8_DIR32U_20  9
#define R_H8_DIR32U_16 10
#define R_H8_DIR24     11
#define R_H8_DIR24_20  12
#define R_H8_DIR24_16  13
#define R_H8_DIR24U    14
#define R_H8_DIR24U_20 15
#define R_H8_DIR24U_16 16
#define R_H8_DIR16     17
#define R_H8_DIR16U    18
#define R_H8_DIR16S_32 19
#define R_H8_DIR16S_28 20
#define R_H8_DIR16S_24 21
#define R_H8_DIR16S_20 22
#define R_H8_DIR16S    23
#define R_H8_DIR8      24
#define R_H8_DIR8U     25
#define R_H8_DIR8Z_32  26
#define R_H8_DIR8Z_28  27
#define R_H8_DIR8Z_24  28
#define R_H8_DIR8Z_20  29
#define R_H8_DIR8Z_16  30
#define R_H8_PCREL16   31
#define R_H8_PCREL8    32
#define R_H8_BPOS      33
#define R_H8_PCREL32   34
#define R_H8_GOT32O    35
#define R_H8_GOT16O    36
#define R_H8_DIR16A8   59
#define R_H8_DIR16R8   60
#define R_H8_DIR24A8   61
#define R_H8_DIR24R8   62
#define R_H8_DIR32A16  63
#define R_H8_ABS32     65
#define R_H8_ABS32A16 127

#endif /* !_H8300_ELF_H */
