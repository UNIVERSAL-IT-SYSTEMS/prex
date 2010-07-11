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

/* Hitachi HD66421 console (text mode) driver
 */

#include <driver.h>
#include <wscons.h>
#include <devctl.h>
#include <pm.h>
#include "8bitmonofont.h"

#define DEBUG_HD66421CONSOLE 1

#ifdef DEBUG_HD66421CONSOLE
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

struct lcd
{
	volatile uint8_t reg;
	volatile uint8_t data;
};

/* r0 */
#define ADC   (1<<0)
#define HOLT  (1<<1)
#define REV   (1<<2)
#define AMP   (1<<3)
#define PWR   (1<<4)
#define STBY  (1<<5)
#define DISP  (1<<6)
#define RMW   (1<<7)

/* r1 */
#define BLK   (1<<0)
#define INC   (1<<1)
#define DTY0  (1<<2)
#define DTY1  (1<<3)
#define GRAY  (1<<4)
#define WLS   (1<<5)
#define BIS0  (1<<6)
#define BIS1  (1<<7)

/* r17 */
#define PSEL  (1<<0)
#define DSEL  (1<<1)
#define MON   (1<<2)

#define ROWS (100/8)
#define COLS (160/8)

#define ATTR_DIRTY 0x8000

static struct lcd* const lcd = (void*) CONFIG_HD66421_BASE;
static uint8_t ctrlr0 = 0x59; /* values set by boot loader */
static uint8_t ctrlr1 = 0x02;

static int hd66421con_init(struct driver*);
static void	hd66421con_cursor(void*, int, int);
static void	hd66421con_putc(void *, int, int, int);
static void	hd66421con_copyrows(void *,int, int, int);
static void	hd66421con_eraserows(void *,int, int);
static void	hd66421con_set_attr(void *, int);
static void	hd66421con_get_cursor(void *, int *, int *);

struct private
{
	device_t dev;
	uint16_t data[ROWS * COLS];
	uint8_t cursorx;
	uint8_t cursory;
};

static const struct devops hd66421con_devops = {
	/* open */	no_open,
	/* close */	no_close,
	/* read */	no_read,
	/* write */	no_write,
	/* ioctl */	no_ioctl,
	/* devctl */ no_devctl,
};

struct driver hd66421con_driver = {
	/* name */ "hd66421con",
	/* devops */ (struct devops*) &hd66421con_devops,
	/* devsz */	sizeof(struct private),
	/* flags */	0,
	/* probe */	NULL,
	/* init */ hd66421con_init,
	/* shutdown */ NULL,
};

static const struct wscons_video_ops wscons_hd66421con_ops = {
	hd66421con_cursor,
	hd66421con_putc,
	hd66421con_copyrows,
	hd66421con_eraserows,
	hd66421con_set_attr,
	hd66421con_get_cursor
};

static void poke(int reg, uint8_t value)
{
	lcd->reg = reg;
	lcd->data = value;
}

static uint8_t peek(int reg)
{
	lcd->reg = reg;
	return lcd->data;
}

static void change_r0(uint8_t mask, uint8_t flags)
{
	ctrlr0 &= mask;
	ctrlr0 |= flags;
	poke(0, ctrlr0);
}

static void change_r1(uint8_t mask, uint8_t flags)
{
	ctrlr1 &= mask;
	ctrlr1 |= flags;
	poke(1, ctrlr1);
}

static void rendercell(int x, int y, uint8_t c, uint8_t bit, int invert)
{
	int i, j;
	const uint8_t* ptr = font_bitmap[c - FONT_BASE];

#if defined CONFIG_HD66421_INVERT
	ptr += 7;
#endif

	poke(2, x);
	poke(3, y);

	lcd->reg = 4;
	for (i = 0; i < 8; i++)
	{
		uint8_t packed =
#if defined CONFIG_HD66421_INVERT
				*ptr--
#else
				*ptr++
#endif
				;
		uint8_t unpacked = 0;
		for (j = 0; j < 4; j++)
		{
			unpacked <<= 2;
			if (packed & bit)
				unpacked |= 3;
			packed <<= 1;
		}

		if (invert)
			unpacked = ~unpacked;
		lcd->data = unpacked;
	}
}

static int get_y_for_cell(int y)
{
	return
#if defined CONFIG_HD66421_INVERT
		(ROWS - 1 - y) * 8;
#else
		y*8
#endif
		;
}

static void update(struct private* private)
{
	int x, y;

	for (y = 0; y < ROWS; y++)
	{
		uint16_t* cell = &private->data[y*COLS];

		for (x = 0; x < COLS; x++)
		{
			if (*cell & ATTR_DIRTY)
			{
				uint8_t c = *cell & 0x7f;
				int yy = get_y_for_cell(y);
				int invert = (x == private->cursorx) && (y == private->cursory);

				if ((c < FONT_BASE) || (c > (FONT_BASE + sizeof(font_bitmap)/8)))
					c = ' ';

				rendercell(x*2, yy, c, 0x80, invert);
				rendercell(x*2+1, yy, c, 0x08, invert);

				*cell &= ~ATTR_DIRTY;
			}

			cell++;
		}
	}
}

static void	hd66421con_cursor(void* aux, int row, int col)
{
	struct private* private = aux;

	private->data[private->cursorx + private->cursory*COLS] |= ATTR_DIRTY;
	private->cursorx = col;
	private->cursory = row;
	private->data[private->cursorx + private->cursory*COLS] |= ATTR_DIRTY;
	update(private);
}

static void	hd66421con_putc(void* aux, int y, int x, int c)
{
	struct private* private = aux;
	uint16_t* cell = &private->data[x + y*COLS];

	*cell = ATTR_DIRTY | c;
	update(private);
}

static void	hd66421con_copyrows(void* aux, int srcrow, int destrow, int rows)
{
	struct private* private = aux;
	int x;

	while (rows--)
	{
		const uint16_t* src = &private->data[srcrow*COLS];
		uint16_t* dest = &private->data[destrow*COLS];

		for (x = 0; x < COLS; x++)
			*dest++ = *src++ | ATTR_DIRTY;

		srcrow++;
		destrow++;
	}

	update(private);
}

static void	hd66421con_eraserows(void* aux, int row, int rows)
{
	struct private* private = aux;
	int x;

	while (rows--)
	{
		uint16_t* dest = &private->data[row*COLS];

		for (x = 0; x < COLS; x++)
			*dest++ = ATTR_DIRTY | ' ';

		row++;
	}

	update(private);
}

static void	hd66421con_set_attr(void* aux, int attr)
{
}

static void	hd66421con_get_cursor(void* aux, int* col, int* row)
{
	struct private* private = aux;

	if (row)
		*row = private->cursory;
	if (col)
		*col = private->cursorx;
}

static void clear(void)
{
	int x, y;

	for (x = 0; x < 160/4; x++)
	{
		poke(2, x);
		poke(3, 0);

		lcd->reg = 4;
		for (y = 0; y < 100; y++)
			lcd->data = 0;
	}
}

static int hd66421con_init(struct driver* self)
{
	struct private* private;
	device_t dev;

	dev = device_create(self, "hd66421con", D_CHR);
	private = device_private(dev);
	private->cursorx = private->cursory = 0;

	//change_r0(~ADC, 0);
	change_r1(~INC, BLK);
	poke(6, 0); /* blink start */
	poke(7, 0); /* blink end */
	poke(8, 0); /* blink raster 1 */
	poke(9, 0); /* blink raster 2 */
	poke(10, 0); /* blink raster 3 */

	clear();
	hd66421con_eraserows(private, 0, ROWS);

	{
		const char msg[10] = "Booting...";
		int x;

		for (x = 0; x < sizeof(msg); x++)
			private->data[x] = msg[x] | ATTR_DIRTY;

		hd66421con_cursor(private, 1, 0);
		update(private);
	}

	wscons_attach_video((struct wscons_video_ops*) &wscons_hd66421con_ops,
			private);
	pm_attach_lcd(dev);

	return 0;
}
