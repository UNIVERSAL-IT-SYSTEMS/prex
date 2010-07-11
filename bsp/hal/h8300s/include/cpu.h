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

#ifndef _H8300_CPU_H
#define _H8300_CPU_H

/* Condition code register */

#define CCR_C   0x01 /* carry */
#define CCR_V   0x02 /* overflow */
#define CCR_Z   0x04 /* zero */
#define CCR_N   0x08 /* negative */
#define CCR_U   0x10 /* user */
#define CCR_H   0x20 /* half-carry */
#define CCR_UI  0x40 /* user / interrupt mask */
#define CCR_I   0x80 /* interrupt mask */

#define CCR_APP_MODE 0
#define EXR_APP_MODE 0

#define IRQ_MASK (~CCR_I & 0xff)
#define IRQ_FLAGS CCR_I

/* Extended control register */

#define EXR_IRQ 0x03 /* interrupt controller? */

/* Interrupt types */

#define INTERRUPT_HARDRESET       0
#define INTERRUPT_MANUALRESET     1
#define INTERRUPT_NMI             7
#define INTERRUPT_TRAP0           8
#define INTERRUPT_TRAP1           9
#define INTERRUPT_TRAP2           10
#define INTERRUPT_TRAP3           11
#define INTERRUPT_IRQ0            16
#define INTERRUPT_IRQ1            17
#define INTERRUPT_IRQ2            18
#define INTERRUPT_IRQ3            19
#define INTERRUPT_IRQ4            20
#define INTERRUPT_IRQ5            21
#define INTERRUPT_IRQ6            22
#define INTERRUPT_IRQ7            23
#define INTERRUPT_SWDTEND         24
#define INTERRUPT_WOVI            25
#define INTERRUPT_ADI             28
#define INTERRUPT_TGI0A           32
#define INTERRUPT_TGI0B           33
#define INTERRUPT_TGI0C           34
#define INTERRUPT_TGI0D           35
#define INTERRUPT_TGI0V           36
#define INTERRUPT_TGI1A           40
#define INTERRUPT_TGI1B           41
#define INTERRUPT_TGI1V           42
#define INTERRUPT_TGI1U           43
#define INTERRUPT_TGI2A           44
#define INTERRUPT_TGI2B           45
#define INTERRUPT_TGI2V           46
#define INTERRUPT_TGI2U           47
#define INTERRUPT_CMIA0           64
#define INTERRUPT_CMIB0           65
#define INTERRUPT_OVI0            66
#define INTERRUPT_CMIA1           68
#define INTERRUPT_CMIB1           69
#define INTERRUPT_OVI1            70
#define INTERRUPT_ERI0            80
#define INTERRUPT_RXI0            81
#define INTERRUPT_TXI0            82
#define INTERRUPT_TEI0            83
#define INTERRUPT_ERI1            84
#define INTERRUPT_RXI1            85
#define INTERRUPT_TXI1            86
#define INTERRUPT_TEI1            87
#define INTERRUPT_ERI2            88
#define INTERRUPT_RXI2            89
#define INTERRUPT_TXI2            90
#define INTERRUPT_TEI2            91

#define NIRQS                     92

/* Interrupt control unit */

#define ICU_SYSCR            0x00ffff39
#define   ICU_SYSCR_RAME     (1<<0)
#define   ICU_SYSCR_NMIEG    (1<<3)
#define   ICU_SYSCR_INTM0    (1<<4)
#define   ICU_SYSCR_INTM1    (1<<5)
#define ICU_ISCR             0x00ffff2c
#define ICU_IER              0x00ffff2e
#define ICU_ISR              0x00ffff2f
#define ICU_ICRA             0x00fffec0
#define ICU_ICRB             0x00fffec1
#define ICU_ICRC             0x00fffec2

/* Watchdog control unit */

#define WDT_TCSR_R           0x00ffffbc
#define WDT_TCSR_W           0x00ffffbc
#define   WDT_TCSR_CKS       (1<<0)
#define   WDT_TCSR_CKS_MASK  7
#define   WDT_TCSR_TME       (1<<5)
#define   WDT_TCSR_WTIT      (1<<6)
#define   WDT_TCSR_OVF       (1<<7)
#define WDT_TCNT_R           0x00ffffbd
#define WDT_TCNT_W           0x00ffffbc
#define WDT_RSTCSR_R         0x00ffffbf
#define WDT_RSTCSR_W         0x00ffffbe

#define WDT_TCSR_PASSWORD    0x5a00
#define WDT_TCNT_PASSWORD    0xa500

/* 8-bit timer unit */

#define TCR0                 0x00ffffb0
#define TCSR0                0x00ffffb2
#define TCORA0               0x00ffffb4
#define TCORB0               0x00ffffb6
#define TCNT0                0x00ffffb8

#define TCR1                 0x00ffffb1
#define TCSR1                0x00ffffb3
#define TCORA1               0x00ffffb5
#define TCORB1               0x00ffffb7
#define TCNT1                0x00ffffb9

#define TCSR_OVF             (1<<5)

#define TCR_CKS_NOCLOCK      0
#define TCR_CKS_I8           1
#define TCR_CKS_I64          2
#define TCR_CKS_I8192        3
#define TCR_OVIE             (1<<5)
#define TCR_CMIEA            (1<<6)
#define TCR_CMIEB            (1<<7)

#define MSTPCR               0x00ffff3c

#define MSTPCR_MSTP12        (1<<12)

/* Serial interface */

#define SCI0                 0x00ffff78
#define SCI1                 0x00ffff80
#define SCI2                 0x00ffff88

#define SMR                  0
#define BRR                  1
#define SCR                  2
#define TDR                  3
#define SSR                  4
#define RDR                  5
#define SCMR                 6

#define SSR_MPBT             (1<<0)
#define SSR_MPB              (1<<1)
#define SSR_TEND             (1<<2)
#define SSR_PER              (1<<3)
#define SSR_FER              (1<<4)
#define SSR_ORER             (1<<5)
#define SSR_RDRF             (1<<6)
#define SSR_TDRE             (1<<7)

#define SCR_CKE0             (1<<0)
#define SCR_CKE1             (1<<1)
#define SCR_TEIE             (1<<2)
#define SCR_MPIE             (1<<3)
#define SCR_RE               (1<<4)
#define SCR_TE               (1<<5)
#define SCR_RIE              (1<<6)
#define SCR_TIE              (1<<7)

#ifndef __ASSEMBLY__

#endif /* !__ASSEMBLY__ */
#endif /* !_H8300_CPU_H */
