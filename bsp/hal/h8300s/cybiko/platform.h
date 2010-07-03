/*
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

#ifndef _CYBIKO_PLATFORM_H
#define _CYBIKO_PLATFORM_H

/* The Cybiko Classic runs on a H8S/2241 in advanced mode 6. */

#define SYSTEMCLOCK 11059200

/* Memory map */

#define MEMORY_FLASH         0x00100000 /* external bus */
#define MEMORY_RAM           0x00200000 /* external bus */
#define MEMORY_LCD           0x00600000 /* external bus */
#define MEMORY_KEYBOARD      0x00e00000 /* external bus */
#define MEMORY_ONCHIPRAM     0x00ffec00 /* internal bus */
#define  MEMORY_ONCHIPRAM_SIZE   0x1000 /* external bus */
#define MEMORY_IOAREA1       0x00fffe3f /* internal bus */
#define MEMORY_IOAREA2       0x00ffff28 /* internal bus */

/* Interrupt vectors */

/* The Cybiko ROM routes all interrupt vectors via a table in the on-chip
 * RAM.
 */

/* Interrupt types */

#define VECTOR_IRQ0            0x00ffec14
#define VECTOR_IRQ1            0x00ffec18
#define VECTOR_IRQ2            0x00ffec1c
#define VECTOR_IRQ3            0x00ffec20
#define VECTOR_IRQ4            0x00ffec24
#define VECTOR_IRQ5            0x00ffec28
#define VECTOR_IRQ6            0x00ffec2c
#define VECTOR_IRQ7            0x00ffec30
#define VECTOR_SWDTEND         0x00ffec34
#define VECTOR_WOVI            0x00ffec38
#define VECTOR_ADI             0x00ffec3c
#define VECTOR_TGI0A           0x00ffec40
#define VECTOR_TGI0B           0x00ffec44
#define VECTOR_TGI0C           0x00ffec48
#define VECTOR_TGI0D           0x00ffec4c
#define VECTOR_TGI0V           0x00ffec50
#define VECTOR_TGI1A           0x00ffec54
#define VECTOR_TGI1B           0x00ffec58
#define VECTOR_TGI1V           0x00ffec5c
#define VECTOR_TGI1U           0x00ffec60
#define VECTOR_TGI2A           0x00ffec64
#define VECTOR_TGI2B           0x00ffec68
#define VECTOR_TGI2V           0x00ffec6c
#define VECTOR_TGI2U           0x00ffec70
#define VECTOR_CMIA0           0x00ffec74
#define VECTOR_CMIB0           0x00ffec78
#define VECTOR_OVI0            0x00ffec7c
#define VECTOR_CMIA1           0x00ffec80
#define VECTOR_CMIB1           0x00ffec84
#define VECTOR_OVI1            0x00ffec88
#define VECTOR_ERI0            0x00ffec8c
#define VECTOR_RXI0            0x00ffec90
#define VECTOR_TXI0            0x00ffec94
#define VECTOR_TEI0            0x00ffec98
#define VECTOR_ERI1            0x00ffec9c
#define VECTOR_RXI1            0x00ffeca0
#define VECTOR_TXI1            0x00ffeca4
#define VECTOR_TEI1            0x00ffeca8
#define VECTOR_ERI2            0x00ffecac
#define VECTOR_RXI2            0x00ffecb0
#define VECTOR_TXI2            0x00ffecb4
#define VECTOR_TEI2            0x00ffecb8

#endif /* !_CYBIKO_PLATFORM_H */
