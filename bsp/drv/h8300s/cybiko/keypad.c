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
 * keypad.c - PC/AT keyboard driver
 */

#include <driver.h>
#include <sys/keycode.h>
#include <wscons.h>
#include <pm.h>

#define POLLING_FREQUENCY 10

struct private
{
	device_t dev;
	timer_t timer;
	uint8_t lastkey;
};

static int keypad_init(struct driver *);
static int keypad_getc(void *);
static void keypad_set_poll(void *, int);
static void kick_timer(struct private* private);
static void keypad_timer(void* arg);

struct driver keypad_driver =
{
	/* name */ "keypad",
	/* devops */ NULL,
	/* devsz */	sizeof(struct private),
	/* flags */ 0,
	/* probe */ NULL,
	/* init */ keypad_init,
	/* shutdown */ NULL,
};

static struct wscons_kbd_ops wscons_keypad_ops =
{
	/* getc */ keypad_getc,
	/* set_poll */ keypad_set_poll,
};

static const uint8_t normal_keymap[] =
{
	/* 		0		1		2		3		4		5		6		7	*/
	/* 1 */	K_F7,	0x1b,	0x7f,	K_LEFT,	'q',	'a',	'`',	0,
	/* 2 */	K_F6,	K_UP,	K_INS,	'2',	'w',	's',	'z',	0,
	/* 3 */	K_F5,	K_F3,	' ',	'3',	'e',	'd',	'x',	0,
	/* 4 */	K_F4,	'1',	9,		'4',	'r',	'f',	'c',	'[',
	/* 5 */	K_RGHT, K_DOWN, '\r',	'5',	't',	'g',	'v',	']',
	/* 6 */ K_F2,	';',	'\n',	'6',	'y',	'h',	'b',	'\\',
	/* 7 */	K_F1,	'/',	'\b',	'7',	'u',	'j',	'n',	0,
	/* 8 */	'-',	'.',	'0',	'8',	'i',	'k',	'm',	0,
	/* 9 */ '\'',	'=',	'9',	'p',	'o',	'l',	',',	0
};

static const uint8_t shifted_keymap[] =
{
	/* 		0		1		2		3		4		5		6		7	*/
	/* 1 */	K_F7,	0x1b,	0x7f,	K_LEFT,	'Q',	'A',	'`',	0,
	/* 2 */	K_F6,	K_UP,	K_INS,	'!',	'W',	'S',	'Z',	0,
	/* 3 */	K_F5,	K_F3,	' ',	'#',	'E',	'D',	'X',	0,
	/* 4 */	K_F4,	'1',	9,		'$',	'R',	'F',	'C',	'{',
	/* 5 */	K_RGHT, K_DOWN, '\r',	'%',	'T',	'G',	'V',	'}',
	/* 6 */ K_F2,	':',	'\n',	'^',	'Y',	'H',	'B',	'|',
	/* 7 */	K_F1,	'?',	'\b',	'&',	'U',	'J',	'N',	0,
	/* 8 */	'_',	'>',	')',	'*',	'I',	'K',	'M',	0,
	/* 9 */ '"',	'+',	'(',	'P',	'O',	'L',	'<',	0
};

static uint8_t probe(uint16_t ymask)
{
	volatile uint16_t* address = (volatile uint16_t*)
			(0xefffff & ~ymask);
	return *address;
}

static int scan_keyboard(struct private* private)
{
	/* Quick test first for any key press. */

	if (probe(0x3ff) != 0xff)
	{
		uint8_t matrixx, matrixy;
		const uint8_t* keymap;

		/* Is shift pressed? */

		if (!(probe(1<<1) & (1<<7)))
			keymap = shifted_keymap;
		else
			keymap = normal_keymap;

		for (matrixy = 1; matrixy < 10; matrixy++)
		{
			uint8_t state = probe(1<<matrixy);
			if (state != 0xff)
			{
				for (matrixx = 0; matrixx < 8; matrixx++)
				{
					if (!(state & (1<<matrixx)))
					{
						int rawkey = matrixx + (matrixy-1)*8;
						uint8_t cookedkey;

						cookedkey = keymap[rawkey];
						if (cookedkey)
						{
							/* Key (that we care about) pressed. Only produce
							 * a keydown event when the key goes down */
							if (private->lastkey != cookedkey)
							{
								private->lastkey = cookedkey;
								return cookedkey;
							}
							else
								return -1;
						}
					}
				}
			}
		}
	}

	/* No keys (that we care about) pressed */
	private->lastkey = 0;
	return -1;
}

static int keypad_getc(void *aux)
{
	struct private* private = aux;

	for (;;)
	{
		int key = scan_keyboard(private);
		if (key != -1)
			return key;
	}
}

static void keypad_set_poll(void *aux, int on)
{
	struct private* private = aux;
	int s = splhigh();

	if (on)
		timer_stop(&private->timer);
	else
		kick_timer(private);

	splx(s);
}

static void kick_timer(struct private* private)
{
	int s = splhigh();
	timer_stop(&private->timer);
	timer_callout(&private->timer, 1000/POLLING_FREQUENCY,
			keypad_timer, private);
	splx(s);
}

static void keypad_timer(void* arg)
{
	struct private* private = arg;
	int key = scan_keyboard(private);
	if (key != -1)
		wscons_kbd_input(key);

	kick_timer(private);
}

static int keypad_init(struct driver *self)
{
	struct private* private;
	device_t dev;

	dev = device_create(self, "kbd", D_CHR);

	private = device_private(dev);
	private->dev = dev;
	private->lastkey = 0;

	wscons_attach_kbd(&wscons_keypad_ops, private);
	kick_timer(private);
	return 0;
}
